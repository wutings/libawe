/*
 * hexdump.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include "awe/hexdump.h"

void awe_hexdump(const void *data, unsigned int len) {
    const uint8_t *_data = (const uint8_t *)data;
    unsigned int offset = 0;
    while (offset < len) {
        printf("0x%04x  ", offset);

        unsigned int n = len - offset;
        if (n > 16) {
            n = 16;
        }

        unsigned int i = 0;
        for (i = 0; i < 16; ++i) {
            if (i == 8) {
                printf(" ");
            }

            if (offset + i < len) {
                printf("%02x ", _data[offset + i]);
            } else {
                printf("   ");
            }
        }

        printf(" ");

        for (i = 0; i < n; ++i) {
            if (isprint(_data[offset + i])) {
                printf("%c", _data[offset + i]);
            } else {
                printf(".");
            }
        }

        printf("\n");

        offset += 16;
    }
}
