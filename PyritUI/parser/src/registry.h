//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: registry.h
//

#ifndef REGISTRY_H
#define REGISTRY_H 1

#include "pui.h"
#include "cfg.h"

typedef struct
{
    char *name;

    pui_Function fn;
    pui_Get get;
    pui_Target  *target;

    void *user;
} pui_Entry;

int pui_Init(void);
pui_Entry *pui_Find(const char *name);

void pui_Context(const char *name, pui_Target *target);

int pui_RegisterFunction(const char *name, pui_Function fn, void *user);
int pui_RegisterProperty(const char *name, pui_Get get, void *user);

const char *pui_Value(const char *name);

int pui_RunFunction(const char *name, const char *args);
void pui_Unregister(const char *name);

#endif
