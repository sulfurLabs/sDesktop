//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: pui.h
//

#pragma once

#include <stdint.h>


typedef int (*pui_Function)(const char *args, void *user);
typedef const char *(*pui_Get)(void *user);

typedef struct
{
    uint32_t *fp;
    int x, y;
    int w, h;
} pui_Target;

struct pyrit
{
    int (*Init)(void);

    void (*Context)(const char *name, pui_Target *target);

    void (*Unregister)(const char *name);
    int (*RegisterFunction)(const char *name, pui_Function fn, void *user);
    int (*RegisterProperty)(const char *name, pui_Get get, void *user);
    int (*RunFunction)(const char *name, const char *args);
    int (*ExpandArg)(const char *arg, char *out, int max);

    const char *(*Value)(const char *name);

    //default handler
    int (*SetContext)(const char *args, void *user);
    int (*LoadFile)(const char *path);
    void (*Draw)(int x, int y, int w, int h);

    void (*DrawDiag)(void);
};

//pyrit. prefix
extern const struct pyrit pyrit;
