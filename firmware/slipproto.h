#pragma once

#ifndef __SLIPPROTO_H__
    #define __SLIPPROTO_H__
    #include <cassert>
    #include <cctype>

/*
 * SLIP encoded serial protocol
 *
 * Standard command/request format:
 *	Single letter code: ! for set, ? for query
 *	SLIP-escaped frame containing
 *		CBOR-encoded command or request
 *		CBOR-encoded parameters
 *		16-bit CRC CCITT/KERMIT format of non-escaped frame
 *	SLIP_END
 *
 * Simple command ACK/NAK
 *	Single letter: + for ACK, - for NAK
 *	SLIP_END
 *
 * Standard query ACK
 *	Single letter: + for ACK
 *	SLIP-escaped frame containing
 *		CBOR-encoded command echo
 *		CBOR-encoded parameters
 *		16-bit CRC CCITT/KERMIT format of non-escaped frame
 *	SLIP_END
 *
 * Simple query NAK
 *	Single letter: - for NAK
 *	SLIP_END (controller might resend request)
 *
 * Special command/request codes
 *	Single letter code: q for query
 *	SLIP-escaped frame containing
 *		CBOR-encoded device version
 *		CBOR-encoded device description
 *		16-bit CRC CCITT/KERMIT format of non-escaped frame
 *	SLIP_END
 *
 */

namespace sproto {
    constexpr uint8_t SLIP_END = '#';  //= 0300;   //** 0xC0   End of packet;
    constexpr uint8_t SLIP_ESC = '\\'; // = 0333;     //** 0xDB   Escape character

    constexpr uint8_t SLIP_ESC_END[]{SLIP_ESC, 'X'};
    constexpr uint8_t SLIP_ESC_ESC[]{SLIP_ESC, 'E'};

    typedef int error_t;

    constexpr error_t NO_ERROR       = 0;  ///< no error
    constexpr error_t ERROR_TIMEOUT  = -1; ///< stream timeout error
    constexpr error_t ERROR_BUFFER   = -2; ///< stream buffer error
    constexpr error_t ERROR_STREAM   = -3; ///< stream not ready error
    constexpr error_t ERROR_ENCODING = -4; ///< Protocol misread/miswrite error

    /**
     * @brief Base class for SLIP + CRC protocol communications
     *
     * @tparam D Derived class used for CRTP implementation of static polymorphism
     */
    template <class D> // D is the derived type
    class SlipProtocolBase {
     public:
        /**
         * @brief Write SLIP escaped buffer
         *
         * @param src       buffer to write
         * @param src_size  size of buffer to write
         * @param writeEnd  add the SLIP_ESC character to the end?
         * @return size_t   number of original un-escaped bytes written (NOT chars transmitted)
         */
        size_t writeSlipEscaped(const uint8_t* src, size_t src_size, bool write_end = false) {
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
            if (write_end) {
                ntx += writeBytes(&SLIP_END, 1);
            }
            return ntx;
        }

        /** \copydoc SlipProtocolBase::writeSlipEscaped */
        size_t writeSlipEscaped(const char* src, size_t src_size, bool write_end = false) {
            return writeSlipEscaped(reinterpret_cast<const uint8_t*>(src), src_size, write_end);
        }

        error_t readSlipEscaped(uint8_t* dest, size_t dest_size, size_t& nread, bool add_end = false) {
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
            if (add_end) {
                dest[0] = SLIP_END;
                nread++;
            }
            return NO_ERROR;
        }

        /** \copydoc SlipProtocolBase::readSlipEscaped */
        error_t readSlipEscaped(char* dest, size_t dest_size, size_t& nread, bool add_end = false) {
            return readSlipEscaped(reinterpret_cast<uint8_t*>(dest), dest_size, nread, add_end);
        }

        /**
         * @brief Writes characters contained in buffer to stream.
         *
         * @param buffer buffer to write
         * @param size size of buffer to write
         * @returns number of characters written to the stream
         */
        size_t writeBytes(const uint8_t* buffer, size_t size) {
            return static_cast<D*>(this)->writeBytes_impl(buffer, size);
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

        void crcKermitReset() {
            return static_cast<D*>(this)->crcKermitReset_impl();
        }

        uint16_t crcKermitCalc(const uint8_t* src, size_t size) {
            return static_cast<D*>(this)->crcKermitCalc_impl(src, size);
        }
    };

}; // namespace sproto

#endif // #ifndef __SLIPCRC_H__