// This implements a really small subset of AsyncUDP.
// This was the bare minimum needed to get meshtastic's multicast UDP transport running.
// PRs are welcome to implement more of the API.

#ifndef ESPASYNCUDP_H
#define ESPASYNCUDP_H

#include "IPAddress.h"
#include "Print.h"
#include <functional>
#include <atomic>
#include <mutex>
#include <memory>
#include <thread>
#include <uv.h>

class AsyncUDP;

class AsyncUDPPacket final
{
private:
    uint8_t *_data;
    size_t _len;

protected:
    AsyncUDPPacket(uint8_t* byte, size_t len) {
        _data = byte;
        _len = len;
    };

public:
    uint8_t * data() {
        return _data;
    };
    size_t length() {
        return _len;
    };

    friend AsyncUDP;
};

class asyncUDPSendTask final {
    protected:
        uint8_t *data;
        size_t len;
        IPAddress addr;
        uint16_t port;

    public:
        asyncUDPSendTask(uint8_t *data, size_t len, IPAddress addr, uint16_t port);
    
        ~asyncUDPSendTask() {
            free(data);
        };

    friend AsyncUDP;
};

typedef std::function<void(AsyncUDPPacket& packet)> AuPacketHandlerFunction;
typedef std::function<void(void * arg, AsyncUDPPacket& packet)> AuPacketHandlerFunctionWithArg;

class AsyncUDP final
{
private:
    std::mutex _handlerMutex;
    AuPacketHandlerFunction _handler;

    std::mutex _sendQueueMutex;
    // the queue is used because uv_udp_send is not threadsafe and uv_async can merge multiple calls into one callback
    std::vector<std::unique_ptr<asyncUDPSendTask>> _sendQueue;

    // Theses must be accessed from the uv loop.
    // _waitingToBeLooped is used to wait for a sent packet to be looped back before we send an other one.
    // This allows the recv callback to omit sent packets.
    std::unique_ptr<asyncUDPSendTask> _waitingToBeLooped;
    bool _emptiedBuffer = false;
    uv_timer_t _timer;

    std::atomic<bool> _quit;
    std::thread _ioThread;

    bool _connected;
    IPAddress _listenIP;

    uv_loop_t _loop;
    uv_udp_t _socket;
    int _fd;
    uv_async_t _async;

public:
    AsyncUDP();
    ~AsyncUDP();

    void onPacket(AuPacketHandlerFunctionWithArg cb, void * arg=NULL) {
        onPacket(std::bind(cb, arg, std::placeholders::_1));
    };
    void onPacket(AuPacketHandlerFunction cb) {
        _handlerMutex.lock();
        _handler = cb;
        _handlerMutex.unlock();
    };

    bool listenMulticast(const IPAddress addr, uint16_t port, uint8_t ttl=1);

    size_t writeTo(const uint8_t *data, size_t len, const IPAddress addr, uint16_t port);

    IPAddress listenIP() {
        return _listenIP;
    };
    operator bool() {
        return _connected;
    };

    // do not call, used internally as callback from libuv's C callback
    void _DO_NOT_CALL_uv_on_read(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags);
    void _DO_NOT_CALL_async_cb();
    void _DO_NOT_CALL_timer_cb();

private:
    // _attemptWrite must be accessed from the uv loop.
    void _attemptWrite();
    // _doWrite must be accessed from the uv loop.
    void _doWrite(const uint8_t *data, size_t len, const IPAddress addr, uint16_t port);
};

#endif
