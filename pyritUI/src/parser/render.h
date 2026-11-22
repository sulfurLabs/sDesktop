//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: render.h
//

#ifndef RENDER_H
#define RENDER_H 1

#include <pui.h>

#include "node.h"
#include "value.h"

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
);

#endif
