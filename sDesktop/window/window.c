//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: window.c
//

#include <string.h>
#include <stdlib.h>

#include "window.h"
#include "../log.h"

window_t *WindowCreate(int x, int y, int width, int height)
{
    window_t *win;

    if (width <= 0 || height <= 0) return NULL;

    win = malloc(sizeof(window_t));
    if (!win) return NULL;

    memset(win, 0, sizeof(*win));

    win->id = -1;
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->visible = 1;

    win->buffer = malloc((size_t)width * height * 4);
    if (!win->buffer)
    {
        free(win);
        return NULL;
    }

    win->stride = width;
    memset(win->buffer, 0, (size_t)width * height * 4);

    //the whole window is new so damage it
    win->dirty = 1;
    win->dx = 0;
    win->dy = 0;
    win->dw = width;
    win->dh = height;

    return win;
}

void WindowDestroy(window_t *win)
{
    if (!win) return;

    if (win->buffer)
    {
        free(win->buffer);
        win->buffer = NULL;
    }

    free(win);
}

void WindowPutPixel(window_t *win, int x, int y, uint32_t color)
{
    if (!win) return;
    if (!win->buffer) return;
    if (x < 0 || x >= win->width) return;
    if (y < 0 || y >= win->height) return;

    win->buffer[y * win->stride + x] = color;
    WindowDamage(win, x, y, 1, 1);
}

//this as a test for the window buffer
// cuz we dont have a libdesktop yet
void WindowDrawRect(
    window_t *win,
    int x,
    int y,
    int width,
    int height,
    uint32_t color
){
    if (!win) return;
    if (!win->buffer) return;
    if (width <= 0 || height <= 0) return;
    if (x < 0) { width += x; x = 0; }
    if (y < 0) { height += y; y = 0; }
    if (x + width > win->width) width = win->width - x;
    if (y + height > win->height) height = win->height - y;
    if (width <= 0 || height <= 0) return;

    for (int py = 0; py < height; py++)
    {
        uint32_t *row = win->buffer + (size_t)(y + py) * win->stride + x;
        for (int px = 0; px < width; px++) row[px] = color;
    }

    WindowDamage(win, x, y, width, height);
}

void WindowDamage(window_t *win, int x, int y, int w, int h)
{
    int ex;
    int ey;
    int nx;
    int ny;

    if (w <= 0 || h <= 0) return;

    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > win->width) w = win->width - x;
    if (y + h > win->height) h = win->height - y;
    if (w <= 0 || h <= 0) return;

    if (!win->dirty)
    {
        win->dirty = 1;
        win->dx = x;
        win->dy = y;
        win->dw = w;
        win->dh = h;
        return;
    }

    nx = win->dx < x ? win->dx : x;
    ny = win->dy < y ? win->dy : y;
    ex = win->dx + win->dw > x + w ? win->dx + win->dw : x + w;
    ey = win->dy + win->dh > y + h ? win->dy + win->dh : y + h;

    win->dx = nx;
    win->dy = ny;
    win->dw = ex - nx;
    win->dh = ey - ny;
}

void WindowMove(window_t *win, int x, int y)
{
    if (!win) return;

    win->x = x;
    win->y = y;

    //whole window moved so damage everything on screen
    win->dirty = 1;
    win->dx = 0;
    win->dy = 0;
    win->dw = win->width;
    win->dh = win->height;
}

void WindowResize(window_t *win, int width, int height)
{
    if (!win) return;
    if (width <= 0 || height <= 0) return;

    uint32_t *newbuf = malloc((size_t)width * height * 4);
    if (!newbuf) return;

    int cpyw = width < win->width ? width : win->width;
    int cpyh = height < win->height ? height : win->height;

    for (int y = 0; y < cpyh; y++)
    {
        memcpy(
            newbuf + (size_t)y * width,
            win->buffer + (size_t)y * win->stride,
            (size_t)cpyw * 4
        );
    }

    free(win->buffer);

    win->buffer = newbuf;
    win->stride = width;
    win->width = width;
    win->height = height;

    memset(
        newbuf + (size_t)cpyh * width,
        0,
        (size_t)(width * height - cpyh * width) * 4
    );

    win->dirty = 1;
    win->dx = 0;
    win->dy = 0;
    win->dw = width;
    win->dh = height;
}

//also needs a event when its back so the
// ui system knows when it needs to draw again
void WindowShow(window_t *win)
{
    if (!win) return;

    win->visible = 1;
    win->dirty = 1;
    win->dx = 0;
    win->dy = 0;
    win->dw = win->width;
    win->dh = win->height;
}

//the ui lib needs to get a event when a window is hidden so its content
// will not be drawn to save power and a bit memory
void WindowHide(window_t *win)
{
    if (!win) return;
    win->visible = 0;
}
