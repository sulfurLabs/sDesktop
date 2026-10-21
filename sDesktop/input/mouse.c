//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: mouse.c
//

#include "mouse.h"
#include "cursor.h"
#include "../display/comp.h"
#include "../log.h"
#include "../os.h"
#include <stdio.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#if defined(__linux__) && !defined(__sulfur__)
    #include <sys/ioctl.h>
    #include <linux/input.h>

    static int mouseScore(int fd)
    {
        unsigned long rel[8] = {0};
        unsigned long abs[8] = {0};
        char name[128] = {0};

        #ifdef EVENT_DEBUG
            if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) >= 0)
                printf("reldev: probing %s\n", name);
        #endif

        if (ioctl(fd, EVIOCGBIT(EV_REL, sizeof(rel)), rel) < 0) return 0;
        if (!((rel[0] >> REL_X) & 1UL)) return 0;
        if (!((rel[0] >> REL_Y) & 1UL)) return 0;

        if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs)), abs) >= 0)
        {
            if (abs[0] & 0x3UL) return 1; //if it has ABS X/Y its proly a touchpad
        }

        return 2;
    }
#endif

static int mousefd = -1;
static unsigned char packets[3];
static int packetLen = 0;
static int lastBtn = 0; //so we only report clicks once

#if defined(__linux__) && !defined(__sulfur__)
    static int mouseIsEvdev = 0;
    static unsigned char evbuf[sizeof(struct input_event)];
    static int evLen = 0;

    static int haveAbsX = 0;
    static int haveAbsY = 0;
    static int lastAbsX = 0;
    static int lastAbsY = 0;
#endif

int OpenMouse()
{
    #ifdef __sulfur__ && !defined(__linux__)
        mousefd = open(MS_DEV, O_RDONLY);
        if (mousefd < 0)
        {
            Log("initMouse: failed");
            return 0;
        }

        return 1;
    #elif defined(__linux__) && !defined(__sulfur__)
        //evdev is there cuz of xorg iirc but the /dev/input/mice didnt work
        // before cuz it wasnt loaded in the simple vm i tested in and
        // yea well evdev just works.
        //
        mousefd = -1;
        mouseIsEvdev = 0;
        evLen = 0;
        packetLen = 0;
        lastBtn = 0;
        haveAbsX = 0;
        haveAbsY = 0;

        {
            //for this mouse stuff to work
            // i added different mouse drivers/modules which my
            // badinit for linux loads then hopefully, if not
            // we still have another fallback
            int fallback = -1;

            for (int i = 0; i < 32; i++)
            {
                char path[32];

                snprintf(path, sizeof(path), "/dev/input/event%d", i);
                int fd = open(path, O_RDONLY | O_NONBLOCK);
                if (fd < 0) continue;

                int score = mouseScore(fd);

                #ifdef EVENT_DEBUG
                    printf("reldev: %s score %d\n", path, score);
                #endif

                if (score >= 2)
                {
                    if (fallback >= 0) close(fallback);

                    mousefd = fd;
                    mouseIsEvdev = 1;

                    break;
                }

                if (score >= 1 && fallback < 0) fallback = fd;
                else close(fd);
            }

            //if its not a real mouse we use rel fallback
            if (mousefd < 0 && fallback >= 0)
            {
                mousefd = fallback;
                mouseIsEvdev = 1;
            }

            //but still a fallback incase it doesnt work... somehow.....
            if (mousefd < 0)
            {
                int fd = open(MS_DEV, O_RDONLY | O_NONBLOCK);
                if (fd >= 0)
                {
                    mousefd = fd;
                    mouseIsEvdev = 0;
                }
            }
        }

        if (mousefd < 0) return 0;

        return 1;
    #else
        #error "Unsupported OS"
        return 0;
    #endif
}

