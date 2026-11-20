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

#include "../cfg.h"

typedef struct
{
    char *name;
    char *value;
} pui_DefineEntry;

//values of a instance the copies of its <context>.* defines
typedef struct
{
    char name[PUI_NAME_MAX];
    char value[PUI_ARGS_MAX];
} pui_Var;

void pui_DefineDiag(void);
void pui_DefineClear(void);

const char *pui_DefineGet(const char *name);
const char *pui_DefineName(int index);
const char *pui_DefineValue(int index);
const char *pui_Lookup(const pui_Var *vars, int varCount, const char *name);

int pui_DefineSet(const char *name, const char *value);
int pui_DefineCount(void);

int pui_ResolveLabel(
    const char *raw,
    const pui_Var *vars,
    int varCount,
    char *out,
    int max
);

int pui_ExpandArg(const char *arg, char *out, int max);
int pui_ValueColor(const char *value, unsigned int *out);
int pui_ValuePixels(const char *value, int *out);

#endif
