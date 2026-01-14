#ifndef __ZSERIAL_H
#define __ZSERIAL_H

#include "ztypes.h"

// These are implemented in your main.c or a separate uart.c
int16_t zm_recv();
ZRESULT zm_send(uint8_t chr);

// Helper for strings
ZRESULT zm_await(char *str, char *buf, uint16_t buf_size);

// Debugging macros - redirected to nothing to save space on Uzebox
#define TRACEF(...)

#endif
