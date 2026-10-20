//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: keyboard.h
//

#pragma once

#include "event.h"

int OpenKeyboard(void);
void CloseKeyboard(void);
int PollKeyboard(event_queue_t *q);
