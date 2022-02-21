#pragma once

#ifndef __ARDUINOSLIP_H__
    #define __ARDUINOSLIP_H__

    #include "slipproto.h"
    #include <Arduino.h>
    #include <Stream.h>

namespace sproto {

    template <typename S>
    class ArduinoSlipProtocol : public SlipProtocolBase<ArduinoSlipProtocol<S>> {
        typedef SlipProtocolBase<ArduinoSlipProtocol<S>> BASE;
        friend BASE;

     public:
        ArduinoSlipProtocol(S& stream) : stream_(stream), timeout_(999) {
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
        /** \copydoc SlipProtocolBase::writeBytes_ */
        size_t writeBytes_(const char* buffer, size_t size) {
            return stream_.write(buffer, size);
        }

        /** \copydoc SlipProtocolBase::readBytesUntil_ */
        error_t readBytesUntil_(char* buffer, const size_t size, const char terminator, size_t& nread) {
            // return Serial.readBytesUntil(terminator, buffer, size);
            unsigned long startMillis = millis();
            nread                     = stream_.readBytesUntil(terminator, buffer, size);
            if (millis() - startMillis >= timeout_) {
                return ERROR_TIMEOUT;
            }
            return NO_ERROR;
        }

        /** \copydoc SlipProtocolBase::hasBytes_ */
        bool hasBytes_() {
            return stream_.available();
        }

        /** \copydoc SlipProtocolBase::writeNow_ */
        void writeNow_() {
            stream_.flush();
            return true;
        }

        /** \copydoc SlipProtocolBase::clearInput_ */
        void clearInput_() {
            Serial.clear();
        }

        /** \copydoc SlipProtocolBase::isStreamReady_ */
        bool isStreamReady_() {
            return stream_;
        }


        S& stream_;             ///< Aruino stream to write to
        unsigned long timeout_; ///< Terminated read timeout in msec
    };

}; // namespace

#endif // #ifndef __ARDUINOSLIP_H__