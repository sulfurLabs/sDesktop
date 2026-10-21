//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: window.h
//

#ifndef WINDOW_H
#define WINDOW_H 1

#include "../display/comp.h"

#define WINDOW_MAX 32 //i dont think if a maximum is good lol

typedef struct
{
    int id;
    int x;
    int y;
    int width;
    int height;
    int visible;

    uint32_t *buffer; // the apps backbuffer, like wayland wl_shm
    int stride;

    int dirty;
    int dx;
    int dy;
    int dw;
    int dh;
} window_t;

window_t *WindowCreate(int x, int y, int width, int height);
void WindowDestroy(window_t *win);

void WindowPutPixel(window_t *win, int x, int y, uint32_t color);
void WindowDrawRect(
    window_t *win,
    int x,
    int y,
    int width,
    int height,
    uint32_t color
);

void WindowDamage(window_t *win, int x, int y, int w, int h);
void WindowMove(window_t *win, int x, int y);
void WindowResize(window_t *win, int width, int height);
void WindowShow(window_t *win);
void WindowHide(window_t *win);

#endif
