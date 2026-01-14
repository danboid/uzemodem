/*
 *------------------------------------------------------------
 *                                  ___ ___ _
 *  ___ ___ ___ ___ ___       _____|  _| . | |_
 * |  _| . |_ -|  _| . |     |     | . | . | '_|
 * |_| |___|___|___|___|_____|_|_|_|___|___|_,_|
 *                     |_____|       firmware v1
 * ------------------------------------------------------------
 * Copyright (c)2020 Ross Bamford
 * See top-level LICENSE.md for licence information.
 *
 * Routines for working with Zmodem headers
 * ------------------------------------------------------------
 */

#ifndef __ROSCO_M68K_ZHEADERS_H
#define __ROSCO_M68K_ZHEADERS_H

#include <stdint.h>
#include "ztypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// The length of a ZMODEM Hex Header string:
// '*' + 'B' + (Type + F3 + F2 + F1 + F0 + CRC1 + CRC2 [all in hex]) + CR + LF + XON
// 1 + 1 + (2 * 7) + 1 + 1 + 1 = 19 characters (plus null terminator)
#define HEX_HDR_STR_LEN     20

// Standard ZMODEM Frame Types (Header Types)
#define ZRQINIT     0       /* Request receive init */
#define ZRINIT      1       /* Receive init */
#define ZSINIT      2       /* Send init sequence (optional) */
#define ZACK        3       /* ACK to above */
#define ZFILE       4       /* File name from sender */
#define ZSKIP       5       /* To skip the file */
#define ZNAK        6       /* Last packet was corrupted */
#define ZABORT      7       /* Abort sequence */
#define ZFIN        8       /* Finish session */
#define ZRPOS       9       /* Resume from this position */
#define ZDATA       10      /* Data packets to follow */
#define ZEOF        11      /* End of file */

// Header Formats
#define ZHEX        'B'     /* Hex Header */
#define ZBIN16      'A'     /* Binary 16-bit CRC Header */
#define ZBIN32      'C'     /* Binary 32-bit CRC Header */

// Error Codes (Add these if they aren't in ztypes.h)
#ifndef OUT_OF_SPACE
#define OUT_OF_SPACE  0x0005
#endif

// Prototypes
void zm_calc_hdr_crc(ZHDR *hdr);
ZRESULT zm_to_hex_header(ZHDR *hdr, uint8_t *buf, int max_len);

/*
 * One-shot calculate the CRC for a ZHDR and set the
 * crc1 and crc2 fields appropriately.
 */
void zm_calc_hdr_crc(ZHDR *hdr);

uint16_t zm_calc_data_crc(uint8_t *buf, uint16_t len);
uint32_t zm_calc_data_crc32(uint8_t *buf, uint16_t len);

#define CRC(msb, lsb)   (uint16_t)(((uint16_t)(msb) << 8) | (uint8_t)(lsb))

#define CRC32(b1, b2, b3, b4) ( \
    ((uint32_t)(b1) << 24) | \
    ((uint32_t)(b2) << 16) | \
    ((uint32_t)(b3) << 8)  | \
    ((uint32_t)(b4))         \
)

/*
 * Converts ZHDR to wire-format hex header. Expects CRC is already
 * computed. Result placed in the supplied buffer.
 *
 * The encoded header includes the 'B' header-type character and
 * trailing CRLF, but does not include other Zmodem control
 * characters (e.g. leading ZBUF/ZDLE etc).
 *
 * Returns actual used length (max 0xff bytes), or OUT_OF_SPACE
 * if the supplied buffer is not large enough.
 */
ZRESULT zm_to_hex_header(ZHDR *hdr, uint8_t *buf, int max_len);

ZRESULT zm_check_header_crc16(ZHDR *hdr, uint16_t crc);
ZRESULT zm_check_header_crc32(ZHDR *hdr, uint32_t crc);

#ifdef ZDEBUG
/* this is wasteful, but only if debugging is on, so, y'know... */
static char *__hdrtypes[] __attribute__((unused)) = {
    "ZRQINIT", "ZRINIT",     "ZSINIT",     "ZACK",
    "ZFILE",   "ZSKIP",      "ZNAK",       "ZABORT",
    "ZFIN",    "ZRPOS",      "ZDATA",      "ZEOF",
    "ZERR",    "ZCRC",       "ZCHALLENGE", "ZCOMPL",
    "ZCAN",    "ZFREECOUNT", "ZCOMMAND",   "ZSTDERR"
};

#define DEBUG_DUMPHDR_F(hdr)                    \
  DEBUGF("DEBUG: Header type [%s]:\n",          \
      __hdrtypes[hdr->type]);                   \
  DEBUGF("  type: 0x%02x\n", hdr->type);        \
  DEBUGF("    f0: 0x%02x\n", hdr->flags.f0);    \
  DEBUGF("    f1: 0x%02x\n", hdr->flags.f1);    \
  DEBUGF("    f2: 0x%02x\n", hdr->flags.f2);    \
  DEBUGF("    f3: 0x%02x\n", hdr->flags.f3);    \
  DEBUGF("  crc1: 0x%02x\n", hdr->crc1);        \
  DEBUGF("  crc2: 0x%02x\n", hdr->crc2);        \
  DEBUGF("   RES: 0x%02x\n", hdr->PADDING);     \
  DEBUGF("\n");

#define DEBUG_DUMPHDR_P(hdr)                    \
  DEBUGF("DEBUG: Header type [%s]:\n",          \
      __hdrtypes[hdr->type]);                   \
  DEBUGF("  type: 0x%02x\n", hdr->type);        \
  DEBUGF("    p0: 0x%02x\n", hdr->position.p0); \
  DEBUGF("    p1: 0x%02x\n", hdr->position.p1); \
  DEBUGF("    p2: 0x%02x\n", hdr->position.p2); \
  DEBUGF("    p3: 0x%02x\n", hdr->position.p3); \
  DEBUGF("  crc1: 0x%02x\n", hdr->crc1);        \
  DEBUGF("  crc2: 0x%02x\n", hdr->crc2);        \
  DEBUGF("   RES: 0x%02x\n", hdr->PADDING);     \
  DEBUGF("\n");

#define DEBUG_DUMPHDR_R(hdr)                       \
  DEBUGF("DEBUG: Header received  [%s]:\n",        \
      __hdrtypes[hdr->type]);                      \
  DEBUGF("  type: 0x%02x\n", hdr->type);           \
  DEBUGF("    p0/f3: 0x%02x\n", hdr->position.p0); \
  DEBUGF("    p1/f2: 0x%02x\n", hdr->position.p1); \
  DEBUGF("    p2/f1: 0x%02x\n", hdr->position.p2); \
  DEBUGF("    p3/f0: 0x%02x\n", hdr->position.p3); \
  DEBUGF("  crc1: 0x%02x\n", hdr->crc1);           \
  DEBUGF("  crc2: 0x%02x\n", hdr->crc2);           \
  DEBUGF("  crc3: 0x%02x\n", hdr->crc3);           \
  DEBUGF("  crc4: 0x%02x\n", hdr->crc4);           \
  DEBUGF("   RES: 0x%02x\n", hdr->PADDING);        \
  DEBUGF("\n");

#define DEBUG_DUMPHDR   DEBUG_DUMPHDR_F
#else
#define DEBUG_DUMPHDR_F(hdr)
#define DEBUG_DUMPHDR_P(hdr)
#define DEBUG_DUMPHDR_R(hdr)
#define DEBUG_DUMPHDR(hdr)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ROSCO_M68K_ZHEADERS_H */
