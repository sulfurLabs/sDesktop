//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: cfg.h
//

#pragma once

#define DEBUGOUT 1
#define DPI 144
#define BILINEAL true
//#define SCALE 2

#ifdef __sulfur__
    //sulfurOS doesnt have /home
    #define PUI_DM_CONFIG "/users/pc/.config/desktop/desktop.pui"
#endif
