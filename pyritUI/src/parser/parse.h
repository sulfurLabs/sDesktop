//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: parse.h
//

#ifndef PARSE_H
#define PARSE_H 1

#include "../cfg.h"
#include "context.h"

typedef struct
{
    char file[PUI_PATH_MAX]; //file which is parsed rn
    int depth; // for includes

    pui_Context *context; //the context SetContext() has selected
    pui_Node *parent; //nesting level
    pui_Node *lastNode; //last element before }‚

    pui_Node *nest[PUI_NEST_MAX];
    int nestCount;

    //pending styles
    int w;
    int h;
    int anchorBottom;
    char name[PUI_NAME_MAX];
    int hasColor;
    unsigned int color;

    int nodes; //element counter for the auto ids
} pui_ParseState;

pui_ParseState *pui_State(void);

//main file + includes
int pui_LoadFile(const char *path);
//one file
int pui_ParseFile(const char *path);
int pui_DispatchArgs(const char *args, char *buf, int max);

void pui_ParseText(const char *text);

#endif
