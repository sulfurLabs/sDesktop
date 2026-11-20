//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: render.c
//

#include <string.h>

#include <pui.h>

#include "render.h"
#include "value.h"

typedef struct
{
    pui_Target *t;
    //the damage region on screen
    int sx;
    int sy;
    int sw;
    int sh;

    //shared position with old data
    int x;
    int y;
} pui_Layout;

//fill means the target size minus what got used already
static void NodeSize(const pui_Node *n, pui_Layout *l, int *w, int *h)
{
    if (*w < 0)
    {
        //button fallback
        // this is not smart but a temporary solution
        if (strcmp(n->type, "Button") == 0) *w = PUI_BUTTON_W;
        else *w = l->t->w - l->x;
    }

    if (*h < 0) *h = l->t->h - l->y;
}

static void DrawRect(pui_Layout *l, int x, int y, int w, int h, unsigned int color)
{
    pui_Target *t = l->t;

    //target rect in screen space
    int tx = t->x + x;
    int ty = t->y + y;

    int ix = l->sx > tx ? l->sx : tx;
    int iy = l->sy > ty ? l->sy : ty;
    int endx = l->sx + l->sw < tx + w ? l->sx + l->sw : tx + w;
    int endy = l->sy + l->sh < ty + h ? l->sy + l->sh : ty + h;

    //target is a border itself
    if (endx > t->x + t->w) endx = t->x + t->w;
    if (endy > t->y + t->h) endy = t->y + t->h;

    int w2 = endx - ix;
    int h2 = endy - iy;
    if (w2 <= 0 || h2 <= 0) return;

    for (int row = 0; row < h2; row++)
    {
        uint32_t *dst =
            t->fp + (row + iy - t->y)  *
            t->w + (ix - t->x)
        ;

        for (int col = 0; col < w2; col++) dst[col] = color;
    }
}

static void RenderNodes(pui_Node *n, pui_Layout *l)
{
    for (; n; n = n->next)
    {
        int w = n->w;
        int h = n->h;

        NodeSize(n, l, &w, &h);

        //anchor is to set a fix position
        // like we need for taskbar
        if (n->anchorBottom) l->y = l->t->h - h;
        if (n->hasColor) DrawRect(l, l->x, l->y, w, h, n->color);
        if (strcmp(n->type, "Button") == 0) l->x += w; //horizontal stack

        RenderNodes(n->kids, l);
    }
}

void pui_RenderTree(
    const pui_Node *root,
    pui_Target *t,
    int sx,
    int sy,
    int sw,
    int sh
){
    pui_Layout l;

    if (!root || !t || !t->fp) return;

    l.t = t;
    l.sx = sx;
    l.sy = sy;
    l.sw = sw;
    l.sh = sh;
    l.x = 0;
    l.y = 0;

    RenderNodes(root->kids, &l);
}
