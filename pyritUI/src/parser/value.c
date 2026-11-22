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
#include <limits.h>

#include "value.h"
#include "commands.h"

static pui_DefineEntry defines[PUI_DEFINES_MAX];
static int defineCount;

int pui_DefineSet(const char *name, const char *value)
{
    if (!name || !value) return 0;
    if (strlen(name) >= PUI_NAME_MAX) return 0;
    if (pui_DefineGet(name)) return 1; //redefines r ignored
    if (defineCount >= PUI_DEFINES_MAX) return 0;

    pui_DefineEntry *d = &defines[defineCount];

    //store the raw value
    // when its used it expands
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

void pui_DefineClear(void)
{
    for (int i = 0; i < defineCount; i++)
    {
        free(defines[i].name);
        free(defines[i].value);
    }

    defineCount = 0;
}

void pui_DefineDiag(void)
{
    printf("pui: %d defines\n", defineCount);

    for (int i = 0; i < defineCount; i++)
    {
        printf("pui: define %s = %s\n", defines[i].name, defines[i].value);
    }
}

int pui_DefineCount(void)
{
    return defineCount;
}

const char *pui_DefineName(int index)
{
    if (index < 0 || index >= defineCount) return NULL;

    return defines[index].name;
}
const char *pui_DefineValue(int index)
{
    if (index < 0 || index >= defineCount) return NULL;

    return defines[index].value;
}

//sulfur doesnt have strtoul yet
//TODO:
// sulflibc needs strtoul
// remove this then
static int ParseUInt(const char **pp, unsigned int max, unsigned int *out)
{
    const char *p = *pp;
    unsigned int v = 0;

    if (*p < '0' || *p > '9') return 0;

    while (*p >= '0' && *p <= '9')
    {
        unsigned int digit = (unsigned int)(*p - '0');

        if (digit > max || v > (max - digit) / 10) return 0;

        v = v * 10 + digit;
        p++;
    }

    *pp = p;
    *out = v;

    return 1;
}

static const char *SkipSpace(const char *p)
{
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;

    return p;
}

static int HexDigit(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;

    return -1;
}

int pui_ValueColor(const char *value, unsigned int *out)
{
    if (!value || !out) return 0;

    unsigned int slots[4];
    int want = 0;
    //int count = 0;

    const char *p = SkipSpace(value);

    //TODO
    // bracket parser
    if (!strncmp(p, "argb(", 5))
    {
        p += 5;
        want = 4;
    }
    else if (!strncmp(p, "rgb(", 4))
    {
        p += 4;
        want = 3;
    }
    else if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
    {
        const char *hex = p + 2;
        int digits = 0;
        unsigned int v = 0;

        while (HexDigit((unsigned char)hex[digits]) >= 0) digits++;

        //0xRRGGBB or 0xAARRGGBB but nothin else
        if (digits != 6 && digits != 8) return 0;

        for (int i = 0; i < digits; i++)
        {
            v = v << 4 | (unsigned int)HexDigit((unsigned char)hex[i]);
        }

        p = SkipSpace(hex + digits);
        if (*p) return 0;

        *out = digits == 6 ? 0xFF000000u | v : v;

        return 1;
    }

    if (want)
    {
        for (int i = 0; i < want; i++)
        {
            p = SkipSpace(p);
            if (!ParseUInt(&p, 255, &slots[i])) return 0;
            p = SkipSpace(p);

            if (i + 1 < want)
            {
                if (*p != ',') return 0;
                p++;
            }
        }

        if (*p != ')') return 0;

        p = SkipSpace(p + 1);
        if (*p) return 0;

        unsigned int a = 255, r = slots[0], g = slots[1], b = slots[2];

        if (want == 4)
        {
            a = slots[0];
            r = slots[1];
            g = slots[2];
            b = slots[3];
        }

        *out = a << 24 | r << 16 | g << 8 | b;

        return 1;
    }

    unsigned int gray;

    if (!ParseUInt(&p, 255, &gray)) return 0;

    p = SkipSpace(p);
    if (*p) return 0;

    *out = 255u << 24 | gray << 16 | gray << 8 | gray;

    return 1;
}

int pui_ValuePixels(const char *value, int *out)
{
    if (!value || !out) return 0;

    const char *p = SkipSpace(value);
    unsigned int v;

    if (!strncmp(p, "fill", 4))
    {
        p = SkipSpace(p + 4);
        if (*p) return 0;

        *out = -1;

        return 1;
    }

    //TODO:
    // yea... px( isnt a great solution, we need a function which automatically detects brackets etc
    if (!strncmp(p, "px(", 3))
    {
        p = SkipSpace(p + 3);
        if (!ParseUInt(&p, (unsigned int)INT_MAX, &v)) return 0;

        p = SkipSpace(p);
        if (*p != ')') return 0;

        p = SkipSpace(p + 1);
        if (*p) return 0;

        *out = (int)v;

        return 1;
    }

    //plain digits have the px suffix
    if (!ParseUInt(&p, (unsigned int)INT_MAX, &v)) return 0;

    if (p[0] == 'p' && p[1] == 'x') p += 2;

    p = SkipSpace(p);
    if (*p) return 0;

    *out = (int)v;

    return 1;
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
            printf("pui: define '%s' chained to strong\n", arg);
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

const char *pui_Lookup(const pui_Var *vars, int varCount, const char *name)
{
    if (!name) return NULL;

    const char *val = pui_Value(name);
    if (val) return val;

    //the value a instance owns
    for (int i = 0; i < varCount; i++)
    {
        if (strcmp(vars[i].name, name) == 0) return vars[i].value;
    }

    return pui_DefineGet(name);
}

int pui_ResolveLabel(
    const char *raw,
    const pui_Var *vars,
    int varCount,
    char *out,
    int max
){
    if (!raw || !out || max <= 0) return 0;

    const char *p = raw;
    while (*p == ' ' || *p == '\t') p++;

    //GetArg(name) or Bind(name) read from the live values
    //TODO:
    // again the brackets...
    if (!strncmp(p, "GetArg(", 7) || !strncmp(p, "Bind(", 5))
    {
        const char *inner = strchr(p, '(') + 1;
        const char *end = strrchr(inner, ')');
        const char *val;
        char name[PUI_NAME_MAX];

        if (!end || inner == end) return 0;
        if ((int)(end - inner) >= PUI_NAME_MAX) return 0;

        memcpy(name, inner, (size_t)(end - inner));
        name[end - inner] = '\0';
        val = pui_Lookup(vars, varCount, name);

        if (!val) return 0;
        if ((int)strlen(val) >= max) return 0;

        strcpy(out, val);

        return (int)strlen(val);
    }

    //plain text or a define name
    const char *val = pui_Value(p);
    if (val)
    {
        if ((int)strlen(val) >= max) return 0;

        strcpy(out, val);

        return (int)strlen(val);
    }

    return pui_ExpandArg(p, out, max);
}
