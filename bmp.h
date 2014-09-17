/*
 * Copyright (C) 2014 Olivier Gayot <duskcoder@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef BMP_H
#define BMP_H

#include <endian.h>

#include <stdint.h>

struct packed_bmp_header {
    uint16_t magic;
    uint32_t bmp_size;
    uint16_t reserved;
    uint16_t reserved2;
    uint32_t data_offset;
} __attribute__((packed));

struct bitmap {
    uint32_t size;
    uint32_t data_offset;
    uint32_t width;
    uint32_t height;
    uint16_t bpp;
};

/* Windows BITMAPINFOHEADER */
struct bitmap_info_header {
    uint32_t header_size;
    uint32_t bmp_width;
    uint32_t bmp_height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t img_size;
    uint32_t horizontal_res;
    uint32_t vertical_res;
    uint32_t palette;
    uint32_t important_colors;
} __attribute((packed));

enum {
#if __BYTE_ORDER == __BIG_ENDIAN
    BMP_MAGIC_BM = 0x424D,
    BMP_MAGIC_BA = 0x4241,
    BMP_MAGIC_CI = 0x4349,
    BMP_MAGIC_CP = 0x4350,
    BMP_MAGIC_IC = 0x4943,
    BMP_MAGIC_PT = 0x5054,
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    BMP_MAGIC_BM = 0x4D42,
    BMP_MAGIC_BA = 0x4142,
    BMP_MAGIC_CI = 0x4943,
    BMP_MAGIC_CP = 0x5043,
    BMP_MAGIC_IC = 0x4349,
    BMP_MAGIC_PT = 0x5450,
#endif
};

enum {
    /* bmp header */
    BMP_ESUCCESS,
    BMP_ENOTBMP,
    BMP_ENOTSUPP,

    BMP_EINVALSIZ,

    BMP_EINVALBPP,

    BMP_ERRNO_COUNT,
};

const char *bmp_error_string[] = {
    [BMP_ESUCCESS] = "Success",
    [BMP_ENOTBMP] = "File is not a BMP",
    [BMP_ENOTSUPP] = "File format not supported",
    [BMP_EINVALSIZ] = "Inconsistent file size",
    [BMP_EINVALBPP] = "Invalid BPP value",
};

int bmp_errno = BMP_ESUCCESS;

static inline const char *bmp_strerror(int err)
{
    return bmp_error_string[err];
}

#endif /* BMP_H */
