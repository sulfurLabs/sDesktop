//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: event.h
//

#pragma once

#include <stdint.h>

//for contributors:
// enable when smth doesnt work with event
#ifdef EVENT_DEBUG
    #include <stdio.h>
#endif

//we have one big ring for all events

#define EV_NONE         0
#define EV_MOUSE_MOVE   1
#define EV_MOUSE_BUTTON 2 //1 left 2 right 3 middle
#define EV_KEY          3

//plain data 16 bytes cuz i want to maaybe track everything
// and put in a file (for list) and its simpler for ipc
typedef struct
{
    uint16_t type;  //one of EV_* above
    uint16_t code;  //key/button
    int32_t value;
    int32_t x;
    int32_t y;
} event_t;

#define EVENT_QUEUE_LEN 128

typedef struct
{
    event_t buf[EVENT_QUEUE_LEN];
    int head; //where the next one goes
    int tail; //where the next one comes from
} event_queue_t;

static inline void EventQueueInit(event_queue_t *q)
{
    q->head = 0;
    q->tail = 0;
}

static inline int EventPush(event_queue_t *q, const event_t *ev)
{
    int next = (q->head + 1) % EVENT_QUEUE_LEN;

    //full means main was too slow, so we drop it
    // 128 per frame is big enough so this wont happen
    if (next == q->tail) return 0;

    q->buf[q->head] = *ev;
    q->head = next;

    #ifdef EVENT_DEBUG
        printf("ev push %d %d %d %d %d\n", ev->type, ev->code, ev->value, ev->x, ev->y);
    #endif

    return 1;
}

static inline int EventPop(event_queue_t *q, event_t *ev)
{
    if (q->head == q->tail) return 0; //their empty

    *ev = q->buf[q->tail];
    q->tail = (q->tail + 1) % EVENT_QUEUE_LEN;

    #ifdef EVENT_DEBUG
        printf("ev pop %d %d %d %d %d\n", ev->type, ev->code, ev->value, ev->x, ev->y);
    #endif

    return 1;
}

static inline void EventPushMove(event_queue_t *q, int dx, int dy)
{
    if (dx == 0 && dy == 0) return;

    event_t ev;
    ev.type = EV_MOUSE_MOVE;
    ev.code = 0;
    ev.value = 0;
    ev.x = dx;
    ev.y = dy;

    EventPush(q, &ev);
}

static inline void EventPushButton(event_queue_t *q, int button, int down)
{
    event_t ev;
    ev.type = EV_MOUSE_BUTTON;
    ev.code = (uint16_t)button;
    ev.value = down ? 1 : 0;
    ev.x = 0;
    ev.y = 0;

    EventPush(q, &ev);
}

static inline void EventPushKey(event_queue_t *q, int code, int down)
{
    event_t ev;

    ev.type = EV_KEY;
    ev.code = (uint16_t)code;
    ev.value = down ? 1 : 0;
    ev.x = 0;
    ev.y = 0;

    EventPush(q, &ev);
}
