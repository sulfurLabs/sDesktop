//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: parse.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pui.h>

#include "parse.h"
#include "cfg.h"
#include "draw.h"
#include "registry.h"

//TODO:
// bracket parser
//
static int IsNameChar(int c)
{
    return
        (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == '_' || c == '.'
    ;
}

static int IsSpace(int c)
{
    return
        c == ' '  ||
        c == '\t' ||
        c == '\n' ||
        c == '\r'
    ;
}

static char *SkipSpace(char *p)
{
    while (
        IsSpace(*p) ||
        *p == ','   ||
        *p == ';'   ||
        *p == '{'   ||
        *p == '}'
    ) p++;

    return p;
}

static void TrimTail(char *start, char *end)
{
    while (end > start && IsSpace(end[-1])) end--;
    *end = '\0';
}

static void StripComments(char *s)
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

        //maybe there r slashes
        if (*s == '"')
        {
            *w++ = *s++;
            while (*s && *s != '"') *w++ = *s++;
            if (*s) *w++ = *s++;
            continue;
        }

        *w++ = *s++;
    }

    *w = '\0';
}

static int LoadText(const char *path, char **out)
{
    FILE *f = fopen(path, "rb");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size < 0)
    {
        fclose(f);
        return 0;
    }

    char *text = malloc((size_t)size + 1);
    if (!text)
    {
        fclose(f);
        return 0;
    }

    size_t got = fread(text, 1, (size_t)size, f);

    fclose(f);

    text[got] = '\0';
    *out = text;

    return 1;
}

static void ScanQuote(char **pp, char *buf, int max)
{
    char *cur = *pp;
    int end = 0;

    buf[0] = '\0';
    if (*cur != '"') return;

    cur++;
    while (*cur && *cur != '"')
    {
        if (end < max - 1) buf[end] = *cur;

        end++;
        cur++;
    }

    buf[end < max ? end : max - 1] = '\0';
    if (*cur) cur++;

    *pp = cur;
}

static int IncludePath(const char *current, const char *name, char *out, int max)
{
    size_t nlen = strlen(name);

    const char *slash = strrchr(current, '/');

    if (slash)
    {
        int dirlen = (int)(slash - current);

        if (dirlen + 1 + (int)nlen >= max) return 0;

        memcpy(out, current, (size_t)dirlen);
        out[dirlen] = '/';
        memcpy(out + dirlen + 1, name, nlen + 1);

        return 1;
    }

    if ((int)nlen >= max) return 0;

    memcpy(out, name, nlen + 1);

    return 1;
}

static int ScanCall(char *start, char **after, char *args, int max)
{
    int level = 1;
    char *p = start;

    while (*p)
    {
        if (*p == '"')
        {
            p++;
            while (*p && *p != '"') p++;
            if (*p) p++;

            continue;
        }

        if (*p == '(') level++;
        if (*p == ')') level--;
        if (level == 0) break;

        p++;
    }

    if (!*p) return 0;
    if ((int)(p - start) >= max) return 0;

    memcpy(args, start, (size_t)(p - start));
    args[p - start] = '\0';

    *after = p + 1;

    return 1;
}

static void CallDispatch(const char *name, char *argstart, char **after, const char *path)
{
    char args[PUI_ARGS_MAX];
    const char *where = path ? path : "(nested)";

    if (!ScanCall(argstart, after, args, sizeof(args)))
    {
        printf("%s\n", name);

        *after = argstart + strlen(argstart);

        return;
    }

    if (!pui_Find(name))
    {
        printf("pui: %s not knowing '%s'\n", where, name);

        return;
    }

    if (!pui_RunFunction(name, args))
    {
        printf("pui '%s' failed at/on %s\n", name, where);
    }
}

static void ScanText(const char *path, char *text, int depth)
{
    char *cur = text;

    for (;;)
    {
        cur = SkipSpace(cur);
        if (!*cur) break;

        char *w = cur;
        char name[PUI_NAME_MAX];
        int size;
        while (*cur && IsNameChar((unsigned char)*cur)) cur++;

        size = (int)(cur - w);

        if (*cur == '(')
        {

            if (size >= PUI_NAME_MAX)
            {
                printf("pui: name too long in %s\n", path ? path : "(nested)");

                char rest[PUI_ARGS_MAX];
                if (!ScanCall(cur + 1, &cur, rest, sizeof(rest))) cur++;

                continue;
            }

            memcpy(name, w, (size_t)size);
            name[size] = '\0';

            CallDispatch(name, cur + 1, &cur, path);

            continue;
        }

        if (size == 6 && strncmp(w, "Import", 6) == 0)
        {
            cur = SkipSpace(cur);
            char quotebuf[PUI_NAME_MAX];
            char target[PUI_PATH_MAX] = {0};

            ScanQuote(&cur, quotebuf, (int)sizeof(quotebuf));

            if (*quotebuf == '\0')
            {
                printf("pui: import having no filename %s\n", path);
                continue;
            }
            if (depth >= PUI_DEPTH_MAX)
            {
                printf("pui: import %s to long in %s\n", quotebuf, path);
                continue;
            }
            if (!IncludePath(path, quotebuf, target, sizeof(target)))
            {
                printf("pui: import's '%s' path is too long\n", quotebuf);
                continue;
            }

            pui_ScanFile(target, depth + 1);

            continue;
        }

        if (size == 6 && strncmp(w, "Define", 6) == 0)
        {
            //theres no ";" to tell the end of it so we just wait for new line
            char *line = strchr(cur, '\n');
            char args[PUI_ARGS_MAX];

            if (!line) line = cur + strlen(cur);

            if ((int)(line - cur) >= PUI_ARGS_MAX)
            {
                printf("pui: Define too long in %s\n", path);
                cur = line;

                continue;
            }

            memcpy(args, cur, (size_t)(line - cur));
            args[line - cur] = '\0';
            TrimTail(args, args + strlen(args));

            if (!pui_RunFunction("Define", args))
            {
                printf("pui: Define '%.*s' is broken in %s\n", size, w, path);
            }

            cur = line;

            continue;
        }

        printf("pui: unexpected '%.*s' in %s\n", size ? size : 1, size ? w : cur, path ? path : "(nested)");
        cur += size ? size : 1;
    }
}

int pui_ScanFile(const char *path, int depth)
{
    char *text;
    if (!LoadText(path, &text))
    {
        printf("pui: cannot open '%s'\n", path);
        return 0;
    }

    StripComments(text);

    printf("pui: parsed %s (%u bytes)\n", path, (unsigned)strlen(text));
    ScanText(path, text, depth);
    free(text);

    return 1;
}

int pui_LoadFile(const char *path)
{
    if (!path) return 0;

    pui_FreshDraw();

    return pui_ScanFile(path, 0);
}

void pui_ParseText(const char *text)
{
    if (!text) return;

    char buf[PUI_ARGS_MAX];
    if ((int)strlen(text) >= PUI_ARGS_MAX) return;

    strcpy(buf, text);
    ScanText(NULL, buf, 1);
}
