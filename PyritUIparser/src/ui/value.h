//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: value.h
//

#ifndef VALUE_H
#define VALUE_H 1

typedef struct
{
    char *name;
    char *value;
} pui_Define;

int pui_DefineSet(const char *name, const char *value);
const char *pui_DefineGet(const char *name);
void pui_DefineDiag(void);

int pui_ExpandArg(const char *arg, char *out, int max);

#endif
