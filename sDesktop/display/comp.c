//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: comp.c
//

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../os.h"
#include "../log.h"
#include "comp.h"

int CompInit(compositor_t *comp)
{
    if (!comp) return 0;
    memset(comp, 0, sizeof(*comp));

    if (fbInit(&comp->fb) == 0) return 0;

    //full draw cuz its the first draw
    comp->dirty = 1;
    comp->dx = 0;
    comp->dy = 0;
    comp->dw = comp->fb.width;
    comp->dh = comp->fb.height;

    return 1;
}


void CompUnInit(compositor_t *comp)
{
    if (!comp) return;

    fbUnInit(&comp->fb);
}


void CompSetSize(compositor_t *comp, int width, int height)
{
    unsigned int *back;

    //for the shadow buffer which needs the new size
    back = malloc((size_t)width * height * 4);
    if (!back) return;

    free(comp->fb.back);
    comp->fb.back = back;

    memset(back, 0, (size_t)width * height * 4);

    comp->fb.width = width;
    comp->fb.height = height;

    //everything changed this would be a full redraw
    comp->dirty = 1;
    comp->dx = 0;
    comp->dy = 0;
    comp->dw = width;
    comp->dh = height;
}


void CompRun(compositor_t *comp)
{
    if (!comp) return;

    //we present and then clean
    if (comp->dirty) fbFlush(
        &comp->fb,
        comp->dx,
        comp->dy,
        comp->dw,
        comp->dh
    );

    fbPoll(&comp->fb);

    //sets the damage to 0
    comp->dirty = 0;
    comp->dx = 0;
    comp->dy = 0;
    comp->dw = 0;
    comp->dh = 0;
}


void CompDamage(
    compositor_t *comp,
    int x,
    int y,
    int w,
    int h
){
    int ex;
    int ey;
    int nx;
    int ny;

    if (w <= 0 || h <= 0) return;

    //to screen
    if (x < 0) {  w += x;  x = 0;  }
    if (y < 0) {  h += y;  y = 0;  }
    if (x + w > comp->fb.width) w = comp->fb.width - x;
    if (y + h > comp->fb.height) h = comp->fb.height - y;
    if (w <= 0 || h <= 0) return;

    if (!comp->dirty)
    {
        comp->dirty = 1;
        comp->dx = x;
        comp->dy = y;
        comp->dw = w;
        comp->dh = h;

        return;
    }

    //expands the box
    nx = comp->dx < x ? comp->dx : x;
    ny = comp->dy < y ? comp->dy : y;
    ex = comp->dx + comp->dw > x + w ? comp->dx + comp->dw : x + w;
    ey = comp->dy + comp->dh > y + h ? comp->dy + comp->dh : y + h;

    comp->dx = nx;
    comp->dy = ny;
    comp->dw = ex - nx;
    comp->dh = ey - ny;
}


int CompIsDirty(compositor_t *comp)
{
    return comp->dirty;
}


void CompDirtyRect(
    compositor_t *comp,
    int *x,
    int *y,
    int *w,
    int *h
){
    *x = comp->dx;
    *y = comp->dy;
    *w = comp->dw;
    *h = comp->dh;
}


void putPixel(
    compositor_t *comp,
    int x,
    int y,
    uint32_t color
){
    if (!comp) return;

    fbPutPixel(&comp->fb, x, y, color);
    CompDamage(comp, x, y, 1, 1);
}


void createRect(
    compositor_t *comp,
    int x,
    int y,
    int width,
    int height,
    uint32_t color
){
    if (!comp) return;
    if (width <= 0 || height <= 0) return;
    if (x < 0) {  width += x;  x = 0;  }
    if (y < 0) {  height += y;  y = 0;  }
    if (x + width > comp->fb.width) width = comp->fb.width - x;
    if (y + height > comp->fb.height) height = comp->fb.height - y;
    if (width <= 0 || height <= 0) return;

    //if we write row by row instead of pixel per pixel its faster
    for (int py = 0; py < height; py++)
    {
        unsigned int *row = comp->fb.back + (size_t)(y + py) * comp->fb.width + x;
        for (int px = 0; px < width; px++) row[px] = color;
    }

    CompDamage(comp, x, y, width, height);
}
