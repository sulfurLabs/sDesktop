//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: comp.h
//

#ifndef COMP_H
#define COMP_H 1

#include "fb.h"

typedef struct
{
    fb_backend_t fb;

    //the damage system so we dont need a full redraw
    int dirty;
    int dx;
    int dy;
    int dw;
    int dh;
} compositor_t;

int CompInit(compositor_t *comp);
void CompUnInit(compositor_t *comp);
void CompRun(compositor_t *comp);

int CompIsDirty(compositor_t *comp);
void CompDamage(
    compositor_t *comp,
    int x,
    int y,
    int w,
    int h
);
void CompDirtyRect(
    compositor_t *comp,
    int *x,
    int *y,
    int *w,
    int *h
);

void putPixel(
    compositor_t *comp,
    int x,
    int y,
    uint32_t color
);
void createRect(
    compositor_t *comp,
    int x,
    int y,
    int width,
    int height,
    uint32_t color
);


static inline int CompWidth(const compositor_t *comp)
{
    return comp->fb.width;
}

static inline int CompHeight(const compositor_t *comp)
{
    return comp->fb.height;
}

//resize + reallocate (shadow buf)
void CompSetSize(compositor_t *comp, int width, int height);

#endif
