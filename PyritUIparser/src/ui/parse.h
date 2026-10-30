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

int pui_ScanFile(const char *path, int depth);
int pui_LoadFile(const char *path);
void pui_ParseText(const char *text);

#endif
