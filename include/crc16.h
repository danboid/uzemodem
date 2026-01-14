/* crc16.h */

#ifndef __CRC16_H
#define __CRC16_H

#include <stdint.h>

#define CRC_START_XMODEM 0x0000

uint16_t ucrc16(uint8_t data, uint16_t crc);
uint16_t crc16(char* data, uint16_t len);

#endif
