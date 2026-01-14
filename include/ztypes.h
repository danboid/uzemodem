#ifndef __ZTYPES_H
#define __ZTYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef uint16_t ZRESULT;

// Result / Error Codes
#define OK              0x0000
#define CLOSED          0x0001
#define CORRUPTED       0x0002
#define BAD_CRC         0x0003
#define CANCELLED       0x0004
#define OUT_OF_SPACE    0x0005
#define BAD_DIGIT       0x0006
#define OUT_OF_RANGE    0x0007
#define BAD_ESCAPE      0x0008
#define BAD_FRAME_TYPE  0x0009

// ZMODEM Logic Macros
// Fixed IS_ERROR to allow (c = zm_recv()) assignment patterns
#define IS_ERROR(r)     ((r) > 0 && (r) < 0x0100)
#define IS_FIN(r)       ((r) >= 0x0101 && (r) <= 0x0104)
#define ZVALUE(r)       ((uint8_t)((r) & 0xFF))
#define IS_ZVALUE(r, v) (ZVALUE(r) == (v))
#define NONCONTROL(c)   ((c & 0xFF) >= 0x20 && (c & 0xFF) <= 0x7E)

// ZMODEM Characters
#define ZPAD            '*'
#define ZDLE            0x18
#define ZDLEE           (ZDLE ^ 0x40)
#define XON             0x11
#define XOFF            0x13
#define CAN             0x18  // Same as ZDLE
#define CR              0x0D
#define LF              0x0A

// ZMODEM Data Frame Endings (ZDLE + these)
#define ZCRCE           'h'  // CRC Next, Frame Ends
#define ZCRCG           'i'  // CRC Next, Frame Continues
#define ZCRCQ           'j'  // CRC Next, Frame Continues, Send ACK
#define ZCRCW           'k'  // CRC Next, Frame Ends, Send ACK
#define ZRUB0           'l'  // Translate to 0x7F
#define ZRUB1           'm'  // Translate to 0xFF

// Protocol Sizes
#define ZHDR_SIZE       7    // Type + 4 flags + 2 CRC
#define HEX_HDR_STR_LEN 20

// Data Frame Results
#define GOT_CRCE        0x0101
#define GOT_CRCG        0x0102
#define GOT_CRCQ        0x0103
#define GOT_CRCW        0x0104

#define DEBUGF(...)     ((void)0)

// Header Structure (Big-endian mapping)
typedef struct {
    uint8_t type;
    union {
        struct { uint8_t f3, f2, f1, f0; } flags;
        uint32_t position;
    };
    uint8_t crc1, crc2, crc3, crc4;
} ZHDR;

// ZMODEM Capability Flags for ZRINIT
#define CANFDX  0x01  // Can do Full Duplex
#define CANOVIO 0x02  // Can Overlap I/O (we use this to signal Petit FatFs readiness)
#define CANBRK  0x04  // Can send a break
#define CANCRY  0x08  // Can encrypt
#define CANLZW  0x10  // Can LZW compress
#define CANFC32 0x20  // Can use 32-bit CRC

ZRESULT zm_await_header(ZHDR *hdr);
ZRESULT zm_send_flags_hdr(uint8_t type, uint8_t f3, uint8_t f2, uint8_t f1, uint8_t f0);
ZRESULT zm_send_pos_hdr(uint8_t type, uint32_t pos);
ZRESULT zm_read_data_block(uint8_t *buf, uint16_t *len);

#endif
