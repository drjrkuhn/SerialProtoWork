#pragma once

#ifndef __SLIPPROTO_H__
    #define __SLIPPROTO_H__
    #include <cassert>
    #include <cbor.h>
    #include <cctype>
    #include <compilersupport_p.h> // cbor_htons etc

/**
 * @page slipprot
 * SLIP encoded serial protocol
 * ============================
 *
 * Note 16-bit CRC is encoded in network byte order (big endian)
 *
 * Standard command/request format:
 * @code
 *	Single letter code: ! for set, ? for query
 *	SLIP-escaped frame containing
 *		CBOR-encoded command or request
 *		CBOR-encoded parameters
 *		16-bit CRC CCITT/KERMIT format of non-escaped frame
 *	SLIP_END
 * @endcode
 * |>cmd|command|crc-16|end|
 * |----|-------|---|---|
 * |! / ?|CBOR-encoded packet|HI LO|END|
 *
 * Simple command ACK/NAK
 * @code
 *	Single letter: + for ACK, - for NAK
 *	SLIP_END
 * @endcode
 * |<ACK/NAK|end|
 * |----|---|
 * |+/-|END|
 *
 * Standard query ACK
 * @code
 *	Single letter: + for ACK
 *	SLIP-escaped frame containing
 *		CBOR-encoded command echo
 *		CBOR-encoded parameters
 *		16-bit CRC CCITT/KERMIT format of non-escaped frame
 *	SLIP_END
 * @endcode
 * |<ACK|response|crc-16|end|
 * |----|-------|---|---|
 * |  + |CBOR-encoded packet|HI LO|END|
 *
 * Simple query NAK
 * @code
 *	Single letter: - for NAK
 *	SLIP_END (controller might resend request)
 * @endcode
 * |<NAK|end|
 * |----|---|
 * | - |END|
 *
 * Special command/request codes
 * @code
 * SEND
 *	Single letter code: q for query
 * RESPONSE
 *	Single letter: + for ACK
 *	SLIP-escaped frame containing
 *		CBOR-encoded device version
 *		CBOR-encoded device description
 *		16-bit CRC CCITT/KERMIT format of non-escaped frame
 *	SLIP_END
 * @endcode
 *
 */

namespace sproto {
    // for now, we are using human-readable escape and end characters rather than the SLIP default
    // for debugging
    constexpr uint8_t SLIP_END = '#';  //= 0300;   //** 0xC0   End of packet;
    constexpr uint8_t SLIP_ESC = '\\'; // = 0333;     //** 0xDB   Escape character

    constexpr uint8_t SLIP_ESC_END[]{SLIP_ESC, 'X'};
    constexpr uint8_t SLIP_ESC_ESC[]{SLIP_ESC, 'E'};

    constexpr uint8_t PROTO_SET   = '!';
    constexpr uint8_t PROTO_GET   = '?';
    constexpr uint8_t PROTO_ACK   = '+';
    constexpr uint8_t PROTO_NAK   = '-';
    constexpr uint8_t PROTO_QUERY = 'q';
    constexpr uint8_t PROTO_RESET = 'r';

    typedef int error_t;

    constexpr error_t NO_ERROR       = 0;  ///< no error
    constexpr error_t ERROR_TIMEOUT  = -1; ///< stream timeout error
    constexpr error_t ERROR_BUFFER   = -2; ///< stream buffer error
    constexpr error_t ERROR_STREAM   = -3; ///< stream not ready error
    constexpr error_t ERROR_ENCODING = -4; ///< Protocol misread/miswrite error

    /**
     * @brief Holds a structured protocol packet and maintains SLIP+CRC encoding
     * 
     * Encoding
     * -----------------------
     * 
     * | packet   | CRC-16    | END    |
     * |---------:|-----------|--------|
     * |...SLIP encoded...   [tail]| 2-4 bytes | 1 byte |
     * 
     * During creation, the buffer always maintains slip encoding with an escaped 16-bit CRC and SLIP_END.
     * The CCITT (kermit) 16bit CRC is stored in network (big endian) byte order. The two CRC bytes are
     * also SLIP encoded in case they contain the END or ESC characters. With the SLIP_END character,
     * the terminator can be 3, 4, or 5 bytes long.
     * 
     * The tail_ pointer points to the beginning of the CRC. New data will be slip encoded and amended
     * to the tail_. Followed by the running CRC and END terminator.
     * 
     * 
     */

