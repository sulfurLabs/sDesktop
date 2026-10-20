//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: fb_linux.c
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "../os.h"
#include "../log.h"
#include "fb.h"

int fbInit(fb_backend_t *fb)
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    //i noticed it doesnt mount devfs by default so
    // i do it
    mkdir("/dev", 0755);
    mount("devtmpfs", "/dev", "devtmpfs", 0, NULL);

    int tries = 0; // we wait cuz it got glitchy otherwise when i tested it...
    int fd = open(FB_DEV, O_RDWR);
    for (tries = 0; fd < 0 && tries < 50; tries++)
    {
        usleep(100000);

        fd = open(FB_DEV, O_RDWR);
    }

    fb->fd = fd;

    if (fb->fd < 0) return 0;
    if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &finfo) < 0) return 0;
    if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &vinfo) < 0) return 0;

    fb->width = vinfo.xres;
    fb->height = vinfo.yres;
    fb->bpp = vinfo.bits_per_pixel;
    fb->pixel_bytes = (fb->bpp + 7) / 8;
    fb->red_offset = vinfo.red.offset;
    fb->green_offset = vinfo.green.offset;
    fb->blue_offset = vinfo.blue.offset;
    fb->stride = finfo.line_length / fb->pixel_bytes;

    fb->pixels = mmap(
        NULL,
        (size_t)finfo.line_length * vinfo.yres,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fb->fd,
        0
    );

    if (fb->pixels == MAP_FAILED)
    {
        fb->pixels = NULL;
        return 0;
    }

    //the desktop draws into shadow buffer first
    // then it comes to screen after some time
    //
    // drawing straight into vram is slow as hell cuz its uncached
    // so we draw into ram first (fast) and copy it over to the
    // screen all at once in fbFlush with the hope of it being fast...
    fb->back = malloc((size_t)fb->width * fb->height * 4);
    if (!fb->back) return 0;

    memset(fb->back, 0, (size_t)fb->width * fb->height * 4);

    //if the screen is black we can still see if its running
    printf(
        "fb %dx%d bpp %d stride %d rgb %d/%d/%d\n",
        fb->width,
        fb->height,
        fb->bpp,
        fb->stride,

        fb->red_offset,
        fb->green_offset,
        fb->blue_offset
    );

    return 1;
}

void fbUnInit(fb_backend_t *fb)
{
    if (fb->back)
    {
        free(fb->back);
        fb->back = NULL;
    }

    if (fb->pixels)
    {
        munmap(
            fb->pixels,
            (size_t)fb->stride * fb->height * fb->pixel_bytes
        );

        fb->pixels = NULL;
    }
}

void fbPutPixel(fb_backend_t *fb, int x, int y, uint32_t color)
{
    if (
        x < 0 || x >= fb->width  ||
        y < 0 || y >= fb->height
    ) return;

    //canpnical XRGB32
    fb->back[y * fb->width + x] = color;
}

//this should write the shadow pixel in a row
static void fbBlitPixel(
    fb_backend_t *fb,
    uint8_t *row,
    int x,
    uint32_t color
){
    //linux just being complicated.... ._.
    uint32_t r = (color >> 16) & 0xff;
    uint32_t g = (color >> 8) & 0xff;
    uint32_t b = color & 0xff;

    if (fb->bpp == 16)
    {
        *(uint16_t *)(row + (size_t)x * 2) = (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
        return;
    }

    uint32_t v = (r << fb->red_offset) | (g << fb->green_offset) | (b << fb->blue_offset);

    if (fb->bpp == 24)
    {
        row[(size_t)x * 3] = (uint8_t)v;
        row[(size_t)x * 3 + 1] = (uint8_t)(v >> 8);
        row[(size_t)x * 3 + 2] = (uint8_t)(v >> 16);

        return;
    }

    *(uint32_t *)(row + (size_t)x * 4) = v;
}

void fbFlush(fb_backend_t *fb, int x, int y, int w, int h)
{
    //clamps the region to screen
    if (x < 0) {  w += x; x = 0;  }
    if (y < 0) {  h += y; y = 0;  }
    if (x + w > fb->width) w = fb->width - x;
    if (y + h > fb->height) h = fb->height - y;
    if (w <= 0 || h <= 0) return;

    //memcpy in plain rows
    if (
        fb->bpp == 32
        && fb->red_offset == 16
        && fb->green_offset == 8
        && fb->blue_offset == 0
        && fb->pixel_bytes == 4
    ){
        //this is one full widht single copy
        if (x == 0 && w == fb->width && fb->stride == fb->width)
        {
            fbCopyToVram(
                (uint8_t *)fb->pixels + (size_t)y * (size_t)fb->stride * 4,
                fb->back + (size_t)y * fb->width,
                (size_t)h * fb->width * 4
            );

            return;
        }

        for (int row = 0; row < h; row++)
        {
            fbCopyToVram(
                (uint8_t *)fb->pixels + (size_t)(y + row) * (size_t)fb->stride * 4 + (size_t)x * 4,
                fb->back + (y + row) * fb->width + x,
                (size_t)w * 4
            );
        }
        return;
    }

    //qemu gives 24bits
    if (fb->bpp == 24)
    {
        for (int row = 0; row < h; row++)
        {
            uint8_t *dst = (uint8_t *)fb->pixels + (size_t)(y + row) * (size_t)fb->stride * (size_t)fb->pixel_bytes;
            unsigned int *src = fb->back + (y + row) * fb->width + x;

            for (int col = 0; col < w; col++)
            {
                uint32_t color = src[col];
                uint32_t r = (color >> 16) & 0xff;
                uint32_t g = (color >> 8) & 0xff;
                uint32_t b = color & 0xff;
                uint32_t v = (r << fb->red_offset) | (g << fb->green_offset) | (b << fb->blue_offset);

                dst[(size_t)col * 3] = (uint8_t)v;
                dst[(size_t)col * 3 + 1] = (uint8_t)(v >> 8);
                dst[(size_t)col * 3 + 2] = (uint8_t)(v >> 16);
            }
        }

        return;
    }

    //converts pixel by pixel
    for (int row = 0; row < h; row++)
    {
        uint8_t *dst = (uint8_t *)fb->pixels + (size_t)(y + row) * (size_t)fb->stride * (size_t)fb->pixel_bytes;
        unsigned int *src = fb->back + (y + row) * fb->width + x;

        for (int col = 0; col < w; col++) fbBlitPixel(fb, dst, x + col, src[col]);
    }
}

int fbPoll(fb_backend_t *fb)
{
    (void)fb;
    return 1;
}
