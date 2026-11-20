//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: style.c
//

#include <stdio.h>
#include <string.h>

#include "style.h"
#include "../parser/lexer.h"
#include "../parser/parse.h"
#include "../parser/value.h"

static int Pixels(const char *args, int *out)
{
    char value[PUI_ARGS_MAX];

    if (!args) return 0;
    if (!pui_ExpandArg(args, value, sizeof(value))) return 0;

    return pui_ValuePixels(value, out);
}

//the list is seperated by a comma
static int PixelsNext(const char **args, int *out)
{
    char value[PUI_ARGS_MAX];
    int len = 0;

    while (pui_IsSpace((unsigned char)**args))
    {
        (*args)++;
    }

    while (**args && **args != ',')
    {
        if (len >= (int)sizeof(value) - 1) return 0;

        value[len++] = **args;
        (*args)++;
    }

    //trims the tail
    while (len > 0 && pui_IsSpace((unsigned char)value[len - 1])) len--;

    value[len] = '\0';

    if (**args == ',') (*args)++;

    return Pixels(value, out);
}

static int Value(const char *args, char *value, int max)
{
    if (!args) return 0;

    return pui_ExpandArg(args, value, max);
}

//well its the container nameID
int pui_Name(const char *args, void *user)
{
    (void)user;

    return Value(args, pui_State()->name, sizeof(pui_State()->name));
}

int pui_Width(const char *args, void *user)
{
    (void)user;
    return Pixels(args, &pui_State()->w);
}
int pui_Height(const char *args, void *user)
{
    (void)user;
    return Pixels(args, &pui_State()->h);
}

//padding(top, right, bottom, left)
int pui_Padding(const char *args, void *user)
{
    (void)user;

    pui_ParseState *s = pui_State();
    int values[4];

    if (!args) return 0;

    for (int i = 0; i < 4; i++)
    {
        if (!PixelsNext(&args, &values[i])) return 0;
    }

    while (pui_IsSpace((unsigned char)*args)) args++;
    if (*args) return 0;

    s->paddingTop = values[0];
    s->paddingRight = values[1];
    s->paddingBottom = values[2];
    s->paddingLeft = values[3];

    return 1;
}

int pui_Bg(const char *args, void *user)
{
    (void)user;

    pui_ParseState *s = pui_State();
    char value[PUI_ARGS_MAX];

    if (!Value(args, value, sizeof(value))) return 0;
    if (!pui_ValueColor(value, &s->color))
    {
        printf("pui: (bg) cant draw '%s'\n", value);
        return 0;
    }

    s->hasColor = 1;

    return 1;
}

int pui_Anchor(const char *args, void *user)
{
    (void)user;

    pui_ParseState *s = pui_State();
    char value[PUI_ARGS_MAX];

    if (!Value(args, value, sizeof(value))) return 0;

    s->anchorBottom = !strcmp(value, "bottom");

    return 1;
}

//Style(...) is just args in extra call
// they run like normal pui text
int pui_Style(const char *args, void *user)
{
    (void)user;

    char buf[PUI_ARGS_MAX];

    return pui_DispatchArgs(args, buf, sizeof(buf));
}
