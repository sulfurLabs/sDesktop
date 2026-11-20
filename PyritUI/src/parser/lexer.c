//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: lexer.c
//

#include <string.h>

#include "lexer.h"

int pui_IsNameChar(int c)
{
    return
        (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == '_' || c == '.'
    ;
}

int pui_IsSpace(int c)
{
    return
        c == ' '  ||
        c == '\t' ||
        c == '\n' ||
        c == '\r'
    ;
}

//skips the spaces between calls but not brackets
char *pui_SkipSpace(char *p)
{
    while (
        pui_IsSpace(*p) ||
        *p == ','   ||
        *p == ';'
    ) p++;

    return p;
}

void pui_TrimTail(char *start, char *end)
{
    while (end > start && pui_IsSpace(end[-1])) end--;
    *end = '\0';
}

void pui_StripComments(char *s)
{
    char *w = s;

    while (*s)
    {
        if (s[0] == '/' && s[1] == '/')
        {
            while (*s && *s != '\n') s++;
            continue; //the newline itself stays tho
        }

        if (s[0] == '/' && s[1] == '*')
        {
            s += 2;
            while (*s && !(s[0] == '*' && s[1] == '/')) s++;
            if (*s) s += 2;

            continue;
        }

        //"" stay
        if (*s == '"')
        {
            *w++ = *s++;
            while (*s && *s != '"' && *s != '\n') *w++ = *s++;
            if (*s == '"') *w++ = *s++;
            continue;
        }

        *w++ = *s++;
    }

    *w = '\0';
}

//a broken quote only eats its line, the doc
// after the newline keeps parsing
// returns 0 when the quote never ends
int pui_ScanQuote(char **pp, char *buf, int max)
{
    char *cur = *pp;
    int end = 0;

    buf[0] = '\0';
    if (*cur != '"') return 0;

    int closed = 0;

    cur++;
    while (*cur && *cur != '"' && *cur != '\n')
    {
        if (end < max - 1) buf[end] = *cur;

        end++;
        cur++;
    }

    buf[end < max ? end : max - 1] = '\0';
    if (*cur == '"')
    {
        cur++;
        closed = 1;
    }

    *pp = cur;

    return closed;
}

//the next brace of the flow, a broken call syncs
// there so the doc behind it still parses
static char *SyncBrace(char *p)
{
    int quotes = 1;

    while (*p)
    {
        if (*p == '"') quotes = !quotes;
        else if (*p == '\n') quotes = 1;
        else if (quotes && (*p == '{' || *p == '}')) break;

        p++;
    }

    return p;
}

//scans a (...) call until its closing paren
// the parens inside the args r balanced, parens and
// braces inside strings r protected
// returns 1 with the args filled, 2 when the call ended
// but the args dont fit max, 3 on a unterminated string
// and 0 when the call never closed
// every return fills after with a sync point, so the caller
// can go on behind the broken call instead of eating the doc
int pui_ScanCall(char *start, char **after, char *args, int max)
{
    int level = 1;
    char *p = start;
    int quotes = 1;

    while (*p)
    {
        if (*p == '"')
        {
            quotes = !quotes;
        }
        else if (*p == '\n')
        {
            //a string which never ended breaks the call
            // but only eats its line, not the whole doc
            if (!quotes)
            {
                *after = SyncBrace(p);

                return 3;
            }
        }
        else if (quotes)
        {
            //a brace belongs to the flow, an unclosed
            // call may not run over it
            if (*p == '{' || *p == '}')
            {
                *after = p;

                return 0;
            }

            if (*p == '(') level++;
            if (*p == ')') level--;
        }

        if (level == 0)
        {
            *after = p + 1;

            break;
        }

        p++;
    }

    //the call closed fine
    if (level == 0)
    {
        if ((int)(p - start) >= max) return 2;

        memcpy(args, start, (size_t)(p - start));
        args[p - start] = '\0';

        return 1;
    }

    //the text runs out without a close
    *after = p;

    return quotes != 1 ? 3 : 0;
}
