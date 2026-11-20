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
#include "lexer.h"
#include "context.h"
#include "value.h"
#include "commands.h"

static pui_ParseState state;

//includes only load once
static char loaded[PUI_INCLUDES_MAX][PUI_PATH_MAX];
static int loadedCount;

//smth broken stays
static int scanDepth;

pui_ParseState *pui_State(void)
{
    return &state;
}

//to make fancy error output with lines :D
static int RowOf(const char *doc, const char *p)
{
    int line = 1;

    if (!doc) return 0;

    for (const char *c = doc; *c && c < p; c++)
    {
        if (*c == '\n') line++;
    }

    return line;
}

//%*s crashes on sulfurOS, means we remove it
static const char *Snip(const char *w, int size, char *buf, int max)
{
    if (size > max - 1) size = max - 1;
    if (size < 0) size = 0;

    memcpy(buf, w, (size_t)size);
    buf[size] = '\0';

    return buf;
}

static int LoadedHas(const char *path)
{
    for (int i = 0; i < loadedCount; i++)
    {
        if (strcmp(loaded[i], path) == 0) return 1;
    }

    return 0;
}
static void LoadedAdd(const char *path)
{
    if (loadedCount >= PUI_INCLUDES_MAX) return;

    snprintf(loaded[loadedCount], PUI_PATH_MAX, "%s", path);
    loadedCount++;
}

static void CallDispatch(const char *doc, const char *name, char *argstart, char **after)
{
    char args[PUI_ARGS_MAX];
    char snip[PUI_NAME_MAX];
    int fit = pui_ScanCall(argstart, after, args, sizeof(args));

    //everything after broken stuff stays alive tho
    if (fit == 0 || fit == 3)
    {
        printf(
            "pui: line %d: %s( %s in %s\n",
            RowOf(doc, argstart), name,
            fit == 3 ? "string not ended" : "missing ')'",
            state.file
        );

        return;
    }

    //call ended but args r too long
    if (fit == 2)
    {
        printf(
            "pui: line %d: %s( args too long in %s\n",
            RowOf(doc, argstart), name, state.file
        );

        return;
    }

    if (!pui_Find(name))
    {
        printf(
            "pui: line %d: not knowing '%s' in %s\n",
            RowOf(doc, argstart),
            Snip(
                name,
                (int)strlen(name),
                snip,
                sizeof(snip)
            ),
            state.file
        );

        return;
    }

    if (!pui_RunFunction(name, args))
    {
        printf(
            "pui: line %d: '%s' failed at '%s'  in %s\n",
            RowOf(doc, argstart),
            name,
            Snip(
                args,
                24,
                snip,
                sizeof(snip)
            ),
            state.file
        );
    }
}

//Include("file")/Import("")
static void ParseInclude(const char *doc, const char *name, char *cur, char **after)
{
    char quotebuf[PUI_PATH_MAX];
    char target[PUI_PATH_MAX] = {0};
    char snip[PUI_PATH_MAX];

    cur = pui_SkipSpace(cur);

    if (*cur == '(')
    {
        cur++;
        cur = pui_SkipSpace(cur);

        if (!pui_ScanQuote(&cur, quotebuf, (int)sizeof(quotebuf)))
        {
            *after = cur;

            printf(
                "pui: line %d: %s string not ended in %s\n",
                RowOf(doc, cur),
                name,
                state.file
            );

            return;
        }

        cur = pui_SkipSpace(cur);
        if (*cur == ')') cur++;
    }
    else
    {
        if (!pui_ScanQuote(&cur, quotebuf, (int)sizeof(quotebuf)))
        {
            *after = cur;

            printf(
                "pui: line %d: %s string not ended in %s\n",
                RowOf(doc, cur),
                name,
                state.file
            );

            return;
        }
    }

    *after = cur;

    if (!quotebuf[0])
    {
        printf(
            "pui: line %d: %s having no filename in %s\n",
            RowOf(doc, cur),
            name,
            state.file
        );

        return;
    }
    if (state.depth >= PUI_DEPTH_MAX)
    {
        printf(
            "pui: line %d: include '%s' nested to deep in %s\n",
            RowOf(doc, cur),
            Snip(
                quotebuf,
                24,
                snip,
                sizeof(snip)
            ),
            state.file
        );

        return;
    }

    //includes are always in the same
    // directory as the file itself
    const char *slash = strrchr(state.file, '/');
    if (slash)
    {
        size_t dirlen = (size_t)(slash - state.file);

        if (dirlen + 1 + strlen(quotebuf) >= PUI_PATH_MAX)
        {
            printf(
                "pui: line %d: include '%s' path too long\n",
                RowOf(doc, cur),
                Snip(
                    quotebuf,
                    24,
                    snip,
                    sizeof(snip)
                )
            );

            return;
        }

        memcpy(target, state.file, dirlen);
        target[dirlen] = '/';

        strcpy(target + dirlen + 1, quotebuf);
    }
    else
    {
        snprintf(target, sizeof(target), "%s", quotebuf);
    }

    if (LoadedHas(target)) return;

    if (!pui_ParseFile(target))
    {
        printf(
            "pui: line %d: %s could not load '%s' in %s\n",
            RowOf(doc, cur),
            name,
            target,
            state.file
        );
    }
}

