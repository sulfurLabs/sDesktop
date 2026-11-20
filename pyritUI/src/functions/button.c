//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: button.c
//

#include <pui.h>

#include "button.h"
#include "../parser/parse.h"
#include "../parser/node.h"

int pui_Button(const char *args, void *user)
{
    (void)user;

    char buf[PUI_ARGS_MAX];
    pui_Node *n;

    if (!pui_DispatchArgs(args, buf, sizeof(buf))) return 0;

    n = pui_NodeCreate("Button");
    if (!n) return 0;

    pui_NodeUse(n);

    return 1;
}