    template <typename D>
    struct CRTP
    {
        D& derived() { return static_cast<D&>(*this); }
        D const& derived() const { return static_cast<D const&>(*this); }
    };

    // class ProtoPacket {
    //  public:
    //     ProtoPacket(uint8_t* buffer, size_t size)
    //         : ProtoPacket(buffer, buffer + size) {}

    //     ProtoPacket(uint8_t* begin, uint8_t* end)
    //         : head_(begin), tail_(begin), end_(end), current_crc_(0) {}

    //     size_t Encode(uint8_t* buffer, size_t size) {
    //         Encode(buffer, buffer+size);
    //     }
    //     size_t Encode(uint8_t* begin, uint8_t* end) {

    //     }

    //  protected:
    //     uint8_t* head_; ///< absolute start of buffer
    //     uint8_t* tail_; ///< start of next free space, points to CRC in network order
    //     uint8_t* end_; ///< absolute end of buffer
    //     uint16_t current_crc_;
    //     CRC crc_;
    // };

    /**
     * @brief Base class for SLIP + CRC protocol communications
     *
     * @tparam D Derived class used for CRTP implementation of static polymorphism
     */
    template <class D> // D is the derived type
    class SlipProtocolBase : CRTP<D> {
     public:
        SlipProtocolBase(bool use_crc)
            : use_crc_(use_crc) {
        }

        /**
         * @brief Write SLIP escaped buffer.
         *
         * @param src       buffer to write
         * @param src_size  size of buffer to write
         * @return size_t   number of original un-escaped bytes written (NOT chars transmitted)
         */
        size_t writeSlipEscaped(const uint8_t* src, size_t src_size) {
            if (!isStreamReady())
                return 0;
            const uint8_t* end = src;
            size_t ntx         = 0; // total src buffer characters processed (NOT chars transmitted)

            while (src_size--) {
                switch (end[0]) {
                    case SLIP_END:
                        if (0 < end - src) {
                            ntx += writeBytes(src, end - src);
                        }
                        if (writeBytes(SLIP_ESC_END, 2) == 2) {
                            ntx++; // processed one escape character
                        }
                        end++; // skip escaped char
                        src = end;
                        break;
                    case SLIP_ESC:
                        if (0 < end - src) {
                            ntx += writeBytes(src, end - src);
                        }
                        if (writeBytes(SLIP_ESC_ESC, 2) == 2) {
                            ntx++; // processed one escape character
                        }
                        end++; // skip escaped char
                        src = end;
                        break;
                    default:
                        end++;
                }
            }
            // write any remaining characters
            if (0 < end - src) {
                ntx += writeBytes(src, end - src);
            }
            return ntx;
        }

        /**
         * @brief UTF8 character version
         */
        size_t writeSlipEscaped(const char* src, size_t src_size) {
            return writeSlipEscaped(reinterpret_cast<const uint8_t*>(src), src_size);
        }

        size_t writeSlipEnd() {
            return writeBytes(&SLIP_END, 1);
        }

        size_t writeSlipEnd(uint16_t crc) {
            crc      = cbor_htons(crc);
            size_t n = writeSlipEscaped(reinterpret_cast<const uint8_t*>(&crc), sizeof(uint16_t));
            return n + writeSlipEnd();
        }

