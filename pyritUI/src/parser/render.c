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

    const char *context;
    const pui_Args *args;
    const pui_Var *values;
    int valueCount;

    //the damage region on screen
    int sx;
    int sy;
    int sw;
    int sh;

    //shared position with old data
    int x;
    int y;

    //space a child can use
    int w;
    int h;

    //pixels per row
    int stride;
} pui_Layout;

//fill means the space the parent has left
static void NodeSize(const pui_Node *n, pui_Layout *l, int *w, int *h)
{
    if (*w < 0)
    {
        //button fallback
        // this is not smart but a temporary solution
        if (strcmp(n->type, "Button") == 0) *w = PUI_BUTTON_W;
        else *w = l->w;
    }

    if (*h < 0) *h = l->h;
}

static void DrawRect(pui_Layout *l, int x, int y, int w, int h, unsigned int color)
{
    pui_Target *t = l->t;

    //target rect in screen space
    long long tx = (long long)t->x + x;
    long long ty = (long long)t->y + y;
    long long bx = tx + w;
    long long by = ty + h;

    long long ix = l->sx > tx ? l->sx : tx;
    long long iy = l->sy > ty ? l->sy : ty;
    long long endx = (long long)l->sx + l->sw < bx ? (long long)l->sx + l->sw : bx;
    long long endy = (long long)l->sy + l->sh < by ? (long long)l->sy + l->sh : by;

    //target is a border itself
    long long right = (long long)t->x + t->w;
    long long bottom = (long long)t->y + t->h;
    if (endx > right) endx = right;
    if (endy > bottom) endy = bottom;

    long long w2 = endx - ix;
    long long h2 = endy - iy;
    if (w2 <= 0 || h2 <= 0) return;

    for (long long row = 0; row < h2; row++)
    {
        uint32_t *dst =
            t->fp + (row + iy - t->y) * l->stride + (ix - t->x)
        ;

        for (long long col = 0; col < w2; col++) dst[col] = color;
    }
}

//TODO:
// there is no font here yet
// so it does nothing
static void DrawLabel(pui_Layout *l, const char *text)
{
    (void)l;
    (void)text;
}

static void RenderNodes(const pui_Node *n, pui_Layout *l)
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

        //labels r resolved while rendered
        if (n->label)
        {
            char text[PUI_ARGS_MAX];

            if (
                pui_ResolveLabel(
                    n->label,
                    l->args,
                    l->context,
                    l->values,
                    l->valueCount,
                    text,
                    sizeof(text)
                )
            ){
                DrawLabel(l, text);
            }
        }

        //kids live inside the padding
        int x = l->x;
        int y = l->y;
        int spaceW = l->w;
        int spaceH = l->h;

        l->x += n->paddingLeft;
        l->y += n->paddingTop;
        l->w = w - n->paddingLeft - n->paddingRight;
        l->h = h - n->paddingTop - n->paddingBottom;

        RenderNodes(n->kids, l);

        l->x = x;
        l->y = y;
        l->w = spaceW;
        l->h = spaceH;

        if (strcmp(n->type, "Button") == 0)
        {
            l->x += w; //horizontal stack
            l->w -= w;
        }
    }
}

void pui_RenderTree(
    const pui_Node *root,
    pui_Target *t,
    const char *context,
    const pui_Args *args,
    const pui_Var *values,
    int valueCount,
    int sx,
    int sy,
    int sw,
    int sh
){
    pui_Layout l;
    int stride;

    if (!root || !t || !t->fp) return;
    if (t->w <= 0 || t->h <= 0) return;
    if (args && !pui_ArgsValid(args)) return;

    //old api has 0
    if (t->stride < 0) return;

    stride = t->stride ? t->stride : t->w;
    if (stride < t->w) return;

    l.t = t;
    l.context = context;
    l.args = args;
    l.values = values;
    l.valueCount = valueCount;
    l.stride = stride;
    l.sx = sx;
    l.sy = sy;
    l.sw = sw;
    l.sh = sh;
    l.x = 0;
    l.y = 0;
    l.w = t->w;
    l.h = t->h;

    RenderNodes(root->kids, &l);
}
