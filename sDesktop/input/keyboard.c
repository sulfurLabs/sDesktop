//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: keyboard.c
//

#include "keyboard.h"
#include "../log.h"
#include "../os.h"
#include <stdio.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

static int kbdfd = -1;

#if defined(__linux__) && !defined(__sulfur__)
    #include <sys/ioctl.h>
    #include <linux/input.h>

    static unsigned char evbuf[sizeof(struct input_event)];
    static int evLen = 0;

    static int hasKey(unsigned long *key, int code)
    {
        return (key[code / (int)(8 * sizeof(long))] >> (code % (int)(8 * sizeof(long)))) & 1UL;
    }

    static int keyboardScore(int fd)
    {
        unsigned long key[8] = {0};

        if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key)), key) < 0) return 0;

        //f linux with its keyboard and powerbutton being the sameeee >:c
        if (hasKey(key, KEY_SPACE)) return 2;
        if (hasKey(key, KEY_ENTER)) return 2;
        if (hasKey(key, KEY_A)) return 2;

        return 0;
    }
#endif

int OpenKeyboard()
{
    //well sulfur is the main OS here cuz well sDesktop is built for sulfur..
    // for replacement of s4
    #if defined(__sulfur__)
        kbdfd = open(KB_DEV, O_RDONLY);
        if (kbdfd < 0)
        {
            Log("initKeyboard: failed");
            return 0;
        }

        return 1;
    #elif defined(__linux__) && !defined(__sulfur__)
        kbdfd = -1;
        evLen = 0;

        for (int i = 0; i < 32; i++)
        {
            char path[32];

            //TODO:
            // there are other keyboard devices,
            // make mechanism which chooses the best ig
            // (yes i might be wrong, idk much abt linux duh)
            snprintf(path, sizeof(path), "/dev/input/event%d", i);
            int fd = open(path, O_RDONLY | O_NONBLOCK);
            if (fd < 0) continue;

            if (keyboardScore(fd) >= 2)
            {
                kbdfd = fd;
                break;
            }

            close(fd);
        }

        if (kbdfd < 0) return 0;

        return 1;
    #else
        #error "Unsupported OS"
        return 0;
    #endif
}

int PollKeyboard(event_queue_t *q)
{
    #if defined(__linux__) && !defined(__sulfur__)
        for (;;)
        {
            while (evLen < (int)sizeof(struct input_event))
            {
                ssize_t n = read(
                    kbdfd,
                    evbuf + evLen,
                    sizeof(struct input_event) - evLen
                );

                if (n == 0) break; //nothing yet
                if (n < 0)
                {
                    //EAGAIN just means empty
                    // anything else means unplugged
                    if (errno != EAGAIN) return -1;
                    break;
                }
                evLen += (int)n;
            }

            if (evLen < (int)sizeof(struct input_event)) break;

            evLen = 0;

            struct input_event ev;
            memcpy(&ev, evbuf, sizeof(ev));

            if (ev.type == EV_KEY)
            {
                //repeat (2) is still down
                EventPushKey(q, ev.code, ev.value != 0);
            }
        }

        return 0;
    #elif defined(__sulfur__)
        //sulfur gives already finished events so we can use them
        input_event_t evs[16];
        long n = read(kbdfd, evs, sizeof(evs));

        if (n < 0) return -1; //device is gone

        for (long i = 0; i + (long)sizeof(input_event_t) <= n; i += (long)sizeof(input_event_t))
        {
            input_event_t *ev = (input_event_t *)((unsigned char *)evs + i);
            if (ev->type == INPUT_EV_KEY) EventPushKey(q, ev->code, ev->value != 0);
        }

        return 0;
    #else
        #error "Unsupported OS"
        return 0;
    #endif
}

void CloseKeyboard(void)
{
    if (kbdfd >= 0) close(kbdfd);

    kbdfd = -1;

    #if defined(__linux__) && !defined(__sulfur__)
        evLen = 0;
    #endif
}
