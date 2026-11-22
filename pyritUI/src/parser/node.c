//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: node.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "parse.h"

static int nodeCount;

static int CheckCount(const char *what)
{
    if (nodeCount >= PUI_NODES_MAX)
    {
        printf(
            "pui: too many nodes (%d max), dropped '%s' in %s\n",
            PUI_NODES_MAX,
            what,
            pui_State()->file
        );

        return 0;
    }

    return 1;
}

pui_Node *pui_NodeCreate(const char *type)
{
    pui_ParseState *s = pui_State();
    pui_Node *p;
    pui_Node *n;

    if (!CheckCount(type)) return NULL;

    p = s->parent ? s->parent : (s->context ? s->context->root : NULL);
    if (!p)
    {
        printf(
            "pui: '%s' outside of SetContext in %s\n",
            type,
            s->file
        );

        return NULL;
    }

    n = malloc(sizeof(pui_Node));
    if (!n)
    {
        printf(
            "pui: no memory for '%s' in %s\n",
            type,
            s->file
        );

        return NULL;
    }

    memset(n, 0, sizeof(pui_Node));

    snprintf(
        n->type,
        sizeof(n->type),
        "%s",
        type ? type : ""
    );

    n->parent = p;
    n->w = -1;
    n->h = -1;

    //kids stay in document order
    if (p->kids)
    {
        pui_Node *last = p->kids;
        while (last->next) last = last->next;

        last->next = n;
    }
    else
    {
        p->kids = n;
    }

    nodeCount++;
    s->lastNode = n;

    return n;
}

pui_Node *pui_NodeRoot(void)
{
    pui_Node *n = malloc(sizeof(pui_Node));
    if (!n)
    {
        printf("pui: no memory for a context root\n");

        return NULL;
    }

    memset(n, 0, sizeof(pui_Node));
    snprintf(
        n->type,
        sizeof(n->type),
        "Context"
    );

    nodeCount++;

    return n;
}

void pui_NodeUse(pui_Node *n)
{
    pui_ParseState *s = pui_State();

    if (!n) return;

    if (s->name[0])
    {
        snprintf(
            n->id,
            sizeof(n->id),
            "%s",
            s->name
        );
    }
    else
    {
        snprintf(
            n->id,
            sizeof(n->id),
            "c%d",
            s->nodes
        );
    }

    n->w = s->w;
    n->h = s->h;
    n->anchorBottom = s->anchorBottom;
    n->paddingTop = s->paddingTop;
    n->paddingRight = s->paddingRight;
    n->paddingBottom = s->paddingBottom;
    n->paddingLeft = s->paddingLeft;
    n->hasColor = s->hasColor;
    n->color = s->color;

    s->w = -1;
    s->h = -1;
    s->anchorBottom = 0;
    s->paddingTop = 0;
    s->paddingRight = 0;
    s->paddingBottom = 0;
    s->paddingLeft = 0;
    s->name[0] = '\0';
    s->hasColor = 0;
    s->color = 0;
    s->nodes++;
}

void pui_NodeFree(pui_Node *n)
{
    if (!n) return;

    pui_NodeFree(n->kids);
    pui_NodeFree(n->next);

    if (n->label) free(n->label);

    free(n);

    nodeCount--;
}

int pui_NodeCount(void)
{
    return nodeCount;
}