        /**
         * @brief Read SLIP escaped sequence from stream into buffer and remove escapes.
         *
         * Looks for standard SLIP END character.
         *
         * @param dest      destination buffer to fill
         * @param dest_size size of destination buffer (should be large enough to read escaped stream)
         * @param nread     number of bytes read after encoding
         * @return
         *  - ERROR_TIMEOUT timeout occurred before terminating character was found
         *  - ERROR_BUFFER  read buffer too small
         *  - ERROR_ENCODING slip stream was improperly encoded
         *  - NO_ERROR      terminator found and read complete
         */
        error_t readSlipEscaped(uint8_t* dest, size_t dest_size, size_t& nread) {
            if (!isStreamReady())
                return ERROR_STREAM;
            // leave room for SLIP_END at end of buffer
            error_t err = readBytesUntil(dest, dest_size - 1, SLIP_END, nread);
            if (err != NO_ERROR) {
                return err;
            }
            if (nread == 0) {
                return ERROR_TIMEOUT;
            }
            uint8_t* src     = dest;
            size_t remaining = nread;
            size_t nrx       = 0;
            bool misread     = false;
            while (remaining--) {
                if (src[0] == SLIP_ESC) {
                    if (remaining > 0 && src[1] == SLIP_ESC_END[1]) {
                        dest[0] = SLIP_END;
                        src++;
                    } else if (remaining > 0 && src[1] == SLIP_ESC_ESC[1]) {
                        dest[0] = SLIP_ESC;
                        src++;
                    } else {
                        dest[0] = SLIP_ESC;
                        misread = true;
                    }
                } else {
                    dest[0] = src[0];
                }
                src++;
                dest++;
                nrx++;
            }
            nread = nrx;
            if (nrx == 0 || misread) {
                return ERROR_ENCODING;
            }
            return NO_ERROR;
        }

        /** @brief UTF8 character version */
        error_t readSlipEscaped(char* dest, size_t dest_size, size_t& nread) {
            return readSlipEscaped(reinterpret_cast<uint8_t*>(dest), dest_size, nread);
        }

        /**
         * @brief Writes characters contained in buffer to stream.
         *
         * @param buffer buffer to write
         * @param size size of buffer to write
         * @returns number of characters written to the stream
         */
        size_t writeBytes(const uint8_t* buffer, size_t size) {
            // return static_cast<D*>(this)->writeBytes_impl(buffer, size);
            derived().writeBytes_impl(buffer, size);
        }

        /**
         * @brief Read a string of bytes from the input UNTIL a terminator character is received
         *  or a timeout occurs. The terminator character is NOT added to the end of the buffer.
         *  timeout period is implementation defined.
         *
         * @param buffer    buffer to read. Buffer is not necessarily null terminated after read
         * @param size      total size of buffer to read
         * @param terminator terminator character to look for
         * @param[out] nread number of characters read
         * @returns
         *  - ERROR_TIMEOUT timeout occurred before terminating character was found
         *  - ERROR_BUFFER  read buffer too small
         *  - NO_ERROR      terminator found and read complete
         */
        error_t readBytesUntil(uint8_t* buffer, size_t size, char terminator, size_t& nread) {
            return static_cast<D*>(this)->readBytesUntil_impl(buffer, size, terminator, nread);
        }

        /**
         * @brief Does the stream have bytes in the receive buffer?
         *
         * @return true if input received
         * @return false if no input detected
         */
        bool hasBytes() {
            return static_cast<D*>(this)->hasBytes_impl();
        }

        /**
         * @brief flush (write) the contents of the transmit buffer immediately.
         * Required by Teensy USB implementation if transmission is less than the
         * 64 byte USB buffer size.
         *
         */
        void writeNow() {
            static_cast<D*>(this)->writeNow_impl();
        }

        /**
         * @brief clear (flush) the contents of the receive buffer immediately.
         */
        void clearInput() {
            static_cast<D*>(this)->clearInput_impl();
        }

        /**
         * @brief Is the stream ready for transmission and reception? Usually set to
         * true after startup
         *
         * @return true     stream is ready in both directions
         * @return false    stream has not yet been started
         */
        bool isStreamReady() {
            return static_cast<D*>(this)->isStreamReady_impl();
        }

        /**
         * @brief reset the KERMIT CRC16 seed
         */
        void crcKermitReset() {
            return static_cast<D*>(this)->crcKermitReset_impl();
        }

        /**
         * @brief Calculate the KERMIT CRC16 value of a buffer.
         * Use @ref crcKermitReset() before sending more data.
         * Must calculate CRC-CCITT (True CCITT) value, compatible with the kermit protocol.
         *
         * @param src buffer to calculate
         * @param size number of bytes in buffer
         * @return calculated CRC16 value
         */
        uint16_t crcKermitCalc(const uint8_t* src, size_t size) {
            return static_cast<D*>(this)->crcKermitCalc_impl(src, size);
        }

        uint16_t crcKermitCalc(const char* src, size_t size) {
            return crcKermitCalc(reinterpret_cast<const uint8_t*>(src), size);
        }

        bool use_crc_;
    };

}; // namespace sproto

#endif // #ifndef __SLIPCRC_H__