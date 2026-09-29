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

#endif