static void ScanText(char *text)
{
    char *cur = text;
    char *doc = text;

    //if a nested is broken it doesnt ercurse forever
    if (scanDepth >= PUI_SCAN_MAX) return;
    scanDepth++;

    for (;;)
    {
        cur = pui_SkipSpace(cur);
        if (!*cur) break;

        //everything inside brackets is part of
        // the element before the brackets
        if (*cur == '{')
        {
            if (state.lastNode)
            {
                if (state.nestCount < PUI_NEST_MAX)
                {
                    state.nest[state.nestCount++] = state.parent;
                    state.parent = state.lastNode;
                }
                else
                {
                    printf(
                        "pui: line %d: braces nested to deep in %s\n",
                        RowOf(doc, cur),
                        state.file
                    );
                }
            }
            else
            {
                printf(
                    "pui: line %d: '{' with nothing before it in %s\n",
                    RowOf(doc, cur),
                    state.file
                );
            }

            cur++;

            continue;
        }

        if (*cur == '}')
        {
            if (state.nestCount > 0) state.parent = state.nest[--state.nestCount];
            else printf("pui: line %d: unexpected '}' in %s\n", RowOf(doc, cur), state.file);

            cur++;

            continue;
        }

        char *w = cur;
        char snip[PUI_NAME_MAX];
        char name[PUI_NAME_MAX];
        int size;

        while (*cur && pui_IsNameChar((unsigned char)*cur)) cur++;

        size = (int)(cur - w);

        if (*cur == '(')
        {
            if (size >= PUI_NAME_MAX)
            {
                printf(
                    "pui: line %d: name too long in %s\n",
                    RowOf(doc, w),
                    state.file
                );

                char rest[PUI_ARGS_MAX];
                if (pui_ScanCall(cur + 1, &cur, rest, sizeof(rest)) == 0) cur++;

                continue;
            }

            memcpy(name, w, (size_t)size);
            name[size] = '\0';

            //include or import, idk what to use tbh..
            // so we support both
            if (size == 7 && !strcmp(name, "Include"))
            {
                ParseInclude(doc, name, cur + 1, &cur);
                continue;
            }
            if (size == 6 && !strcmp(name, "Import"))
            {
                ParseInclude(doc, name, cur + 1, &cur);
                continue;
            }

            if (size == 6 && !strcmp(name, "Define"))
            {
                //defines dont have ; so we wait for new line
                char *line = strchr(cur, '\n');
                char args[PUI_ARGS_MAX];

                if (!line) line = cur + strlen(cur);

                if ((int)(line - cur) >= PUI_ARGS_MAX)
                {
                    printf(
                        "pui: line %d: Define too long in %s\n",
                        RowOf(doc, cur),
                        state.file
                    );

                    cur = line;

                    continue;
                }

                memcpy(args, cur, (size_t)(line - cur));
                args[line - cur] = '\0';
                pui_TrimTail(args, args + strlen(args));

                if (!pui_RunFunction("Define", args))
                {
                    printf(
                        "pui: line %d: Define '%s' is broken in %s\n",
                        RowOf(doc, w),
                        Snip(
                            w,
                            24,
                            snip,
                            sizeof(snip)
                        ),
                        state.file
                    );
                }

                cur = line;

                continue;
            }

            CallDispatch(doc, name, cur + 1, &cur);

            continue;
        }

        //Import "file"
        // Include "file"
        if (size == 6 && !strncmp(w, "Import", 6))
        {
            ParseInclude(doc, "Import", cur, &cur);
            continue;
        }
        if (size == 7 && !strncmp(w, "Include", 7))
        {
            ParseInclude(doc, "Include", cur, &cur);
            continue;
        }

        //Define name value needs no parens
        if (size == 6 && !strncmp(w, "Define", 6))
        {
            char *line = strchr(cur, '\n');
            char args[PUI_ARGS_MAX];

            if (!line) line = cur + strlen(cur);

            if ((int)(line - cur) >= PUI_ARGS_MAX)
            {
                printf(
                    "pui: line %d: Define too long in %s\n",
                    RowOf(doc, cur),
                    state.file
                );
                cur = line;

                continue;
            }

            memcpy(args, cur, (size_t)(line - cur));
            args[line - cur] = '\0';
            pui_TrimTail(args, args + strlen(args));

            if (!pui_RunFunction("Define", args))
            {
                printf(
                    "pui: line %d: Define '%s' is broken in %s\n",
                    RowOf(doc, w),
                    Snip(
                        w,
                        24,
                        snip,
                        sizeof(snip)
                    ),
                    state.file
                );
            }

            cur = line;

            continue;
        }

        printf(
            "pui: line %d: unexpected '%s' in %s\n",
            RowOf(doc, w),
            Snip(
                w,
                size ? size : 1,
                snip,
                sizeof(snip)
            ),
            state.file
        );

        cur += size ? size : 1;
    }

    scanDepth--;
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
        printf("pui: no memory for '%s'\n", path);

        fclose(f);
        return 0;
    }

    size_t got = fread(text, 1, (size_t)size, f);

    fclose(f);

    text[got] = '\0';
    *out = text;

    return 1;
}

