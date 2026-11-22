//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: pui.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pui.h>

#include "cfg.h"
#include "commands.h"
#include "parser/parse.h"
#include "parser/context.h"
#include "parser/node.h"
#include "parser/render.h"
#include "parser/value.h"

//the ui instance of a specific "context tree"
// the host program (sDesktop) tells it which context is which target
// like Windows, Desktop or whatever PyritUI shouldnt know this
// cuz its not only for desktops, i want it to work in apps too
// well in bigger apps, which still want customization ig
typedef struct pui_Instance
{
    int id;
    pui_Target *target;
    pui_Node *tree;     //the "clone"
    pui_Var values[PUI_INSTANCE_VARS_MAX]; //values for a specific instance
    int valueCount;

    struct pui_Instance *next;
} pui_Instance;

static pui_Instance *instancesFirst;
static pui_Instance *instancesLast;
static int instanceCount;
static int nextInstanceId;

static void SeedValues(pui_Instance *w, pui_Context *c)
{
    for (
        pui_Property *p = c->properties;
        p && w->valueCount < PUI_INSTANCE_VARS_MAX;
        p = p->next
    ) {
        if (!p->name || !p->value) continue;
        if (strlen(c->name) + 1 + strlen(p->name) >= PUI_NAME_MAX) continue;

        snprintf(
            w->values[w->valueCount].name,
            PUI_NAME_MAX,
            "%s.%s",
            c->name,
            p->name
        );
        snprintf(
            w->values[w->valueCount].value,
            PUI_ARGS_MAX,
            "%s",
            p->value
        );

        w->valueCount++;
    }
}

//TODO:
// puilib should not show Contexts/Instances,
// just a way to create Targets and how they should be called in
// pyritUI SetContext
int pui_InstanceNew(const char *name, pui_Target *target)
{
    pui_Instance *w;
    pui_Context *c;

    if (!target) return 0;

    c = pui_ContextFind(name);
    if (!c || !c->root->kids)
    {
        printf("pui: no '%s' context loaded\n", name ? name : "");

        return 0;
    }

    w = malloc(sizeof(pui_Instance));
    if (!w)
    {
        printf("pui: no memory for a ui instance\n");

        return 0;
    }

    memset(w, 0, sizeof(pui_Instance));

    w->target = target;
    w->tree = pui_NodeClone(c->root);
    if (!w->tree)
    {
        free(w);

        return 0;
    }

    SeedValues(w, c);

    if (instancesLast) instancesLast->next = w;
    else instancesFirst = w;

    instancesLast = w;
    instanceCount++;

    //ids r never reused
    w->id = ++nextInstanceId;

    return w->id;
}

void pui_InstanceDrop(int id)
{
    pui_Instance **link = &instancesFirst;

    while (*link)
    {
        pui_Instance *w = *link;

        if (w->id != id)
        {
            link = &w->next;

            continue;
        }

        pui_NodeFree(w->tree);

        *link = w->next;
        if (instancesLast == w)
        {
            //found again after the unlink
            instancesLast = NULL;
            for (pui_Instance *x = instancesFirst; x; x = x->next) instancesLast = x;
        }

        free(w);
        instanceCount--;

        return;
    }
}

int pui_InstanceValue(int id, const char *name, const char *value)
{
    pui_Instance *w;
    pui_Var *v;

    for (w = instancesFirst; w; w = w->next)
    {
        if (w->id == id)
        {
            if (!name || strlen(name) >= PUI_NAME_MAX) return 0;

            //a set value overwrites what the doc made
            for (int i = 0; i < w->valueCount; i++)
            {
                if (strcmp(w->values[i].name, name) == 0)
                {
                    snprintf(w->values[i].value, PUI_ARGS_MAX, "%s", value ? value : "");

                    return 1;
                }
            }

            if (w->valueCount >= PUI_INSTANCE_VARS_MAX) return 0;
            if (!value) return 1;

            v = &w->values[w->valueCount++];
            snprintf(v->name, PUI_NAME_MAX, "%s", name);
            snprintf(v->value, PUI_ARGS_MAX, "%s", value);

            return 1;
        }
    }

    return 0;
}

