//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: pui.c
//

#include <stdio.h>
#include <stdlib.h>
#include <pui.h>

#include "pui.h"
#include "cfg.h"
#include "log.h"
#include "wm/wm.h"

// Window properties
static const char *PyritUIWindowTitle(void *user)
{
    (void)user;
    return "sDesktop";
}

static char PyritUIDesktopValue[3][24];
static char PyritUIWindowValue[24];

static const char *PyritUIDesktopWidth(void *user)
{
    compositor_t *comp = user;
    snprintf(
        PyritUIDesktopValue[0],
        sizeof(PyritUIDesktopValue[0]),
        "%d",
        CompWidth(comp)
    );

    return PyritUIDesktopValue[0];
}
static const char *PyritUIDesktopHeight(void *user)
{
    compositor_t *comp = user;
    snprintf(
        PyritUIDesktopValue[1],
        sizeof(PyritUIDesktopValue[1]),
        "%d",
        CompHeight(comp)
    );

    return PyritUIDesktopValue[1];
}
static const char *PyritUIDesktopPixels(void *user)
{
    compositor_t *comp = user;
    snprintf(
        PyritUIDesktopValue[2],
        sizeof(PyritUIDesktopValue[2]),
        "%d",
        CompWidth(comp) * CompHeight(comp)
    );

    return PyritUIDesktopValue[2];
}

static const char *PyritUIWindowWidth(void *user)
{
    window_t *win = user;
    snprintf(
        PyritUIWindowValue,
        sizeof(PyritUIWindowValue),
        "%d",
        win ? win->width : 0
    );

    return PyritUIWindowValue;
}
static const char *PyritUIWindowHeight(void *user)
{
    window_t *win = user;
    snprintf(
        PyritUIWindowValue,
        sizeof(PyritUIWindowValue),
        "%d",
        win ? win->height : 0
    );

    return PyritUIWindowValue;
}
static const char *PyritUIWindowZ(void *user)
{
    (void)user;
    return "0";
}

void PyritUILoad(compositor_t *comp, window_t *win)
{
    int w = CompWidth(comp);
    int h = CompHeight(comp);

    if (!pyrit.Init()) return;

    static pui_Target screen;
    static pui_Target windowTarget;
    screen = (pui_Target){ comp->fb.back, 0, 0, w, h };
    windowTarget = (pui_Target){ win->buffer, win->x, win->y, win->width, win->height };

    pyrit.Context("Desktop", &screen);
    pyrit.RegisterProperty("Desktop.width", PyritUIDesktopWidth, comp);
    pyrit.RegisterProperty("Desktop.height", PyritUIDesktopHeight, comp);
    pyrit.RegisterProperty("Desktop.pixels", PyritUIDesktopPixels, comp);

    pyrit.Context("Window", &windowTarget);
    pyrit.RegisterProperty("Window.title", PyritUIWindowTitle, win);
    pyrit.RegisterProperty("Window.width", PyritUIWindowWidth, win);
    pyrit.RegisterProperty("Window.height", PyritUIWindowHeight, win);
    pyrit.RegisterProperty("Window.z", PyritUIWindowZ, win);

    pyrit.RegisterFunction("SetContext", pyrit.SetContext, NULL);
    //i want to create windows per pui so i can make buttons like start button, but i also need a popup version then
    // so a plan would be to just make a Window and give it properties like SD_POPUP or SD_WIN
    //
    // or we just register, CreateWindow and CreatePopup.
    //
    // or we add the logic for PyritUI to register function names themself, and save those names as a function which can be called in
    // PyritUI itself and we just tell the parser to e.g set properties like z-order or coordinates, etc
    //
    // havent decided what to choose yet, so this will stay a stupid comment.
    //pyrit.RegisterFunction("CreateWindow", WMCreateWindow(comp->fb.back, 0, 0, ..., ...))

    //linux has env but sulfurOS not so
    // sulfd config path is used
    const char *home = getenv("HOME");
    char path[256];

    if (
        home &&
        snprintf(path, sizeof(path), "%s/.config/desktop/desktop.pui", home) > 0 &&
        pyrit.LoadFile(path)
    ){
        return;
    }

    #ifdef __sulfur__
        const char *fallback = PUI_DM_CONFIG;
    #else
        //sulfurOS name is always pc for now
        const char *fallback = "/users/pc/.config/desktop/desktop.pui";
    #endif

    if (!pyrit.LoadFile(fallback))
    {
        Log("could not load the pui config, coded ui stays");
    }

    #if DEBUGOUT
        //shows what the config pushed, catches stale configs fast
        pyrit.DrawDiag();
    #endif
}
