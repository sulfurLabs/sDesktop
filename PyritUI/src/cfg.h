//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: cfg.h
//

#ifndef PUI_CFG_H
#define PUI_CFG_H 1

#define PUI_NAME_MAX          32 //names defines and id
#define PUI_ARGS_MAX        1024 //nested calls r many args
#define PUI_PATH_MAX         256
#define PUI_DEPTH_MAX          8 //nesting limit (define + include files)
#define PUI_NEST_MAX          32
#define PUI_NODES_MAX        512
#define PUI_INCLUDES_MAX      16
#define PUI_SCAN_MAX           8 //nested args text depth
#define PUI_FUNCTIONS_MAX     64 //the stylesheet should still be simple ig
#define PUI_DEFINES_MAX       32
#define PUI_INSTANCE_VARS_MAX 16 //instance copies of the <context>.* defines

//default styles
#define PUI_BUTTON_W 64 //otherwise the button wouldnt be visible

#endif
