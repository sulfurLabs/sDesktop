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
#include "lexer.h"

static pui_Context *contextsFirst;
static pui_Context *contextsLast;

//property names r local to their context
static int PropertyName(const char *name)
{
    size_t len;

    if (!name || !name[0]) return 0;

    len = strlen(name);
    if (len >= PUI_NAME_MAX) return 0;

    for (size_t i = 0; i < len; i++)
    {
        if (!pui_IsNameChar((unsigned char)name[i])) return 0;

        if (
            name[i] == '.' &&
            (i == 0 || i + 1 == len || name[i - 1] == '.')
        ) return 0;
    }

    return 1;
}

static pui_Property *FindProperty(pui_Context *c, const char *name)
{
    if (!c || !name) return NULL;

    for (pui_Property *p = c->properties; p; p = p->next)
    {
        if (strcmp(p->name, name) == 0) return p;
    }

    return NULL;
}

static void ClearProperties(pui_Context *c)
{
    pui_Property *p = c->properties;

    while (p)
    {
        pui_Property *next = p->next;

        free(p->name);
        free(p->value);
        free(p);

        p = next;
    }

    c->properties = NULL;
}

const char *pui_ContextPropertyGet(pui_Context *c, const char *name)
{
    pui_Property *p = FindProperty(c, name);

    if (!p) return NULL;

    return p->value;
}

int pui_ContextPropertySet(pui_Context *c, const char *name, const char *value)
{
    if (!c || !value || !PropertyName(name)) return 0;

    pui_Property *p = FindProperty(c, name);

    //setting an existing property only replaces its value
    if (p)
    {
        char *copy = strdup(value);

        if (!copy)
        {
            printf("pui: no memory for property '%s'\n", name);

            return 0;
        }

        free(p->value);
        p->value = copy;

        return 1;
    }

    p = malloc(sizeof(pui_Property));
    if (!p)
    {
        printf("pui: no memory for property '%s'\n", name);

        return 0;
    }

    memset(p, 0, sizeof(pui_Property));

    p->name = strdup(name);
    p->value = strdup(value);

    if (!p->name || !p->value)
    {
        printf("pui: no memory for property '%s'\n", name);

        free(p->name);
        free(p->value);
        free(p);

        return 0;
    }

    pui_Property **link = &c->properties;

    while (*link) link = &(*link)->next;

    *link = p;

    return 1;
}

int pui_ContextPropertyRemove(pui_Context *c, const char *name)
{
    if (!c || !PropertyName(name)) return 0;

    pui_Property **link = &c->properties;

    while (*link)
    {
        pui_Property *p = *link;

        if (strcmp(p->name, name) == 0)
        {
            *link = p->next;

            free(p->name);
            free(p->value);
            free(p);

            return 1;
        }

        link = &p->next;
    }

    return 0;
}

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

        ClearProperties(c);
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
