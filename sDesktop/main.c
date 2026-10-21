//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: main.c
//

#include <stdio.h>

#include "display/comp.h"
#include "input/input.h"
#include "os.h"
#include "log.h"
#include "cfg.h"
#include "wm/wm.h"
#include "window/window.h"

int main(void)
{
    Log("starting sDesktop for " GREETER);

    sleep(2); // i want to see output text

    compositor_t comp;

    if (CompInit(&comp) == 0)
    {
        Log("could not set up sDesktop");
        return 1;
    }

    Log("framebuffer ");
    printf("%dx%d\n", CompWidth(&comp), CompHeight(&comp));

    //theres no screen with under 100x70.... i hope..
    //if (CompWidth(&comp) <= 100) CompSetSize(&comp, 1024, CompHeight(&comp));
    //if (CompHeight(&comp) <= 70) CompSetSize(&comp, CompWidth(&comp), 768);

    //init mouse, cuz desktop without mouse?? eh
    // keyboard too, same thing
    cursor_t cursor;
    int haveMouse = 0;
    int haveKeyboard = 0;

    for (int i = 0; i < 30 && (!haveMouse || !haveKeyboard); i++)
    {
        if (!haveMouse) haveMouse = OpenMouse();
        if (!haveKeyboard) haveKeyboard = OpenKeyboard();
        if (!haveMouse || !haveKeyboard) usleep(100000);
    }

    if (haveMouse == 0)
    {
        Log("could not open mouse, continuing without it");
    }

    if (haveKeyboard == 0)
    {
        Log("could not open keyboard, continuing without it");
    }

    wm_t wm;
    if (!WMInit(&wm, &comp))
    {
        Log("could not start wm, continuing without it");
    }

    //some demo windows
    window_t *win1 = WMCreateWindow(&wm, 150, 150, 300, 200);
    window_t *win2 = WMCreateWindow(&wm, 280, 280, 250, 150);

    WindowDrawRect(win1, 0, 0, win1->width, win1->height, 0x002222aa);
    WindowDrawRect(win1, 0, 0, win1->width, WM_TITLE_HEIGHT, 0x00444488);
    WindowDrawRect(win1, 0, 0, win1->width, 1, 0x00ffffff);
    WindowDrawRect(win1, 0, win1->height - 1, win1->width, 1, 0x00ffffff);
    WindowDrawRect(win1, 0, 0, 1, win1->height, 0x00ffffff);
    WindowDrawRect(win1, win1->width - 1, 0, 1, win1->height, 0x00ffffff);

    WindowDrawRect(win2, 0, 0, win2->width, win2->height, 0x00aa2222);
    WindowDrawRect(win2, 0, 0, win2->width, WM_TITLE_HEIGHT, 0x00884444);
    WindowDrawRect(win2, 0, 0, win2->width, 1, 0x00ffffff);
    WindowDrawRect(win2, 0, win2->height - 1, win2->width, 1, 0x00ffffff);
    WindowDrawRect(win2, 0, 0, 1, win2->height, 0x00ffffff);
    WindowDrawRect(win2, win2->width - 1, 0, 1, win2->height, 0x00ffffff);

    //default cursor
    MouseSetCursor(&cursor, CUR_NORMAL);

    //bg switching glitched the windows so
    // now its just one color
    //TODO:
    // background
    unsigned int bg = 0x000080ff;
    int noMouseTick = 0;
    int noKbdTick = 0;
    int quit = 0;

    DesktopDraw: {
        //full screen draw at begin
        // just a solid blue background
        createRect(
            &comp,
            0, 0,
            CompWidth(&comp),
            CompHeight(&comp),
            bg
        );

        WMComposite(&wm);
        DrawMouse(&comp, &cursor);
        CompRun(&comp);
    }

    event_queue_t events;
    EventQueueInit(&events);

    printf(
        "input: mouse %s, keyboard %s\n",
        haveMouse ? "ok" : "missing",
        haveKeyboard ? "ok" : "missing"
    );

    DesktopLoop: {
        #ifdef EVENT_DEBUG
            int dbgTick = 0;
        #endif
        for (;!quit;)
        {
            #ifdef EVENT_DEBUG
                printf("tick %d\n", ++dbgTick);
            #endif

            if (!haveMouse)
            {
                if (++noMouseTick >= 120) // 2 secs
                {
                    noMouseTick = 0;
                    haveMouse = OpenMouse();
                    if (haveMouse)
                    {
                        Log("mouse showed up late, nice");
                        //bg = 0x000080ff;
                        //full redraw
                        CompDamage(&comp, 0, 0, CompWidth(&comp), CompHeight(&comp));
                    }
                }
            }

            if (!haveKeyboard)
            {
                if (++noKbdTick >= 120) //same as mouse
                {
                    noKbdTick = 0;
                    haveKeyboard = OpenKeyboard();
                    if (haveKeyboard) Log("keyboard showed up late, nice");
                }
            }

            //all input lands here first
            if (haveMouse && PollMouse(&events) < 0)
            {
                //if the mouse is gone
                // we restart scanning
                CloseMouse();
                haveMouse = 0;
                noMouseTick = 0;
                //bg = 0x00800000;

                CompDamage(&comp, 0, 0, CompWidth(&comp), CompHeight(&comp));
            }

            if (haveKeyboard && PollKeyboard(&events) < 0)
            {
                CloseKeyboard();
                haveKeyboard = 0;
                noKbdTick = 0;
                Log("keyboard is gone, scanning again");
            }

            event_t ev;
            while (EventPop(&events, &ev))
            {
                if (ev.type == EV_MOUSE_MOVE)
                {
                    //the old cursor pos needs a redraw too
                    CompDamage(&comp, cursor.x, cursor.y, cursor.w, cursor.h);

                    CursorMove(
                        &cursor,
                        ev.x,
                        ev.y,
                        CompWidth(&comp),
                        CompHeight(&comp)
                    );

                    CompDamage(&comp, cursor.x, cursor.y, cursor.w, cursor.h);
                    WMHandleInput(&wm, &ev, cursor.x, cursor.y);
                }
                else if (ev.type == EV_MOUSE_BUTTON)
                {
                    //click = booooop :3
                    //printf("boop, %d\n", ev.code);

                    WMHandleInput(&wm, &ev, cursor.x, cursor.y);
                }
                else if (ev.type == EV_KEY)
                {
                    //beep beeeep
                    //printf("beeep, %d %s\n", ev.code, ev.value ? "down" : "up");

                    //we quit with esc in linux
                    if (ev.code == 1 && ev.value) quit = 1;
                }
            }

            if (CompIsDirty(&comp))
            {
                int x, y, w, h;
                //int ix, iy, ex, ey;

                CompDirtyRect(&comp, &x, &y, &w, &h);

                //redraws inside the damage box
                createRect(&comp, x, y, w, h, bg);

                WMCompositeRegion(&wm, x, y, w, h);

                DrawMouse(&comp, &cursor);
            }

            CompRun(&comp);

            //idle
            usleep(16000);
        }
    }

    CompUnInit(&comp);
    return 0;
}
