//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: draw.c
//

#include <stdio.h>
#include <string.h>

#include "value.h"
#include "cfg.h"
#include "draw.h"

typedef struct
{
    char context[PUI_NAME_MAX];
    char id[PUI_NAME_MAX];
    int x, y, w, h;
    unsigned int color;
} Rect;

static Rect rects[PUI_RECTS_MAX];
static int rectCount;
static pui_DrawState state;

pui_DrawState *pui_State(void)
{
    return &state;
}

void pui_FreshDraw(void)
{
    rectCount = 0;
}


pui_Target *pui_DrawSetContext(const char *name)
{
    if (!name) return NULL;

    pui_Entry *e = pui_Find(name);
    if (!e)
    {
        printf("pui: context '%s' isnt registered\n", name);

        state.context[0] = '\0';
        state.target = NULL;

        return NULL;
    }

    state.target = e->target;
    snprintf(state.context, sizeof(state.context), "%s", name);

    state.x = 0;
    state.y = 0;
    state.w = -1;
    state.h = -1;
    state.anchorBottom = 0;
    state.name[0] = '\0';
    state.nodes = 0;
    state.hasColor = 0;
    state.color = 0;
    state.label[0] = '\0';

    return state.target;
}

int pui_DrawPush(const char *id, int x, int y, int w, int h, unsigned int color)
{
    if (!state.target || rectCount >= PUI_RECTS_MAX) return 0;

    Rect *r = &rects[rectCount];

    snprintf(r->context, sizeof(r->context), "%s", state.context);
    snprintf(r->id, sizeof(r->id), "%s", id ? id : "");

    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
    r->color = color;

    rectCount++;

    return 1;
}

//sulfur doesnt have strtoul yet
//TODO:
// sulflibc needs strotul
// remove this then
static unsigned int ToUInt(const char *text)
{
    unsigned int out = 0;

    for (; *text >= '0' && *text <= '9'; text++)
    {
        out = out * 10 + (unsigned int)(*text - '0');
    }

    return out;
}

int pui_DrawColor(const char *value, unsigned int *out)
{
    if (!value || !out) return 0;

    unsigned int slots[4];
    int want = 0;
    int count = 0;

    const char *p = value;

    //TODO
    // bracket parser
    if (!strncmp(p, "argb(", 5))
    {
        p += 5;
        want = 4;
    }
    else if (!strncmp(p, "rgb(", 4))
    {
        p += 4;
        want = 3;
    }

    for (;;)
    {
        if (*p >= '0' && *p <= '9')
        {
            if (count < 4) slots[count] = ToUInt(p);
            count++;
            while (*p >= '0' && *p <= '9') p++;
        }

        if (*p == ',')
        {
            p++;
            while (*p == ' ') p++;
            continue;
        }

        break;
    }

    if (count == want && count)
    {
        unsigned int a = 255, r = slots[0], g = slots[1], b = slots[2];

        if (want == 4)
        {
            a = slots[0];
            r = slots[1];
            g = slots[2];
            b = slots[3];
        }

        *out = a << 24 | r << 16 | g << 8 | b;

        return 1;
    }

    if (!want && count == 1)
    {
        *out = 255u << 24 | slots[0] << 16 | slots[0] << 8 | slots[0];

        return 1;
    }

    return 0;
}

int pui_DrawPixels(const char *value, int *out)
{
    if (!value || !out) return 0;

    if (!strcmp(value, "fill"))
    {
        *out = -1;
        return 1;
    }

    if (!strncmp(value, "px(", 3)) value += 3;
    //TODO:
    // yea... px( isnt a great solution, we need a function which automatically detects brackets etc

    //plain digits have the px suffix
    int end = 0;

    while (value[end] >= '0' && value[end] <= '9') end++;
    if (!end) return 0;

    *out = (int)ToUInt(value);

    return 1;
}


void pui_DrawDiag(void)
{
    printf("pui: %d rects pushed\n", rectCount);
    pui_DefineDiag();

    if (state.label[0])
    {
        printf("pui: last label in context '%s': '%s'\n", state.context, state.label);
    }

    for (int i = 0; i < rectCount; i++)
    {
        printf(
            "pui: rect %d '%s'  id '%s' at %d,%d size %dx%d color %06x\n",
            i,
            rects[i].context,
            rects[i].id,
            rects[i].x,
            rects[i].y,
            rects[i].w,
            rects[i].h,
            rects[i].color & 0xffffff
        );
    }
}

void pui_Draw(int sx, int sy, int sw, int sh)
{
    for (int i = 0; i < rectCount; i++)
    {
        pui_Entry *e = pui_Find(rects[i].context);
        if (!e || !e->target) continue;

        pui_Target *t = e->target;

        //rect of the target in screen space
        int tx = t->x + rects[i].x;
        int ty = t->y + rects[i].y;

        int ix = sx > tx ? sx : tx;
        int iy = sy > ty ? sy : ty;
        int endx = sx + sw < tx + rects[i].w ? sx + sw : tx + rects[i].w;
        int endy = sy + sh < ty + rects[i].h ? sy + sh : ty + rects[i].h;

        int w = endx - ix;
        int h = endy - iy;
        if (w <= 0 || h <= 0) continue;

        for (int row = 0; row < h; row++)
        {
            uint32_t *dst = t->fp + (row + iy - t->y) * t->w + (ix - t->x);
            for (int col = 0; col < w; col++) dst[col] = rects[i].color;
        }
    }
}