int pui_ParseFile(const char *path)
{
    char *text;
    int oldDepth;
    char oldFile[PUI_PATH_MAX];

    if (!path) return 0;

    text = NULL;
    if (!LoadText(path, &text))
    {
        printf("pui: cannot open '%s'\n", path);

        return 0;
    }

    pui_StripComments(text);

    printf("pui: parsed %s (%u bytes)\n", path, (unsigned)strlen(text));

    //keeps parsing after nested functions
    snprintf(oldFile, sizeof(oldFile), "%s", state.file);
    snprintf(state.file, sizeof(state.file), "%s", path);

    oldDepth = state.depth;
    state.depth++;

    LoadedAdd(path);
    ScanText(text);

    state.depth = oldDepth;
    snprintf(state.file, sizeof(state.file), "%s", oldFile);

    free(text);

    return 1;
}

int pui_LoadFile(const char *path)
{
    if (!path) return 0;

    pui_ContextClear();
    pui_DefineClear();
    loadedCount = 0;

    state.context = NULL;
    state.parent = NULL;
    state.lastNode = NULL;
    state.nestCount = 0;
    state.depth = 0;
    state.file[0] = '\0';

    //old doc stuff gets reset
    state.w = -1;
    state.h = -1;
    state.anchorBottom = 0;
    state.paddingTop = 0;
    state.paddingRight = 0;
    state.paddingBottom = 0;
    state.paddingLeft = 0;
    state.name[0] = '\0';
    state.hasColor = 0;
    state.color = 0;

    return pui_ParseFile(path);
}

void pui_ParseText(const char *text)
{
    if (!text) return;

    char buf[PUI_ARGS_MAX];
    if ((int)strlen(text) >= PUI_ARGS_MAX) return;

    strcpy(buf, text);
    ScanText(buf);
}

int pui_DispatchArgs(const char *args, char *buf, int max)
{
    if ((int)strlen(args) >= max) return 0;

    strcpy(buf, args);
    pui_ParseText(buf);

    return 1;
}
