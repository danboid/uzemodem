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
 * Numeric-related routines for Zmodem implementation.
 * ------------------------------------------------------------
 */

#ifndef __ROSCO_M68K_ZNUMBERS_H
#define __ROSCO_M68K_ZNUMBERS_H

#include <stdint.h>
#include "ztypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// Error codes for numeric conversion
#define BAD_DIGIT       0x0006
#define OUT_OF_RANGE    0x0007

// Macros for nybble and byte manipulation
#define BMSN(b) ((uint8_t)(((b) >> 4) & 0x0f))                // Bit Most Significant Nybble
#define BLSN(b) ((uint8_t)((b) & 0x0f))                       // Bit Least Significant Nybble
#define NTOB(n1, n2) ((uint8_t)((((n1) & 0x0f) << 4) | ((n2) & 0x0f))) // Nybbles TO Byte

ZRESULT zm_hex_to_nybble(char c1);

ZRESULT zm_nybble_to_hex(uint8_t nybble);

/*
 * *buf MUST have space for exactly two characters!
 *
 * Returns OK on success, or an error code.
 * If an error occues, the buffer will be unchanged.
 */
ZRESULT zm_byte_to_hex(uint8_t byte, uint8_t *buf);

ZRESULT zm_hex_to_byte(unsigned char c1, unsigned char c2);

#ifdef __cplusplus
}
#endif

#endif /* __ROSCO_M68K_ZNUMBERS_H */