int PollMouse(event_queue_t *q)
{
    #if defined(__linux__) && !defined(__sulfur__)
        if (mouseIsEvdev)
        {
            //evdev structure is just the input event
            // and read X and Y coordinate
            // well ofc there is more but i dont need that rn lol
            //
            // also first time i like smth from linux is evdev...
            int dx = 0;
            int dy = 0;
            int got = 0;

            for (;;)
            {
                while (evLen < (int)sizeof(struct input_event))
                {
                    //same thing as before but now with evdev
                    ssize_t n = read(
                        mousefd,
                        evbuf + evLen,
                        sizeof(struct input_event) - evLen
                    );

                    if (n == 0) break; //nothing yet
                    if (n < 0)
                    {
                        //EAGAIN just means empty anything else
                        // means the device is gone like unplugged
                        if (errno != EAGAIN) return -1;
                        break;
                    }
                    evLen += (int)n;
                }

                if (evLen < (int)sizeof(struct input_event)) break;

                evLen = 0;

                struct input_event ev;
                memcpy(&ev, evbuf, sizeof(ev));

                if (ev.type == EV_REL)
                {
                    //we need buttons in future
                    // but eh if i understood correctly those are harder
                    if (ev.code == REL_X)
                    {
                        #ifdef EVENT_DEBUG
                            printf("reldev: REL_X %d\n", ev.value);
                        #endif
                        dx += ev.value;
                        got = 1;
                    }
                    else if (ev.code == REL_Y)
                    {
                        #ifdef EVENT_DEBUG
                            printf("reldev: REL_Y %d\n", ev.value);
                        #endif
                        dy += ev.value;
                        got = 1;
                    }
                }
                else if (ev.type == EV_ABS)
                {
                    if (ev.code == ABS_X)
                    {
                        if (haveAbsX) dx += ev.value - lastAbsX;

                        lastAbsX = ev.value;
                        haveAbsX = 1;
                        got = 1;
                    }
                    else if (ev.code == ABS_Y)
                    {
                        if (haveAbsY) dy += ev.value - lastAbsY;

                        lastAbsY = ev.value;
                        haveAbsY = 1;
                        got = 1;
                    }
                }
                else if (ev.type == EV_KEY)
                {
                    //linux i dont understand you sometimes....
                    if (ev.code == BTN_LEFT) EventPushButton(q, 1, ev.value != 0);
                    else if (ev.code == BTN_RIGHT) EventPushButton(q, 2, ev.value != 0);
                    else if (ev.code == BTN_MIDDLE) EventPushButton(q, 3, ev.value != 0);
                    //meyybe we need rest later
                }
            }

            if (!got) return 0;

            EventPushMove(q, dx, dy);

            return 1;
        }
    #endif

    //hard fallback

    //ps2 protocol
    // 1 = button
    // 2 = move x
    // 3 = move y
    int dx = 0;
    int dy = 0;
    int got = 0;

    for (;;)
    {
        if (packetLen < 3)
        {
            ssize_t n = read(
                mousefd,
                packets + packetLen,
                3 - packetLen
            );

            if (n == 0) break; //nothing yet
            if (n < 0) return -1; //device is gone

            packetLen += (int)n; //it was ssize before
        }

        if (packetLen < 3) break; //not finished package

        //resyncs
        // first package always has 3bytes ig
        if ((packets[0] & 0x08) == 0)
        {
            packets[0] = packets[1];
            packets[1] = packets[2];
            packetLen--;

            continue;
        }

        int btn = packets[0] & 0x07; // left right middle
        if (btn != lastBtn)
        {
            //only what changed or it spams
            for (int b = 0; b < 3; b++)
            {
                int was = (lastBtn >> b) & 1;
                int is = (btn >> b) & 1;
                if (was != is) EventPushButton(q, b + 1, is);
            }
            lastBtn = btn;
        }

        dx += (signed char)packets[1];
        dy += -(signed char)packets[2]; //y goes down
        got = 1;
        packetLen = 0; //set back
    }

    if (!got) return 0;

    EventPushMove(q, dx, dy);

    return 1;
}
/*
typedef struct
{
    int x;
    int y;
} cursor_t;*/

void CursorMove(cursor_t *c, int MoveX, int MoveY, int Width, int Height)
{
    c->x += MoveX;
    c->y += MoveY;

    if (c->x < 0) c->x = 0;
    if (c->y < 0) c->y = 0;
    if (c->x >= Width) c->x = Width - 1;
    if (c->y >= Height) c->y = Height - 1;
}

void DrawMouse(compositor_t *comp, const cursor_t *c)
{
    //picks the texture + size for the current cursor type
    const unsigned int *tex;
    int w;
    int h;

    if (c->type == CUR_HRESIZE)
    {
        tex = HresizeCursor;
        w = CUR_RESIZE_W;
        h = CUR_RESIZE_H;
    }
    else if (c->type == CUR_VRESIZE)
    {
        tex = VresizeCursor;
        w = CUR_RESIZE_W;
        h = CUR_RESIZE_H;
    }
    else if (c->type == CUR_TLBRRESIZE)
    {
        tex = TLBRresizeCursor;
        w = CUR_RESIZE_W;
        h = CUR_RESIZE_H;
    }
    else if (c->type == CUR_TRBLRESIZE)
    {
        tex = TRBLresizeCursor;
        w = CUR_RESIZE_W;
        h = CUR_RESIZE_H;
    }
    else
    {
        tex = NormalCursor;
        w = CUR_NORMAL_W;
        h = CUR_NORMAL_H;
    }

    //draw the texture pixel per pixel
    //0x00000000 is transparent so we skip it
    for (int py = 0; py < h; py++)
    {
        for (int px = 0; px < w; px++)
        {
            unsigned int color = tex[py * w + px];
            if (color != 0x00000000u) putPixel(comp, c->x + px, c->y + py, color);
        }
    }
}

void MouseSetCursor(cursor_t *c, int type)
{
    c->type = type;

    //set the size so the damage rects match the texture
    if (
        type == CUR_HRESIZE    || type == CUR_VRESIZE ||
        type == CUR_TLBRRESIZE || type == CUR_TRBLRESIZE)
    {
        c->w = CUR_RESIZE_W;
        c->h = CUR_RESIZE_H;
    }
    else
    {
        c->w = CUR_NORMAL_W;
        c->h = CUR_NORMAL_H;
    }
}

int InitMouse(mouse *m)
{
    (void)m;
    //TODO:
    // if we have a event for changing cursor icon
    // then here it will set a default icon and load the others
    // programs can switch cursor per event then
    return 0;
}

void CloseMouse(void)
{
    if (mousefd >= 0) close(mousefd);

    mousefd = -1;
    packetLen = 0;
    lastBtn = 0;

    #if defined(__linux__) && !defined(__sulfur__)
        mouseIsEvdev = 0;
        evLen = 0;
        //pads
        haveAbsX = 0;
        haveAbsY = 0;
    #endif
}
