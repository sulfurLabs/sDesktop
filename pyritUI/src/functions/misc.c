//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: misc.c
//

#include <stdio.h>
#include <string.h>

#include "misc.h"
#include "../parser/parse.h"
#include "../parser/lexer.h"
#include "../parser/value.h"

//Define name value
// in the document
int pui_Define(const char *args, void *user)
{
    //TODO:
    // brackets for defines
    (void)user;

    if (!args) return 0;
    while (*args == ' ' || *args == '\t') args++;

    char name[PUI_NAME_MAX];
    const char *split = strchr(args, ' ');

    if (!split) return 0;
    if ((int)(split - args) >= PUI_NAME_MAX) return 0;

    //the name has to look like one
    for (int i = 0; i < split - args; i++)
    {
        if (!pui_IsNameChar((unsigned char)args[i]))
        {
            printf("pui: Define '%s' has a broken name\n", args);

            return 0;
        }
    }

    memcpy(name, args, (size_t)(split - args));
    name[split - args] = '\0';

    while (*split == ' ' || *split == '\t') split++;

    if (!pui_DefineSet(name, split))
    {
        printf("pui: Define '%s' couldnt store '%s'\n", name, split);

        return 0;
    }

    return 1;
}

//GetArg reads argument/value
int pui_Bind(const char *args, void *user)
{
    (void)user;

    const char *val;

    if (!args) return 0;

    val = pui_Lookup(NULL, 0, args);
    if (!val)
    {
        printf("pui: GetArg(%s) not registered/defined\n", args);

        return 0;
    }

    printf("pui: [debug] GetArg(%s) > '%s'\n", args, val);

    return 1;
}

// SetArg writes
// GetArg reads
int pui_SetArg(const char *args, void *user)
{
    (void)user;

    if (!args) return 0;

    const char *split = strchr(args, ',');
    char name[PUI_NAME_MAX];
    char expanded[PUI_ARGS_MAX];

    if (!split) return 0;
    if ((int)(split - args) >= PUI_NAME_MAX) return 0;

    memcpy(name, args, (size_t)(split - args));
    name[split - args] = '\0';

    char *end = name + strlen(name);
    while (end > name && (end[-1] == ' ' || end[-1] == '\t')) end--;
    *end = '\0';

    const char *value = split + 1;
    while (*value == ' ' || *value == '\t') value++;

    if (!pui_ExpandArg(value, expanded, sizeof(expanded))) return 0;

    return pui_DefineSet(name, expanded);
}

int pui_OnHover(const char *args, void *user)
{
    (void)user;

    char buf[PUI_ARGS_MAX];

    return pui_DispatchArgs(args, buf, sizeof(buf));
}

//for not finished functions
int pui_Quiet(const char *args, void *user)
{
    (void)args;
    (void)user;

    return 1;
}
