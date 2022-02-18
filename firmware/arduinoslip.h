#pragma once

#ifndef __ARDUINOSLIP_H__
    #define __ARDUINOSLIP_H__

    #include "slipproto.h"
    #include <Arduino.h>
    #include <Stream.h>

// SLIP encoded serial protocol

template <typename S>
class ArduinoSlipProtocol : public SlipProtocolBase<ArduinoSlipProtocol<S>> {
    typedef SlipProtocolBase<ArduinoSlipProtocol<S>> BASE;
    friend BASE;

 public:
    ArduinoSlipProtocol(S& stream) : stream_(stream) {
    }
    void begin() {
        stream_.begin(115200);
        while (!stream_) {
            ; // wait for serial port to connect. Needed for native USB port only
        }
    }

    void end() {
        stream_.end();
    }

 protected:
    size_t writeBytes_impl(const char* buffer, size_t size) {
        return stream_.write(buffer, size);
    }

    /** Read a string of bytes from the input UNTIL a terminator character is received, or a
    timeout occurrs. The terminator character is NOT added to the end of the buffer. */
    size_t readBytesUntil_impl(char* buffer, size_t size, char terminator) {
        return stream_.readBytesUntil(terminator, buffer, size);
    }

    bool hasBytes_impl() {
        return stream_.available();
    }

    void writeNow_impl() {
    #ifdef TEENSYDUINO
        stream_.send_now();
    #endif
        return true;
    }

    bool isReady_impl() {
        return stream_;
    }

    // send_now_fn send_now_fn_;
    S& stream_;
};

#endif // #ifndef __ARDUINOSLIP_H__