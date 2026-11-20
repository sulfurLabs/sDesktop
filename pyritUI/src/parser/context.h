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

typedef struct pui_Context
{
    char name[PUI_NAME_MAX];
    pui_Node *root;

    struct pui_Context *next;
} pui_Context;

pui_Context *pui_ContextFirst(void);
pui_Context *pui_ContextFind(const char *name);
pui_Context *pui_ContextUse(const char *name);

void pui_ContextClear(void);

int pui_SetContext(const char *args, void *user);

#endif
