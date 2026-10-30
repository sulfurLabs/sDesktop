//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: draw.h
//

#ifndef DRAW_H
#define DRAW_H 1

#include "registry.h"

typedef struct
{
    pui_Target *target;
    char context[PUI_NAME_MAX];

    int x;
    int y;
    int w;
    int h;
    int anchorBottom;

    char name[PUI_NAME_MAX];
    int nodes;

    int hasColor;
    unsigned int color;

    char label[PUI_ARGS_MAX];
} pui_DrawState;

pui_DrawState *pui_State(void);
void pui_FreshDraw(void);
pui_Target *pui_DrawSetContext(const char *name);
int pui_DrawPush(const char *id, int x, int y, int w, int h, unsigned int color);

int pui_DrawColor(const char *value, unsigned int *out);
int pui_DrawPixels(const char *value, int *out);

void pui_Draw(int x, int y, int w, int h);
void pui_DrawDiag(void);

#endif
