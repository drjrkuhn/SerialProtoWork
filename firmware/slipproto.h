#pragma once

#ifndef __SLIPPROTO_H__
    #define __SLIPPROTO_H__
    #include <cassert>

/*
 * SLIP encoded serial protocol
 *
 * Standard command/request format:
 *	Single letter code: ! for set, ? for query
 *	SLIP-escaped frame containing
 *		CBOR-encoded command or request
 *		CBOR-encoded parameters
 *		16-bit CRC CCITT format of non-escaped frame
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
 *		16-bit CRC CCITT format of non-escaped frame
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
 *		16-bit CRC CCITT format of non-escaped frame
 *	SLIP_END
 *
 */

constexpr char SLIP_END = '#';  //= 0300;   //** 0xC0   End of packet;
constexpr char SLIP_ESC = '\\'; // = 0333;     //** 0xDB   Escape character

constexpr char SLIP_ESC_END[]{SLIP_ESC, 'N'};
constexpr char SLIP_ESC_ESC[]{SLIP_ESC, 'E'};

// Use CRTP to implement static polymorphism
template <class D> // D is the derived type
class SlipProtocolBase {
 public:
    size_t writeSlipEscaped(const char* src, size_t src_size, bool writeEnd = false) {
        if (!isReady()) return 0;
        const char* end = src;
        size_t ntx      = 0; // total src buffer characters processed (NOT chars transmitted)

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
        if (writeEnd) {
            ntx += writeBytes(&SLIP_END, 1);
        }
        return ntx;
    }

    size_t readSlipEscaped(char* dest, size_t dest_size) {
        if (!isReady()) return 0;
        // leave room for SLIP_END at end of buffer
        size_t nread = readBytesUntil(dest, dest_size - 1, SLIP_END);
        if (nread == 0) {
            return 0;
        }
        Serial.print("\t> read ");
        Serial.println(nread);
        char* src        = dest;
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
        if (nrx > 0 && !misread) {
            dest[0] = SLIP_END;
            nrx++;
        }
        return nrx;
    }

    size_t writeBytes(const char* buffer, size_t size) {
        return static_cast<D*>(this)->writeBytes_impl(buffer, size);
    }

    size_t readBytesUntil(char* buffer, size_t size, char terminator) {
        return static_cast<D*>(this)->readBytesUntil_impl(buffer, size, terminator);
    }

    bool hasBytes() {
        return static_cast<D*>(this)->hasBytes_impl();
    }

    void writeNow() {
        static_cast<D*>(this)->writeNow_impl();
    }

    bool isReady() {
        return static_cast<D*>(this)->isReady_impl();
    }

 protected:
    /** Write several bytes to the output. The term character should be included
    in the buffer, or you can use a single writeByte() to write
    the term character. */
    size_t writeBytes_impl(const char* buffer, size_t size) {
        assert(false);
        return 0;
    }

    /** Read a string of bytes from the input UNTIL a terminator character is received, or a
    timeout occurrs. The terminator character is NOT added to the end of the buffer. */
    size_t readBytesUntil_impl(char* buffer, size_t size, char terminator) {
        assert(false);
        return 0;
    }

    bool hasBytes_impl() {
        assert(false);
        return false;
    }

    void writeNow_impl() { assert(false); }

    bool isReady_impl() {
        assert(false);
        return false;
    }
};

#endif // #ifndef __SLIPCRC_H__