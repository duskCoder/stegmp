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

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sysexits.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "bmp.h"

static int stegmp_parse_windows_bitmap_info_header(const unsigned char *addr,
        size_t size, struct bitmap *bmpp)
{
    struct bitmap_info_header info_header;

    if (size < sizeof(typeof(info_header))) {
        bmp_errno = BMP_EINVALSIZ;
        return -1;
    }

    info_header = *((typeof(info_header) *)addr);

    switch (info_header.bpp) {
        case 8:
        case 16:
        case 24:
        case 32:
            bmpp->bpp = info_header.bpp;
                break;
        default:
            bmp_errno = BMP_EINVALBPP;
            return -1;
    }

    /* TODO check the compression type */

    bmpp->width = info_header.bmp_width;
    bmpp->height = info_header.bmp_height;

    return 0;
}

/* guess which type of DIB header it is and handle it or not */
static int stegmp_parse_dib_header(const unsigned char *addr, size_t size,
        struct bitmap *bmpp)
{
    uint32_t dib_size;

    if (size < sizeof(typeof(dib_size))) {
        bmp_errno = BMP_EINVALSIZ;
        return -1;
    }

    dib_size = *((typeof(dib_size) *)addr);

    switch (dib_size) {
        case 12:
        case 52:
        case 56:
        case 64:
        case 108:
        case 124:
            bmp_errno = BMP_ENOTSUPP;
            return -1;

        case 40:
            return stegmp_parse_windows_bitmap_info_header(addr, size, bmpp);

        default:
            bmp_errno = BMP_ENOTBMP;
            return -1;
    }
}

/* parse the main header of the bmp file */
static int stegmp_parse_headers(const unsigned char *addr, size_t orig_size,
        struct bitmap *bmpp)
{
    struct packed_bmp_header packed_header;

    if (orig_size < sizeof(packed_header)) {
        bmp_errno = BMP_ENOTBMP;
        return -1;
    }

    packed_header = *((typeof(packed_header) *)addr);

    switch (packed_header.magic) {
        case BMP_MAGIC_BM:
            break;

        case BMP_MAGIC_BA:
        case BMP_MAGIC_CI:
        case BMP_MAGIC_CP:
        case BMP_MAGIC_IC:
        case BMP_MAGIC_PT:
            bmp_errno = BMP_ENOTSUPP;
            return -1;
        default:
            bmp_errno = BMP_ENOTBMP;
            return -1;
    }

    if (packed_header.bmp_size != orig_size) {
        bmp_errno = BMP_EINVALSIZ;
        return -1;
    }

    bmpp->data_offset = packed_header.data_offset;
    bmpp->size = packed_header.bmp_size;

    stegmp_parse_dib_header(addr + sizeof(packed_header), orig_size, bmpp);

    return 0;
}

/*
 * if `input' is NULL, we, want to read bmp_file
 */
static int stegmp(const char *bmp_file, FILE *input)
{
    void *addr;
    struct stat stat;
    struct bitmap bmp;
    FILE *fh = fopen(bmp_file, "r+");

    if (fh == NULL) {
        fprintf(stderr, "%s: %m\n", bmp_file);
        return -1;
    }

    fstat(fileno(fh), &stat);
    addr = mmap(NULL, stat.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
            fileno(fh), 0);

    if (addr == MAP_FAILED) {
        fprintf(stderr, "%s: %m\n", bmp_file);
        fclose(fh);
        return -1;
    }

    /* retrieve the required information (i.e. width, height, bpp .. ) */
    if (stegmp_parse_headers(addr, stat.st_size, &bmp) >= 0) {
        (void) input;

    } else {
        fprintf(stderr, "unable to parse headers: %s\n",
                bmp_strerror(bmp_errno));
    }

    munmap(addr, stat.st_size);
    fclose(fh);

    return 0;
}

static inline int stegmp_read(const char *bmp_file)
{
    return stegmp(bmp_file, NULL);
}

static int stegmp_write(const char *bmp_file, const char *in_file)
{
    if (strcmp(in_file, "-") == 0) {
        return stegmp(bmp_file, stdin);
    } else {
        int ret;
        FILE *fh = fopen(in_file, "r");

        if (fh == NULL) {
            fprintf(stderr, "%s: %m\n", in_file);
            return -1;
        }

        ret = stegmp(bmp_file, fh);

        fclose(fh);

        return ret;
    }
}

__attribute__((noreturn))
static void usage(const char *prgnam)
{
    fprintf(stderr, "usage: %s COMMAND\n", prgnam);
    fprintf(stderr, "\nCOMMAND\n");
    fprintf(stderr, "  read  BMP\n");
    fprintf(stderr, "  write BMP in_file\n");

    exit(EX_USAGE);
}

int main(int argc, char *argv[])
{
    /* we need a file passed as argument since we want to map it */
    if (argc < 3) {
        usage(argv[0]);
    }

    if (strcmp(argv[1], "read") == 0) {
        /* read */

        return stegmp_read(argv[2]);
    } else if (strcmp(argv[1], "write") == 0) {
        /* write */
        if (argc < 4) {
            usage(argv[0]);
        }

        return stegmp_write(argv[2], argv[3]);
    } else {
        usage(argv[0]);
    }
}
