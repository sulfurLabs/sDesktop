//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: container.c
//

#include <pui.h>

#include "container.h"
#include "../parser/parse.h"
#include "../parser/node.h"

int pui_Container(const char *args, void *user)
{
    (void)user;

    char buf[PUI_ARGS_MAX];
    pui_Node *n;

    //the args run first so the pending props r ready
    if (!pui_DispatchArgs(args, buf, sizeof(buf))) return 0;

    n = pui_NodeCreate("Container");
    if (!n) return 0;

    pui_NodeUse(n);

    return 1;
}
