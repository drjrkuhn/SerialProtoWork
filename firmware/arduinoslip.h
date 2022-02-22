#pragma once

#ifndef __ARDUINOSLIP_H__
    #define __ARDUINOSLIP_H__

    #include "slipproto.h"
    #include <Arduino.h>
    #include <FastCRC.h>
    #include <Stream.h>

namespace sproto {

    /**
     * @brief Arduino/Teensy specific SLIP + CRC protocol implementation.
     *
     * @tparam S Stream class to use. Usually <Serial>, <Serial1>, <Serial2>, etc.
     */
    template <class S>
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
        /**
         * \copydoc SlipProtocolBase::writeBytes
         * CRTP implementation.
         */
        size_t writeBytes_impl(const uint8_t* buffer, size_t size) {
            return stream_.write(reinterpret_cast<const char*>(buffer), size);
        }

        /**
         * \copydoc SlipProtocolBase::readBytesUntil
         * CRTP implementation.
         */
        error_t readBytesUntil_impl(uint8_t* buffer, const size_t size, const char terminator, size_t& nread) {
            const unsigned long startMillis = millis();
            // Arduino specific implementation. Could have its own timeout.
            // Kludg: Keep our timeout shorter.
            // calls Serial.readBytesUntil(terminator, buffer, size) or similar.
            nread = stream_.readBytesUntil(terminator, reinterpret_cast<char*>(buffer), size);
            if (millis() - startMillis >= timeout_) {
                return ERROR_TIMEOUT;
            }
            return NO_ERROR;
        }

        /**
         * \copydoc SlipProtocolBase::hasBytes
         * CRTP implementation.
         */
        bool hasBytes_impl() {
            return stream_.available();
        }

        /**
         * \copydoc SlipProtocolBase::writeNow
         * CRTP implementation.
         */
        void writeNow_impl() {
            stream_.flush();
            return true;
        }

        /**
         * \copydoc SlipProtocolBase::clearInput
         */
        void clearInput_impl() {
            Serial.clear();
        }

        /**
         * \copydoc SlipProtocolBase::isStreamReady
         * CRTP implementation
         */
        bool isStreamReady_impl() {
            return stream_;
        }

        /**
         * \copydoc SlipProtocolBase::crcKermitReset
         * CRTP implementation
         */
        void crcKermitReset_impl() {
            crc_.kermit(NULL, 0);
        }

        /**
         * \copydoc SlipProtocolBase::crcKermitCalc
         * CRTP implementation
         */
        uint16_t crcKermitCalc_impl(const uint8_t* src, size_t size) {
            return crc_.kermit_upd(src, size);
        }

        S& stream_;             ///< Aruino stream to write to
        unsigned long timeout_; ///< Terminated read timeout in msec
        FastCRC16 crc_;
    };

}; // namespace

#endif // #ifndef __ARDUINOSLIP_H__