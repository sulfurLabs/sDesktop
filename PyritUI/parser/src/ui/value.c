//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: value.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registry.h"
#include "value.h"
#include "cfg.h"

static pui_Define defines[PUI_DEFINES_MAX];
static int defineCount;

int pui_DefineSet(const char *name, const char *value)
{
    if (!name || !value) return 0;
    if (strlen(name) >= PUI_NAME_MAX) return 0;
    if (pui_DefineGet(name)) return 1; //re Imports r ignored
    if (defineCount >= PUI_DEFINES_MAX) return 0;

    pui_Define *d = &defines[defineCount];

    d->name = strdup(name);
    d->value = strdup(value);

    if (!d->name || !d->value)
    {
        free(d->name);
        free(d->value);
        return 0;
    }

    defineCount++;

    return 1;
}

const char *pui_DefineGet(const char *name)
{
    if (!name) return NULL;

    for (int i = 0; i < defineCount; i++)
    {
        if (strcmp(defines[i].name, name) == 0) return defines[i].value;
    }

    return NULL;
}

void pui_DefineDiag(void)
{
    printf("pui: %d defines\n", defineCount);

    for (int i = 0; i < defineCount; i++)
    {
        printf("pui: define %s = %s\n", defines[i].name, defines[i].value);
    }
}

static int QuotedCopy(const char *text, size_t len, char *out, int max)
{
    if (len < 2 || text[0] != '"' || text[len - 1] != '"') return 0;
    if ((int)len - 2 >= max) return 0;

    memcpy(out, text + 1, len - 2);
    out[len - 2] = '\0';

    return (int)len - 2;
}

int pui_ExpandArg(const char *arg, char *out, int max)
{
    if (!arg || !out || max <= 0) return 0;

    size_t len = strlen(arg);

    int quoted = QuotedCopy(arg, len, out, max);
    if (quoted) return quoted;

    for (int depth = 0; ; depth++)
    {
        if (depth >= PUI_DEPTH_MAX)
        {
            printf("pui: sefine '%s' chained to strong\n", arg);
            return 0;
        }

        const char *def = pui_DefineGet(arg);

        if (!def) break;

        len = strlen(def);

        int defQuoted = QuotedCopy(def, len, out, max);
        if (defQuoted) return defQuoted;

        arg = def;
    }

    if ((int)len >= max) return 0;

    strcpy(out, arg);

    return (int)len;
}
