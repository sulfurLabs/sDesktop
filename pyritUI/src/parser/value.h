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

#include <pui.h>

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

//one raw element of Array(...)
// the pointers stay inside the parsed string
typedef struct
{
    const char *start;
    int len;
} pui_ArrayItem;

typedef struct
{
    int count;
    pui_ArrayItem items[PUI_ARRAY_MAX];
} pui_Array;

void pui_DefineDiag(void);
void pui_DefineClear(void);

const char *pui_DefineGet(const char *name);
const char *pui_DefineName(int index);
const char *pui_DefineValue(int index);

int pui_LookupName(const char *start, const char *end, char *name, int max);
int pui_ArgsValid(const pui_Args *args);
const char *pui_Lookup(
    const pui_Args *args,
    const char *context,
    const pui_Var *vars,
    int varCount,
    const char *name
);

int pui_DefineSet(const char *name, const char *value);
int pui_DefineCount(void);

int pui_ResolveLabel(
    const char *raw,
    const pui_Args *args,
    const char *context,
    const pui_Var *vars,
    int varCount,
    char *out,
    int max
);

int pui_ExpandArg(const char *arg, char *out, int max);
int pui_ValueColor(const char *value, unsigned int *out);
int pui_ValuePixels(const char *value, int *out);
int pui_ValueIsArray(const char *value);
int pui_ValueArray(const char *value, pui_Array *out);

#endif
