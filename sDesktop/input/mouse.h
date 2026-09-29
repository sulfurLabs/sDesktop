//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: mouse.h
//

#ifndef MOUSE_H
#define MOUSE_H 1

#include "../display/comp.h"

typedef struct
{
    compositor_t *compositor;
} mouse;

typedef struct
{
    int x;
    int y;
} cursor_t;

int OpenMouse();
void CloseMouse();
//if it errors -1 it means theres no mouse device/ it dissapeared
int ReadMouse(int *MoveX, int *MoveY);
void CursorMove(cursor_t *c, int MoveX, int MoveY, int Width, int Height);
void DrawMouse(compositor_t *comp, const cursor_t *c);

#endif
