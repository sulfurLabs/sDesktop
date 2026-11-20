//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: lexer.h
//

#ifndef LEXER_H
#define LEXER_H 1

int pui_IsNameChar(int c);
int pui_IsSpace(int c);
char *pui_SkipSpace(char *p);
void pui_TrimTail(char *start, char *end);
void pui_StripComments(char *s);

int pui_ScanQuote(char **pp, char *buf, int max);
int pui_ScanCall(char *start, char **after, char *args, int max);

#endif
