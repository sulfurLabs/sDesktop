//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: fb_sulfur.c
//

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../os.h"
#include "../log.h"
#include "fb.h"

int fbInit(fb_backend_t *fb)
{
    Log("setting up the sulfur framebuffer");

    fb_info_t info;
    unsigned long vaddr = 0;

    fb->fd = open(FB_DEV, O_RDWR);

    if (fb->fd < 0) return 0;
    if (ioctl(fb->fd, FB_IOCTL_GET_INFO, &info) < 0) return 0;
    if (ioctl(fb->fd, FB_IOCTL_MAP, &vaddr) < 0) return 0;

    fb->width = info.width;
    fb->height = info.height;
    fb->stride = info.pitch / 4;
    fb->pixels = (uint32_t *)vaddr;
    fb->bpp = 32;
    fb->pixel_bytes = 4;

    //explanation in linux fb
    fb->back = malloc((size_t)fb->width * fb->height * 4);
    if (!fb->back) return 0;

    memset(fb->back, 0, (size_t)fb->width * fb->height * 4);

    return 1;
}

void fbUnInit(fb_backend_t *fb)
{
    if (fb->back)
    {
        free(fb->back);
        fb->back = NULL;
    }
}

void fbPutPixel(fb_backend_t *fb, int x, int y, uint32_t color)
{
    if (
        x < 0 || x >= fb->width  ||
        y < 0 || y >= fb->height
    ) return;

    fb->back[y * fb->width + x] = color;
}

void fbFlush(fb_backend_t *fb, int x, int y, int w, int h)
{
    //clamp region to screen
    if (x < 0) {  w += x;  x = 0;  }
    if (y < 0) {  h += y;  y = 0;  }
    if (x + w > fb->width) w = fb->width - x;
    if (y + h > fb->height) h = fb->height - y;
    if (w <= 0 || h <= 0) return;

    //sulfur vram is 32bit
    // rows r plain memory
    // one full single copy
    if (x == 0 && w == fb->width && fb->stride == fb->width)
    {
        fbCopyToVram(
            fb->pixels + (size_t)y * fb->stride,
            fb->back + (size_t)y * fb->width,
            (size_t)h * fb->width * 4
        );

        return;
    }

    for (int row = 0; row < h; row++)
    {
        fbCopyToVram(
            fb->pixels + (y + row) * fb->stride + x,
            fb->back + (y + row) * fb->width + x,
            (size_t)w * 4
        );
    }
}

int fbPoll(fb_backend_t *fb)
{
    (void)fb;
    return 1;
}
