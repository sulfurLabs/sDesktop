//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: log.c
//

#include "log.h"
#include "cfg.h"

#include <stdio.h>
#include <unistd.h>

void Log(char *msg)
{
    #if DEBUGOUT == 1
        printf(":: ");
        printf("%s", msg);
        printf("\n");
    #endif
}
