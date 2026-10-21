//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: wm.c
//

#include <string.h>

#include "wm.h"
#include "../log.h"

static int next_id = 1;

int WMInit(wm_t *wm, compositor_t *comp)
{
    if (!wm || !comp) return 0;

    memset(wm, 0, sizeof(*wm));
    wm->comp = comp;
    wm->count = 0;

    Log("wm started");
    return 1;
}

void WMUnInit(wm_t *wm)
{
    if (!wm) return;

    for (int i = 0; i < wm->count; i++)
    {
        WindowDestroy(wm->windows[i]);
    }

    wm->count = 0;
}

window_t *WMCreateWindow(wm_t *wm, int x, int y, int width, int height)
{
    if (!wm) return NULL;
    if (wm->count >= WINDOW_MAX) return NULL;

    window_t *win = WindowCreate(x, y, width, height);
    if (!win) return NULL;

    win->id = next_id++;
    wm->windows[wm->count] = win;
    wm->count++;

    return win;
}

void WMDestroyWindow(wm_t *wm, window_t *win)
{
    if (!wm || !win) return;

    for (int i = 0; i < wm->count; i++)
    {
        if (wm->windows[i] == win)
        {
            WindowDestroy(win);

            for (int j = i; j < wm->count - 1; j++)
            {
                wm->windows[j] = wm->windows[j + 1];
            }

            wm->count--;
            return;
        }
    }
}

static void WMBlitRect(
    wm_t *wm,
    window_t *win,
    int wx,
    int wy,
    int ww,
    int wh
){
    compositor_t *comp = wm->comp;
    int sw = CompWidth(comp);
    int sh = CompHeight(comp);

    int sx = win->x + wx;
    int sy = win->y + wy;

    //clamp to screen
    if (sx < 0) { ww += sx; sx = 0; }
    if (sy < 0) { wh += sy; sy = 0; }
    if (sx + ww > sw) ww = sw - sx;
    if (sy + wh > sh) wh = sh - sy;
    if (ww <= 0 || wh <= 0) return;

    for (int row = 0; row < wh; row++)
    {
        uint32_t *dst = comp->fb.back + (size_t)(sy + row) * comp->fb.width + sx;
        const uint32_t *src = win->buffer + (size_t)(wy + row) * win->stride + wx;
        for (int col = 0; col < ww; col++) dst[col] = src[col];
    }

    CompDamage(comp, sx, sy, ww, wh);
}

void WMCompositeRegion(wm_t *wm, int x, int y, int w, int h)
{
    if (!wm || !wm->comp) return;
    if (w <= 0 || h <= 0) return;

    compositor_t *comp = wm->comp;
    int sw = CompWidth(comp);
    int sh = CompHeight(comp);

    //clamp to screen
    int dx = x < 0 ? 0 : x;
    int dy = y < 0 ? 0 : y;
    int dw = w;
    int dh = h;

    if (dx + dw > sw) dw = sw - dx;
    if (dy + dh > sh) dh = sh - dy;
    if (dw <= 0 || dh <= 0) return;

    //when a window gets focused it will go into front of z order
    for (int i = 0; i < wm->count; i++)
    {
        window_t *win = wm->windows[i];
        if (!win || !win->visible) continue;
        if (!win->buffer) continue;

        int wx = win->x;
        int wy = win->y;
        int ww = win->width;
        int wh = win->height;

        int ix = dx > wx ? dx : wx;
        int iy = dy > wy ? dy : wy;
        int iendx = dx + dw < wx + ww ? dx + dw : wx + ww;
        int iendy = dy + dh < wy + wh ? dy + dh : wy + wh;

        int iw = iendx - ix;
        int ih = iendy - iy;
        if (iw <= 0 || ih <= 0) continue;

        int srcx = ix - wx;
        int srcy = iy - wy;

        WMBlitRect(wm, win, srcx, srcy, iw, ih);
    }
}

void WMComposite(wm_t *wm)
{
    if (!wm || !wm->comp) return;

    //first draw is a full screen draw
    WMCompositeRegion(wm, 0, 0, CompWidth(wm->comp), CompHeight(wm->comp));
}

void WMBlitWindow(wm_t *wm, window_t *win)
{
    if (!wm || !win) return;
    if (!win->visible) return;
    if (!win->buffer) return;

    WMBlitRect(wm, win, 0, 0, win->width, win->height);
}

window_t *WMWindowAt(wm_t *wm, int x, int y)
{
    if (!wm) return NULL;

    for (int i = wm->count - 1; i >= 0; i--)
    {
        window_t *win = wm->windows[i];
        if (!win || !win->visible) continue;
        if (
            x >= win->x && x < win->x + win->width &&
            y >= win->y && y < win->y + win->height
        ) return win;
    }

    return NULL;
}

void WMHandleInput(wm_t *wm, const event_t *ev, int mx, int my)
{
    if (!wm || !ev) return;

    if (ev->type == EV_MOUSE_BUTTON && ev->code == 1)
    {
        if (ev->value) //left button down
        {
            window_t *win = WMWindowAt(wm, mx, my);
            if (win)
            {
                //now bring clicked window to front
                for (int i = 0; i < wm->count; i++)
                {
                    if (wm->windows[i] == win)
                    {
                        //removes it from its current spot
                        for (int j = i; j < wm->count - 1; j++)
                        {
                            wm->windows[j] = wm->windows[j + 1];
                        }

                        wm->windows[wm->count - 1] = win;

                        break;
                    }
                }

                //it should only move when drag on title
                // but if a app wants to have buttons or areas
                // in itself which can make the window drag then...
                //TODO:
                // make a event for the problem above :D
                if (my >= win->y && my < win->y + WM_TITLE_HEIGHT)
                {
                    wm->dragging = 1;
                    wm->drag_offx = mx - win->x;
                    wm->drag_offy = my - win->y;

                    wm->drag_win = wm->count - 1;
                }
            }
        }
        else //left button up
        {
            wm->dragging = 0;
            wm->drag_win = -1;
        }
    }
    else if (ev->type == EV_MOUSE_MOVE && wm->dragging)
    {
        if (wm->drag_win >= 0 && wm->drag_win < wm->count)
        {
            window_t *win = wm->windows[wm->drag_win];
            if (!win) return;

            int oldx = win->x;
            int oldy = win->y;

            int nx = mx - wm->drag_offx;
            int ny = my - wm->drag_offy;

            WindowMove(win, nx, ny);

            if (wm->comp) CompDamage(wm->comp, oldx, oldy, win->width, win->height);
            if (wm->comp) CompDamage(wm->comp, win->x, win->y, win->width, win->height);
        }
    }
}
