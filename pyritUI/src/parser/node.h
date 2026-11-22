//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: node.h
//

#ifndef NODE_H
#define NODE_H 1

#include "../cfg.h"

//one element of the ui tree
typedef struct pui_Node
{
    char type[PUI_NAME_MAX]; // type == element/label/command
                             //         (Container,Button,Label,...)
    char id[PUI_NAME_MAX];

    struct pui_Node *parent;
    struct pui_Node *kids; //first child/kid
    struct pui_Node *next; //next (sibling)

    //size
    // -1 == fill
    int w;
    int h;
    int anchorBottom;

    //padding
    // top right bottom left
    int paddingTop;
    int paddingRight;
    int paddingBottom;
    int paddingLeft;

    int hasColor;
    unsigned int color;

    char *label; //raw Label() (text)
} pui_Node;


pui_Node *pui_NodeCreate(const char *type);
pui_Node *pui_NodeRoot(void);

void pui_NodeUse(pui_Node *n);
void pui_NodeFree(pui_Node *n);

int pui_NodeCount(void);

#endif
