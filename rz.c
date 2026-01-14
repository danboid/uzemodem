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
 * Example usage of Zmodem implementation
 * Uzebox port by Dan MacDonald
 * ------------------------------------------------------------
 */

// TODO: Switch to fatfs, tidy up.

#include <stdbool.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <uzebox.h>
#include "zmodem.h"
#include <petitfatfs/pffconf.h>
#include <petitfatfs/diskio.h>
#include <petitfatfs/pff.h>

int16_t UartGetChar() {
    return UartReadChar();
}

void UartPutChar(uint8_t c) {
    UartSendChar(c);
}

// Reduced for ATmega644 RAM constraints
#define DATA_BUF_LEN    512

// Global file handle for Petit FatFs
FATFS fs;

/* * Uzebox UART Receive
 */
ZRESULT zm_await(char *str, char *buf, uint16_t len) {
    uint16_t matched = 0;
    uint16_t target_len = strlen(str);

    // We'll wait a certain amount of time before giving up
    // On Uzebox, we can use the Frame Count for timing
    uint32_t start_tick = GetVsyncCounter();

    while (true) {
        int16_t c = UartGetChar();

        if (c != -1) {
            uint8_t rx_char = (uint8_t)c;

            // Debug: Show character on screen to see what's happening
            // PrintChar(matched, 10, rx_char, false);

            if (rx_char == str[matched]) {
                matched++;
                if (matched == target_len) {
                    return OK;
                }
            } else {
                // If it doesn't match, reset the state machine
                matched = 0;
            }
        }

        // Timeout: If no 'rz' after ~10 seconds (600 frames)
        if (GetVsyncCounter() - start_tick > 600) {
            return CLOSED;
        }
    }
}

/* * Uzebox UART Send
 */
ZRESULT zm_send(uint8_t chr) {
    UartPutChar(chr);
    return OK;
}

void InitUartCustom(uint32_t baud) {
    // F_CPU is defined in your Makefile as 28636360UL
    uint16_t baud_setting = (F_CPU / (16 * baud)) - 1;
    UBRR0H = (uint8_t)(baud_setting >> 8);
    UBRR0L = (uint8_t)baud_setting;
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
}

void UpdateProgress(uint32_t received, char* filename) {
    char buf[20];

    // Display Filename
    Print(0, 3, PSTR("file: "));
    Print(6, 3, filename);

    // Display Received Bytes
    Print(0, 4, PSTR("recv: "));
    ultoa(received, buf, 10);
    Print(6, 4, buf);

    // Simple Progress Bar (Draws across row 6)
    uint8_t bar_width = (received / 1024) % 30;
    SetTile(bar_width, 6, 0x01); // Use a solid block tile from your charset
}

int main(void) {
    // For pf_write to work, we need a 'written' variable
    UINT written;
    uint8_t rzr_buf[4];
    uint8_t data_buf[DATA_BUF_LEN];
    uint16_t count;
    uint32_t received_data_size = 0;
    ZHDR hdr;
    bool file_open = false;

    // 1. Initialize the Kernel (Setup timers/interrupts)
    InitMusicPlayer(NULL);

    while (1) {
        // Clear screen and reset state for a new session
        ClearVram();
        received_data_size = 0;
        file_open = false;

        Print(0, 0, PSTR("uzebox zmodem receiver"));
        Print(0, 2, PSTR("awaiting zmodem sender"));

        // 2. Initialize SD Card (Attempt mount every loop in case card was swapped)
        if(pf_mount(&fs) != FR_OK) {
            Print(0, 4, PSTR("sd card mount failed!"));
            WaitVsync(60); // Wait 1 second before retrying
            continue;
        }

        // 3. Wait for "rz\r" handshake from PC
        if (zm_await("rz\r", (char*)rzr_buf, 4) == OK) {
            Print(0, 2, PSTR("handshake ok! negotiating..."));

            while (true) {
                uint16_t result = zm_await_header(&hdr);

                if (result == OK) {
                    switch (hdr.type) {
                        case ZRQINIT:
                            zm_send_flags_hdr(ZRINIT, CANOVIO, 0, 0, 0);
                            break;

                        case ZFILE:
                            count = DATA_BUF_LEN;
                            zm_read_data_block(data_buf, &count);

                            // Open the pre-existing container file
                            if(pf_open("UPLOAD.BIN") == FR_OK) {
                                Print(0, 3, PSTR("receiving: "));
                                Print(11, 3, (char*)data_buf);
                                file_open = true;
                                received_data_size = 0;
                                zm_send_pos_hdr(ZRPOS, 0);
                            } else {
                                Print(0, 3, PSTR("error: upload.bin not found"));
                                // Force exit this session
                                goto session_end;
                            }
                            break;

                        case ZDATA:
                            while (file_open) {
                                // Give the kernel a moment to process video interrupts
                                WaitVsync(1);

                                count = DATA_BUF_LEN;
                                result = zm_read_data_block(data_buf, &count);

                                if (!IS_ERROR(result)) {
                                    // Write data block to SD
                                    FRESULT res = pf_write(data_buf, count, &written);

                                    if(res != FR_OK) {
                                        Print(0, 5, PSTR("sd write error!"));
                                        break;
                                    }

                                    received_data_size += (count);
                                    UpdateProgress(received_data_size, "UPLOAD.BIN");

                                    if (result == GOT_CRCQ || result == GOT_CRCW) {
                                        zm_send_pos_hdr(ZACK, received_data_size);
                                    }
                                } else {
                                    zm_send_pos_hdr(ZRPOS, received_data_size);
                                    break;
                                }
                            }
                            break;

                        case ZEOF:
                            pf_write(0, 0, &written); // Finalize file
                            file_open = false;
                            Print(0, 7, PSTR("transfer complete."));
                            zm_send_flags_hdr(ZRINIT, CANOVIO, 0, 0, 0);
                            break;

                        case ZFIN:
                            zm_send_pos_hdr(ZFIN, 0);
                            Print(0, 8, PSTR("session closed."));
                            WaitVsync(120); // Show status for 2 seconds
                            goto session_end;
                    }
                } else {
                    zm_send_pos_hdr(ZNAK, received_data_size);
                }
            }
        }

        session_end:
        // Short pause before the while(1) loop restarts the listener
        WaitVsync(30);
    }

    return 0;
}
