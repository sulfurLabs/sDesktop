//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: wm.h
//

#ifndef WM_H
#define WM_H 1

#include "../window/window.h"
#include "../display/comp.h"
#include "../input/event.h"

#define WM_TITLE_HEIGHT 20

typedef struct
{
    window_t *windows[WINDOW_MAX];
    int count;

    compositor_t *comp;

    //window drag/movement
    int dragging;
    int drag_win;
    int drag_offx;
    int drag_offy;
} wm_t;

int WMInit(wm_t *wm, compositor_t *comp);
void WMUnInit(wm_t *wm);

window_t *WMCreateWindow(wm_t *wm, int x, int y, int width, int height);
void WMDestroyWindow(wm_t *wm, window_t *win);
void WMCompositeRegion(wm_t *wm, int x, int y, int w, int h);
void WMComposite(wm_t *wm);
void WMBlitWindow(wm_t *wm, window_t *win);
void WMHandleInput(wm_t *wm, const event_t *ev, int mx, int my);

window_t *WMWindowAt(wm_t *wm, int x, int y);

#endif
