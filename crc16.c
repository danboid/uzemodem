/* crc16.c */

/* CCITT 16-bit CRC table and calculation function */

#include <avr/pgmspace.h>
#include "crc16.h"

// Move the table to Flash memory (PROGMEM)
const uint16_t crc16tbl[] PROGMEM = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    // ... [Include all 256 values here] ...
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

// Optimized for Uzebox: inline the calculation
uint16_t ucrc16(uint8_t data, uint16_t crc) {
    // We must use pgm_read_word to access the table from Flash
    return pgm_read_word(&crc16tbl[((crc >> 8) ^ data) & 0xFF]) ^ (crc << 8);
}

uint16_t crc16(char* data, uint16_t len) {
    uint16_t crc = 0;
    for(uint16_t i = 0; i < len; i++) {
        crc = ucrc16(data[i], crc);
    }
    return crc;
}
