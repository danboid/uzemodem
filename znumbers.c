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
#ifdef ZDEBUG
#include <stdio.h>
#endif

#include <stdbool.h>
#include "ztypes.h"
#include "znumbers.h"

ZRESULT zm_hex_to_nybble(char c1) {
  if (c1 >= '0' && c1 <= '9') return c1 - '0';
    if (c1 >= 'a' && c1 <= 'f') return c1 - 'a' + 10;
    if (c1 >= 'A' && c1 <= 'F') return c1 - 'A' + 10;
    return BAD_DIGIT;
}

ZRESULT zm_nybble_to_hex(uint8_t nybble) {
  if (nybble < 10) return nybble + '0';
  if (nybble < 16) return nybble - 10 + 'a';
  return OUT_OF_RANGE;
}

ZRESULT zm_byte_to_hex(uint8_t byte, uint8_t *buf) {
  uint16_t h1 = zm_nybble_to_hex(BMSN(byte));

  if (IS_ERROR(h1)) {
    return h1;
  } else {
    uint16_t h2 = zm_nybble_to_hex(BLSN(byte));

    if (IS_ERROR(h2)) {
      return h2;
    } else {
      *buf++ = (uint8_t)h1;
      *buf = (uint8_t)h2;

      return OK;
    }
  }
}

ZRESULT zm_hex_to_byte(unsigned char c1, unsigned char c2) {
  uint16_t n1,n2;

  n1 = zm_hex_to_nybble(c1);
  n2 = zm_hex_to_nybble(c2);

  if (n1 == BAD_DIGIT || n2 == BAD_DIGIT) {
    DEBUGF("Got bad digit: [0x%02x, 0x%02x]\n", c1, c2);
    return BAD_DIGIT;
  } else {
    return NTOB(n1,n2);
  }
}

