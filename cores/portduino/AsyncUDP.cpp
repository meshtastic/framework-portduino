#include "AsyncUDP.h"
#include <unistd.h>
#include <sys/socket.h>
#include "Utility.h"

void _asyncudp_async_cb(uv_async_t *handle) {
    AsyncUDP *udp = (AsyncUDP *)handle->data;
    udp->_DO_NOT_CALL_async_cb();
}

AsyncUDP::AsyncUDP() {
    _handler = NULL;
    _connected = false;
    _fd = 0;
    _quit.store(false);

    uv_loop_init(&_loop);
    _async.data = this;
    uv_async_init(&_loop, &_async, _asyncudp_async_cb);
}

AsyncUDP::~AsyncUDP() {
    _quit.store(true);
    uv_async_send(&_async);
    _ioThread.join();
    uv_loop_close(&_loop);
    if (_fd > 0) {
        close(_fd);
        _fd = 0;
    }
}

asyncUDPSendTask::asyncUDPSendTask(uint8_t *data, size_t len, IPAddress addr, uint16_t port) {
    this->data = (uint8_t*)malloc(len);
    memcpy(this->data, data, len);
    this->len = len;
    this->addr = addr;
    this->port = port;
}

void _asyncudp_alloc_buffer_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

void _asyncudp_on_read_cb(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags) {
    AsyncUDP *udp = (AsyncUDP *)handle->data;
    udp->_DO_NOT_CALL_uv_on_read(handle, nread, buf, addr, flags);
}

void AsyncUDP::_DO_NOT_CALL_uv_on_read(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags) {
    _handlerMutex.lock();
    auto h = _handler;
    _handlerMutex.unlock();
    if (h) {
        AsyncUDPPacket packet((uint8_t*)buf->base, nread);
        h(packet);
    }
    free(buf->base);
}

bool AsyncUDP::listenMulticast(const IPAddress addr, uint16_t port, uint8_t ttl) {
    if (_connected) {
        return false;
    }
    if (uv_udp_init(&_loop, &_socket) < 0) {
        return false;
    }
    _socket.data = this;

    _fd = socket(AF_INET, SOCK_DGRAM|SOCK_NONBLOCK|SOCK_CLOEXEC, 0);
    if (_fd < 0) {
        return false;
    }

    // We want multiple distinct processes to all be able to join the same multicast mesh.

    // SO_REUSEADDR and SO_REUSEPORT allows multiple instances to loadbalance incoming UDP packets.
    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
        // FIXME: show a message, this is not a killer but it will prevent multiple instances to share the same port
    }
    opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(int)) < 0) {
        // FIXME: show a message, this is not a killer but it will prevent multiple instances to share the same port
    }
    // On Linux, setting SO_BROADCAST has the undocumented side effect to change the loadbalance
    // behavior into a broadcast where all sockets receive all packets, which we want.
    // We never want to send packets to the broadcast address, which is the documented behavior.
    opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(int)) < 0) {
        close(_fd);
        _fd = 0;
        return true;
    }
    
    opt = ttl;
    if (setsockopt(_fd, IPPROTO_IP, IP_MULTICAST_TTL, &opt, sizeof(opt)) < 0) {
        close(_fd);
        _fd = 0;
        return true;
    }
    // Have to enable loopback to get multiple processes to inter-communicate
    opt = 1;
    if (setsockopt(_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &opt, sizeof(opt)) < 0) {
        close(_fd);
        _fd = 0;
        return true;
    }

    if (uv_udp_open(&_socket, _fd) < 0) {
        close(_fd);
        _fd = 0;
        return true;
    }

    // FIXME: don't do bytes → string → bytes IP conversion
    int maxIpLength = 3*4+3; // 3 digits per octet, 4 octets, 3 dots
    char addr_str[maxIpLength+1]; // +1 for null terminator
    snprintf(addr_str, maxIpLength, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
    addr_str[maxIpLength] = '\0';
    struct sockaddr uvAddr;
    uv_ip4_addr(addr_str, port, (struct sockaddr_in *)&uvAddr);

    if (uv_udp_set_membership(&_socket, addr_str, NULL, UV_JOIN_GROUP) < 0) {
        close(_fd);
        _fd = 0;
        return true;
    }
    if (uv_udp_bind(&_socket, (const struct sockaddr *)&uvAddr, 0) < 0) {
        close(_fd);
        _fd = 0;
        return true;
    }
    if (uv_udp_recv_start(&_socket, _asyncudp_alloc_buffer_cb, _asyncudp_on_read_cb) < 0) {
        close(_fd);
        _fd = 0;
        return true;
    }

    _ioThread = std::thread([this](){
        uv_run(&_loop, UV_RUN_DEFAULT);
    });

    _listenIP = addr;
    _connected = true;
    return true;
}

size_t AsyncUDP::writeTo(const uint8_t *data, size_t len, const IPAddress addr, uint16_t port) {
    auto task = std::make_unique<asyncUDPSendTask>((uint8_t*)data, len, addr, port);
    _sendQueueMutex.lock();
    _sendQueue.push_back(std::move(task));
    _sendQueueMutex.unlock();
    uv_async_send(&_async);
    return len;
}

void AsyncUDP::_DO_NOT_CALL_async_cb() {
    _sendQueueMutex.lock();
    while (!_sendQueue.empty()) {
        auto task = std::move(_sendQueue.back());
        _sendQueue.pop_back();
        _sendQueueMutex.unlock();
        _doWrite(task->data, task->len, task->addr, task->port);
        _sendQueueMutex.lock();
    }
    _sendQueueMutex.unlock();
    if (_quit.load()) {
        uv_udp_recv_stop(&_socket);
        // FIXME: don't do bytes → string → bytes IP conversion
        int maxIpLength = 3*4+3; // 3 digits per octet, 4 octets, 3 dots
        char addr_str[maxIpLength+1]; // +1 for null terminator
        snprintf(addr_str, maxIpLength, "%d.%d.%d.%d", _listenIP[0], _listenIP[1], _listenIP[2], _listenIP[3]);
        addr_str[maxIpLength] = '\0';
        uv_udp_set_membership(&_socket, addr_str, NULL, UV_LEAVE_GROUP);
        uv_stop(&_loop);
    }
}

void _asyncudp_send_cb(uv_udp_send_t *req, int status) {
    free(req);
}

void AsyncUDP::_doWrite(const uint8_t *data, size_t len, const IPAddress addr, uint16_t port) {
    // FIXME: don't do bytes → string → bytes IP conversion
    int maxIpLength = 3*4+3; // 3 digits per octet, 4 octets, 3 dots
    char addr_str[maxIpLength+1]; // +1 for null terminator
    snprintf(addr_str, maxIpLength, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
    addr_str[maxIpLength] = '\0';

    // FIXME: implement error handling rather than raising SIGSEGV
    struct sockaddr uvAddr;
    uv_ip4_addr(addr_str, port, (struct sockaddr_in *)&uvAddr);

    uv_udp_send_t *req = (uv_udp_send_t *)malloc(sizeof(uv_udp_send_t));
    uv_buf_t msg;
    msg.base = (char *)data;
    msg.len = len;
    if (uv_udp_send(req, &_socket, &msg, 1, (const struct sockaddr *)&uvAddr, _asyncudp_send_cb) < 0) {
        // FIXME: I don't know how to handle this error, better to just drop the packet.
    }
}
