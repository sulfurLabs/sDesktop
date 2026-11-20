//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: context.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "value.h"
#include "parse.h"

static pui_Context *contextsFirst;
static pui_Context *contextsLast;

pui_Context *pui_ContextFirst(void)
{
    return contextsFirst;
}

pui_Context *pui_ContextFind(const char *name)
{
    if (!name) return NULL;

    for (pui_Context *c = contextsFirst; c; c = c->next)
    {
        if (strcmp(c->name, name) == 0) return c;
    }

    return NULL;
}

pui_Context *pui_ContextUse(const char *name)
{
    pui_Context *c = pui_ContextFind(name);

    if (c) return c;

    if (!name || !name[0] || strlen(name) >= PUI_NAME_MAX) return NULL;

    c = malloc(sizeof(pui_Context));
    if (!c)
    {
        printf("pui: no memory for context '%s'\n", name);

        return NULL;
    }

    memset(c, 0, sizeof(pui_Context));
    snprintf(c->name, sizeof(c->name), "%s", name);

    //the root is just a holder and never rendered
    c->root = pui_NodeRoot();
    if (!c->root)
    {
        printf("pui: no memory for context '%s'\n", name);
        free(c);

        return NULL;
    }

    if (contextsLast) contextsLast->next = c;
    else contextsFirst = c;

    contextsLast = c;

    return c;
}

void pui_ContextClear(void)
{
    pui_Context *c = contextsFirst;

    while (c)
    {
        pui_Context *next = c->next;

        pui_NodeFree(c->root);
        free(c);

        c = next;
    }

    contextsFirst = NULL;
    contextsLast = NULL;
}

//the context is the section to where to draw
int pui_SetContext(const char *args, void *user)
{
    (void)user;

    pui_ParseState *s = pui_State();
    pui_Context *c;
    char name[PUI_NAME_MAX];

    if (!args) return 0;
    if (!pui_ExpandArg(args, name, sizeof(name))) return 0;

    c = pui_ContextUse(name);
    if (!c) return 0;

    s->context = c;
    s->parent = NULL;
    s->lastNode = c->root;
    s->nodes = 0; //auto ids start fresh per context

    return 1;
}
