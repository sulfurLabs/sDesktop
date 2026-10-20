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

    event_queue_t events;
    EventQueueInit(&events);

    printf(
        "input: mouse %s, keyboard %s\n",
        haveMouse ? "ok" : "missing",
        haveKeyboard ? "ok" : "missing"
    );

    cursor.x = CompWidth(&comp) / 2;
    cursor.y = CompHeight(&comp) / 2;

    //this helps a lot on real hardware cuz printf would be overdrawn by graphics
    unsigned int bg = haveMouse ? 0x000080ff : 0x00800000;
    int noMouseTick = 0;
    int noKbdTick = 0;

    DesktopDraw: {
        //full screen draw at begin
        // this will be replaced with a background picture later
        createRect(
            &comp,
            0, 0,
            CompWidth(&comp),
            CompHeight(&comp),
            bg
        );

        createRect(
            &comp,
            100, 100,
            300, 200,
            0x00ff0000
        );

        DrawMouse(&comp, &cursor);

        CompRun(&comp);
    }

    int quit = 0;

    DesktopLoop: {
        //this will be the dirty system
        // or damage selection whatever
        for (;!quit;)
        {
            if (!haveMouse)
            {
                if (++noMouseTick >= 120) // 2 secs
                {
                    noMouseTick = 0;
                    haveMouse = OpenMouse();
                    if (haveMouse)
                    {
                        Log("mouse showed up late, nice");
                        bg = 0x000080ff;
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
                bg = 0x00800000;

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
                    CompDamage(&comp, cursor.x, cursor.y, 12, 18);

                    CursorMove(
                        &cursor,
                        ev.x,
                        ev.y,
                        CompWidth(&comp),
                        CompHeight(&comp)
                    );

                    CompDamage(&comp, cursor.x, cursor.y, 12, 18);
                }
                else if (ev.type == EV_MOUSE_BUTTON && ev.value)
                {
                    //click = booooop :3
                    printf("boop, %d\n", ev.code);

                    //if the background flips the buttons work
                    if (ev.code == 1)
                    {
                        bg = (bg == 0x000080ff) ? 0x0000aa55 : 0x000080ff;
                        CompDamage(&comp, 0, 0, CompWidth(&comp), CompHeight(&comp));
                    }
                }
                else if (ev.type == EV_KEY)
                {
                    //beep beeeep
                    printf("beeep, %d %s\n", ev.code, ev.value ? "down" : "up");

                    //we quit with esc in linux
                    if (ev.code == 1 && ev.value) quit = 1;
                }
            }

            if (CompIsDirty(&comp))
            {
                int x, y, w, h;
                int ix, iy, ex, ey;

                CompDirtyRect(&comp, &x, &y, &w, &h);

                //redraws inside the damage box
                createRect(&comp, x, y, w, h, bg);

                ix = x > 100 ? x : 100;
                iy = y > 100 ? y : 100;
                ex = x + w < 400 ? x + w : 400;
                ey = y + h < 300 ? y + h : 300;

                if (ex > ix && ey > iy)
                {
                    createRect(
                        &comp,
                        ix, iy,
                        ex - ix, ey - iy,
                        0x00ff0000
                    );
                }

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
