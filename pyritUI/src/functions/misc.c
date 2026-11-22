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
// Define(name, value) also works
int pui_Define(const char *args, void *user)
{
    (void)user;

    if (!args) return 0;

    char name[PUI_NAME_MAX];
    char value[PUI_ARGS_MAX];

    const char *start = args;
    const char *limit = args + strlen(args);
    const char *tail = limit;
    const char *split;
    const char *vstart;
    const char *vend;

    while (start < limit && (*start == ' ' || *start == '\t')) start++;
    while (tail > start && pui_IsSpace((unsigned char)tail[-1])) tail--;

    if (tail - start >= 2 && start[0] == '(' && tail[-1] == ')')
    {
        start++;
        tail--;

        while (start < tail && (*start == ' ' || *start == '\t')) start++;
        while (tail > start && pui_IsSpace((unsigned char)tail[-1])) tail--;

        limit = tail;
    }

    split = start;

    while (
        split < limit &&
        *split != ' ' &&
        *split != '\t' &&
        *split != ','
    ) split++;

    if (split == limit) return 0;
    if ((int)(split - start) >= PUI_NAME_MAX) return 0;

    //the name has to look like one
    for (const char *c = start; c < split; c++)
    {
        if (!pui_IsNameChar((unsigned char)*c))
        {
            printf("pui: Define '%s' has a broken name\n", args);

            return 0;
        }
    }

    memcpy(name, start, (size_t)(split - start));
    name[split - start] = '\0';

    vstart = split;

    while (
        vstart < limit &&
        (*vstart == ' ' || *vstart == '\t' || *vstart == ',')
    ) vstart++;

    vend = limit;

    while (vend > vstart && pui_IsSpace((unsigned char)vend[-1])) vend--;

    int vlen = (int)(vend - vstart);

    if (vlen >= (int)sizeof(value)) return 0;

    memcpy(value, vstart, (size_t)vlen);
    value[vlen] = '\0';

    if (!pui_DefineSet(name, value))
    {
        printf("pui: Define '%s' couldnt store '%s'\n", name, value);

        return 0;
    }

    return 1;
}

//the current context owns the property of GetArg(name)
static const char *ContextValue(const char *name)
{
    pui_Context *c = pui_State()->context;
    size_t len;

    if (!c || !name) return NULL;

    len = strlen(c->name);

    if (strncmp(name, c->name, len) != 0) return NULL;
    if (name[len] != '.') return NULL;

    return pui_ContextPropertyGet(c, name + len + 1);
}

//GetArg reads argument/value
int pui_Bind(const char *args, void *user)
{
    (void)user;

    const char *val;

    if (!args) return 0;

    val = ContextValue(args);
    if (!val) val = pui_Lookup(NULL, 0, args);

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

    pui_Context *c = pui_State()->context;
    const char *split = strchr(args, ',');
    const char *start;
    const char *end;
    const char *local;
    char name[PUI_NAME_MAX];
    char expanded[PUI_ARGS_MAX];
    size_t len;
    size_t contextLen;

    if (!c || !split) return 0;

    start = args;
    end = split;
    while (*start == ' ' || *start == '\t') start++;
    while (end > start && pui_IsSpace((unsigned char)end[-1])) end--;

    len = (size_t)(end - start);
    if (!len || len >= sizeof(name)) return 0;

    memcpy(name, start, len);
    name[len] = '\0';

    //the property needs to belong to the current context
    contextLen = strlen(c->name);
    if (len <= contextLen) return 0;
    if (strncmp(name, c->name, contextLen) != 0) return 0;
    if (name[contextLen] != '.') return 0;

    local = name + contextLen + 1;

    const char *value = split + 1;
    while (*value == ' ' || *value == '\t') value++;

    if (!pui_ExpandArg(value, expanded, sizeof(expanded))) return 0;

    return pui_ContextPropertySet(c, local, expanded);
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
