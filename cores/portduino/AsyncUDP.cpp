#include "AsyncUDP.h"
#include "Utility.h"

void _asyncudp_async_cb(uv_async_t *handle) {
    AsyncUDP *udp = (AsyncUDP *)handle->data;
    udp->_DO_NOT_CALL_async_cb();
}

AsyncUDP::AsyncUDP() {
    _handler = NULL;
    _connected = false;
    uv_loop_init(&_loop);
    _async.data = this;
    uv_async_init(&_loop, &_async, _asyncudp_async_cb);
}

AsyncUDP::~AsyncUDP() {
    _quit.store(true);
    uv_async_send(&_async);
    _ioThread.join();
    uv_loop_close(&_loop);
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
        portduinoError("FIXME: implement proper error handling; uv_udp_init failed");
    }
    _socket.data = this;
    // FIXME: don't do bytes → string → bytes IP conversion
    int maxIpLength = 3*4+3; // 3 digits per octet, 4 octets, 3 dots
    char addr_str[maxIpLength+1]; // +1 for null terminator
    snprintf(addr_str, maxIpLength, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
    addr_str[maxIpLength] = '\0';
    struct sockaddr uvAddr;
    uv_ip4_addr(addr_str, port, (struct sockaddr_in *)&uvAddr);
    if (uv_udp_bind(&_socket, (const struct sockaddr *)&uvAddr, UV_UDP_REUSEADDR) < 0) {
        portduinoError("FIXME: implement proper error handling; uv_udp_bind failed");
    }
    if (uv_udp_set_multicast_loop(&_socket, false) < 0) {
        portduinoError("FIXME: implement proper error handling; uv_udp_set_multicast_loop failed");
    }
    if (uv_udp_set_multicast_ttl(&_socket, ttl) < 0) {
        portduinoError("FIXME: implement proper error handling; uv_udp_set_multicast_ttl failed");
    }
    if (uv_udp_set_membership(&_socket, addr_str, NULL, UV_JOIN_GROUP) < 0) {
        portduinoError("FIXME: implement proper error handling; uv_udp_set_membership failed");
    }
    if (uv_udp_recv_start(&_socket, _asyncudp_alloc_buffer_cb, _asyncudp_on_read_cb) < 0) {
        portduinoError("FIXME: implement proper error handling; uv_udp_recv_start failed");
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
        portduinoError("FIXME: implement proper error handling; uv_udp_send failed");
    }
}
