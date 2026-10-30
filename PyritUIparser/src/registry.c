//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: registry.c
//

#include <pui.h>
#include "ui/builtin.h"
#include "ui/draw.h"
#include "ui/parse.h"
#include "ui/value.h"
#include "registry.h"
#include <stdlib.h>
#include <string.h>

static pui_Entry functions[PUI_FUNCTIONS_MAX];
static int functionCount;

int pui_Init(void)
{
    for (int i = 0; i < functionCount; i++) free(functions[i].name);

    functionCount = 0;
    memset(functions, 0, sizeof(functions));

    pui_RegisterBuiltins();

    return 1;
}

pui_Entry *pui_Find(const char *name)
{
    if (!name) return NULL;

    for (int i = 0; i < functionCount; i++)
    {
        if (strcmp(functions[i].name, name) == 0) return &functions[i];
    }

    return NULL;
}

static pui_Entry *pui_Fill(
    const char *name,
    pui_Function fn,
    pui_Get get,
    pui_Target *target,
    void *user
){
    if (!name || strlen(name) >= PUI_NAME_MAX) return NULL;

    pui_Entry *e = pui_Find(name);
    if (!e)
    {
        if (functionCount >= PUI_FUNCTIONS_MAX) return NULL;

        e = &functions[functionCount];
        e->name = strdup(name);

        if (!e->name) return NULL;

        functionCount++;
    }

    if (fn) e->fn = fn;
    if (get) e->get = get;
    if (target) e->target = target;

    e->user = user;

    return e;
}

int pui_RegisterFunction(const char *name, pui_Function fn, void *user)
{
    if (!fn) return 0;

    return pui_Fill(name, fn, NULL, NULL, user) ? 1 : 0;
}
int pui_RegisterProperty(const char *name, pui_Get get, void *user)
{
    if (!get) return 0;

    return pui_Fill(name, NULL, get, NULL, user) ? 1 : 0;
}

void pui_Context(const char *name, pui_Target *target)
{
    pui_Fill(name, NULL, NULL, target, NULL);
}

const char *pui_Value(const char *name)
{
    pui_Entry *e = pui_Find(name);
    if (!e || !e->get) return NULL;

    return e->get(e->user);
}

int pui_RunFunction(const char *name, const char *args)
{
    pui_Entry *e = pui_Find(name);
    if (!e || !e->fn) return 0;

    return e->fn(args, e->user);
}

void pui_Unregister(const char *name)
{
    pui_Entry *e = pui_Find(name);
    if (!e) return;

    int index = (int)(e - functions);

    free(e->name);

    functionCount--;

    memmove(
        &functions[index],
        &functions[index + 1],
        sizeof(pui_Entry) * (size_t)(functionCount - index)
    );
    memset(
        &functions[functionCount],
        0,
        sizeof(pui_Entry)
    );
}

const struct pyrit pyrit =
{
    .Init = pui_Init,
    .Context = pui_Context,
    .RegisterFunction = pui_RegisterFunction,
    .RegisterProperty = pui_RegisterProperty,
    .Value = pui_Value,
    .RunFunction = pui_RunFunction,
    .ExpandArg = pui_ExpandArg,
    .Unregister = pui_Unregister,
    .SetContext = pui_SetContext,
    .LoadFile = pui_LoadFile,
    .Draw = pui_Draw,
    .DrawDiag = pui_DrawDiag
};
