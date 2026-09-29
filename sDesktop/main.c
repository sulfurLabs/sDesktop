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
    cursor_t cursor;
    int haveMouse = 0;

    for (int i = 0; i < 30 && !haveMouse; i++)
    {
        haveMouse = OpenMouse();
        if (!haveMouse) usleep(100000);
    }

    if (haveMouse == 0)
    {
        Log("could not open mouse, continuing without it");
    }

    cursor.x = CompWidth(&comp) / 2;
    cursor.y = CompHeight(&comp) / 2;

    //this helps a lot on real hardware cuz printf would be overdrawn by graphics
    unsigned int bg = haveMouse ? 0x000080ff : 0x00800000;
    int noMouseTick = 0;

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

    DesktopLoop: {
        //this will be the dirty system
        // or damage selection whatever
        for (;;)
        {
            int MoveX = 0;
            int MoveY = 0;

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

            int r = haveMouse ? ReadMouse(&MoveX, &MoveY) : 0;
            if (r < 0)
            {
                //if the mouse is gone
                // we restart scanning
                CloseMouse();
                haveMouse = 0;
                noMouseTick = 0;
                bg = 0x00800000;

                CompDamage(&comp, 0, 0, CompWidth(&comp), CompHeight(&comp));
            }
            else if (r)
            {
                if (MoveX || MoveY)
                {
                    //the old cursor pos needs a redraw too
                    CompDamage(&comp, cursor.x, cursor.y, 12, 18);

                    CursorMove(
                        &cursor,
                        MoveX,
                        MoveY,
                        CompWidth(&comp),
                        CompHeight(&comp)
                    );

                    CompDamage(&comp, cursor.x, cursor.y, 12, 18);
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
