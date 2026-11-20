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
    n->hasColor = s->hasColor;
    n->color = s->color;

    s->w = -1;
    s->h = -1;
    s->anchorBottom = 0;
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

static pui_Node *CloneSiblings(const pui_Node *src, pui_Node *parent)
{
    pui_Node *first = NULL;
    pui_Node *last = NULL;

    for (; src; src = src->next)
    {
        pui_Node *n;

        if (!CheckCount(src->type)) break;

        n = malloc(sizeof(pui_Node));
        if (!n)
        {
            printf("pui: no memory for a ui instance\n");
            break;
        }

        nodeCount++;

        memcpy(n, src, sizeof(pui_Node));

        n->parent = parent;
        n->next = NULL;
        n->kids = CloneSiblings(src->kids, n);

        // the lable is shared memory so every instance is a own copy
        n->label = src->label ? strdup(src->label) : NULL;
        if (src->label && !n->label)
        {
            printf("pui: no memory for a ui instance\n");
            pui_NodeFree(n);

            break;
        }

        if (last) last->next = n;
        else first = n;

        last = n;
    }

    return first;
}

pui_Node *pui_NodeClone(const pui_Node *src)
{
    pui_Node *n;

    if (!src) return NULL;
    if (!CheckCount(src->type)) return NULL;

    n = malloc(sizeof(pui_Node));
    if (!n)
    {
        printf("pui: no memory for a ui instance\n");

        return NULL;
    }

    nodeCount++;

    memcpy(n, src, sizeof(pui_Node));

    n->parent = NULL;
    n->next = NULL;
    n->kids = CloneSiblings(src->kids, n);

    n->label = src->label ? strdup(src->label) : NULL;
    if (src->label && !n->label)
    {
        printf("pui: no memory for a ui instance\n");
        pui_NodeFree(n);

        return NULL;
    }

    return n;
}

int pui_NodeCount(void)
{
    return nodeCount;
}
