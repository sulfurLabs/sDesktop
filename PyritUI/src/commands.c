//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: commands.c
//

#include <stdlib.h>
#include <string.h>

#include "commands.h"

//the command implementations live in functions/
#include "parser/context.h"
#include "functions/container.h"
#include "functions/button.h"
#include "functions/label.h"
#include "functions/style.h"
#include "functions/misc.h"

typedef struct
{
    const char *name;
    pui_Function fn;
} pui_Cmd;

//a list of all commands, like a documentation
static const pui_Cmd commands[] =
{
    { "Container", pui_Container },
    { "Button",    pui_Button },
    { "Label",     pui_Label },
    { "Style",     pui_Style },

    //style properties
    { "Name",     pui_Name },
    { "ID",       pui_Name }, //for styles
    { "width",    pui_Width },
    { "height",   pui_Height },
    { "bg",       pui_Bg },
    { "anchor",   pui_Anchor },
    { "layout",   pui_Quiet },
    { "icon",     pui_Quiet },

    //values
    { "Define", pui_Define },
    { "Bind",   pui_Bind },
    { "GetArg", pui_Bind },
    { "SetArg", pui_SetArg },

    { "SetContext", pui_SetContext },
    { "OnHover",    pui_OnHover },

    //not finished yet
    { "LoadImage",  pui_Quiet },
    { "Image",      pui_Quiet },
    { "Execute",    pui_Quiet },
    { "tooltip",    pui_Quiet },
};

static pui_Entry functions[PUI_FUNCTIONS_MAX];
static int functionCount;

int pui_Init(void)
{
    for (int i = 0; i < functionCount; i++) free(functions[i].name);

    functionCount = 0;
    memset(functions, 0, sizeof(functions));

    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
    {
        pui_RegisterFunction(commands[i].name, commands[i].fn, NULL);
    }

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

void pui_ContextTarget(const char *name, pui_Target *target)
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
