//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: context.h
//

#ifndef CONTEXT_H
#define CONTEXT_H 1

#include "../cfg.h"
#include "node.h"

typedef struct pui_Property
{
    char *name; //local to property
    char *value;

    struct pui_Property *next;
} pui_Property;

typedef struct pui_Context
{
    char name[PUI_NAME_MAX];
    pui_Node *root;
    pui_Property *properties;

    struct pui_Context *next;
} pui_Context;

pui_Context *pui_ContextFirst(void);
pui_Context *pui_ContextFind(const char *name);
pui_Context *pui_ContextUse(const char *name);

void pui_ContextClear(void);

const char *pui_ContextPropertyGet(pui_Context *c, const char *name);
int pui_ContextPropertySet(pui_Context *c, const char *name, const char *value);
int pui_ContextPropertyRemove(pui_Context *c, const char *name);

int pui_SetContext(const char *args, void *user);

#endif
