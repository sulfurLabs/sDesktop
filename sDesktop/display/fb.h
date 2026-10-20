//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: fb.h
//

#ifndef FB_H
#define FB_H 1

#include <stdint.h>
#include <stddef.h>

#ifdef __sulfur__
    #include <sys/fb.h>
#endif

typedef struct
{
    int fd;
    int width;
    int height;
    int stride;

    int bpp;
    int pixel_bytes;
    int red_offset;
    int green_offset;
    int blue_offset;

    unsigned int *pixels; //vram the screen
    unsigned int *back; // the backbuffer? or shadowbuffer how wayland calls it
} fb_backend_t;

int fbInit(fb_backend_t *fb);
void fbUnInit(fb_backend_t *fb);
void fbPutPixel(fb_backend_t *fb, int x, int y, uint32_t color);

//upload one region from shadow/backbuffer
void fbFlush(fb_backend_t *fb, int x, int y, int w, int h);
int fbPoll(fb_backend_t *fb);

//vram is uncached so our bytewise memcpy crawls there
// there is sse in sulfur but uhh i had problems and this is just simpler for now
static inline void fbCopyToVram(void *dstV, const void *srcV, size_t bytes)
{
    unsigned char *dst = (unsigned char *)dstV;
    const unsigned char *src = (const unsigned char *)srcV;

    while (bytes && ((uintptr_t)dst & 7))
    {
        *dst++ = *src++;
        bytes--;
    }

    uint64_t *d = (uint64_t *)dst;
    const uint64_t *s = (const uint64_t *)src;
    while (bytes >= 8)
    {
        *d++ = *s++;
        bytes -= 8;
    }

    dst = (unsigned char *)d;
    src = (const unsigned char *)s;

    //the rest goes normally
    while (bytes--) *dst++ = *src++;
}

#endif
