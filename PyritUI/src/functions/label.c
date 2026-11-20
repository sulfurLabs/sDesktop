//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: label.c
//

#include <stdlib.h>
#include <string.h>

#include <pui.h>

#include "label.h"
#include "../parser/parse.h"
#include "../parser/node.h"

int pui_Label(const char *args, void *user)
{
    (void)user;

    pui_Node *n;

    n = pui_NodeCreate("Label");
    if (!n) return 0;

    pui_NodeUse(n);

    if (args)
    {
        while (*args == ' ' || *args == '\t') args++;
        if (!args[0]) return 1;

        n->label = strdup(args);

        if (!n->label) return 1; //label lost but the node stays
    }

    return 1;
}