int pui_SetProperty(
    const char *context,
    const char *name,
    const char *value
){
    pui_Context *c = pui_ContextFind(context);

    if (!c) return 0;

    return pui_ContextPropertySet(c, name, value);
}

int pui_GetProperty(
    const char *context,
    const char *name,
    char *value,
    size_t value_size
){
    pui_Context *c;
    const char *property;
    size_t len;

    if (!value || !value_size) return 0;

    c = pui_ContextFind(context);
    if (!c) return 0;

    property = pui_ContextPropertyGet(c, name);
    if (!property) return 0;

    len = strlen(property);
    if (len >= value_size) return 0;

    memcpy(value, property, len + 1);

    return 1;
}

int pui_RemoveProperty(const char *context, const char *name)
{
    pui_Context *c = pui_ContextFind(context);

    if (!c) return 0;

    return pui_ContextPropertyRemove(c, name);
}

void pui_Draw(int x, int y, int w, int h)
{
    for (pui_Context *c = pui_ContextFirst(); c; c = c->next)
    {
        pui_Entry *e = pui_Find(c->name);

        if (!e || !e->target) continue;

        pui_RenderTree(c->root, e->target, x, y, w, h);
    }

    for (pui_Instance *win = instancesFirst; win; win = win->next)
    {
        pui_RenderTree(win->tree, win->target, x, y, w, h);
    }
}

static void DumpNodes(
    const pui_Node *n,
    const pui_Var *values,
    int valueCount,
    int depth
){
    for (; n; n = n->next)
    {
        printf("pui: ");

        for (int i = 0; i < depth; i++) printf(" ");

        printf("- %s '%s'", n->type, n->id);

        if (n->w < 0 || n->h < 0)
        {
            printf(" fill");
        }
        else
        {
            printf(" %dx%d", n->w, n->h);
        }

        if (n->hasColor)
        {
            printf(" bg %06x", n->color & 0xffffff);
        }

        if (n->label)
        {
            char text[PUI_ARGS_MAX];
            if (pui_ResolveLabel(n->label, values, valueCount, text, sizeof(text)))
            {
                printf(" label '%s'", text);
            }
        }

        printf("\n");

        DumpNodes(n->kids, values, valueCount, depth + 2);
    }
}

void pui_DrawDiag(void)
{
    printf("pui: %d nodes, %d instances\n", pui_NodeCount(), instanceCount);
    pui_DefineDiag();

    for (pui_Context *c = pui_ContextFirst(); c; c = c->next)
    {
        printf("pui: context '%s'\n", c->name);

        for (pui_Property *p = c->properties; p; p = p->next)
        {
            printf("pui:   property %s.%s = %s\n", c->name, p->name, p->value);
        }

        DumpNodes(c->root, NULL, 0, 2);
    }
    for (pui_Instance *w = instancesFirst; w; w = w->next)
    {
        printf(
            "pui: instance %d %dx%d at %d,%d\n",
            w->id,
            w->target->w,
            w->target->h,
            w->target->x,
            w->target->y
        );
        DumpNodes(w->tree, w->values, w->valueCount, 2);
    }
}

void pui_Shutdown(void)
{
    while (instancesFirst) pui_InstanceDrop(instancesFirst->id);

    pui_ContextClear();
    pui_DefineClear();

    instancesLast = NULL;
    nextInstanceId = 0;
}

const struct pyrit pyrit =
{
    .Init = pui_Init,
    .Context = pui_ContextTarget,
    .RegisterFunction = pui_RegisterFunction,
    .RegisterProperty = pui_RegisterProperty,
    .Value = pui_Value,
    .RunFunction = pui_RunFunction,
    .ExpandArg = pui_ExpandArg,
    .Unregister = pui_Unregister,
    .SetProperty = pui_SetProperty,
    .GetProperty = pui_GetProperty,
    .RemoveProperty = pui_RemoveProperty,
    .SetContext = pui_SetContext,
    .LoadFile = pui_LoadFile,
    .Draw = pui_Draw,
    .DrawDiag = pui_DrawDiag,
    .Instance = pui_InstanceNew,
    .InstanceDrop = pui_InstanceDrop,
    .InstanceValue = pui_InstanceValue,
    .Shutdown = pui_Shutdown
};
