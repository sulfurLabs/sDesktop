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
#include <string.h>
#include <pui.h>

#include "pui.h"
#include "cfg.h"
#include "log.h"

//pyritUI itself should never know what a desktop is, or a window
// programs like the Desktop need to call a generic pyrit api

static char PyritUIDesktopValue[3][24];
static char PyritUIPath[256];

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

void PyritUIDesktopAttach(compositor_t *comp)
{
    static pui_Target screen;

    screen = (pui_Target)
    {
        comp->fb.back,
        0,
        0,
        CompWidth(comp),
        CompHeight(comp)
    };

    pyrit.Context("Desktop", &screen);
    pyrit.RegisterProperty("Desktop.width", PyritUIDesktopWidth, comp);
    pyrit.RegisterProperty("Desktop.height", PyritUIDesktopHeight, comp);
    pyrit.RegisterProperty("Desktop.pixels", PyritUIDesktopPixels, comp);

    /*
    pyrit.Context("Window", &windowTarget);
    pyrit.RegisterProperty("Window.title", PyritUIWindowTitle, win);
    pyrit.RegisterProperty("Window.width", PyritUIWindowWidth, win);
    pyrit.RegisterProperty("Window.height", PyritUIWindowHeight, win);
    pyrit.RegisterProperty("Window.z", PyritUIWindowZ, win);*/
    //Windows dont have a context, every window gets a instance
    // and every one has a own target

    //pyrit.RegisterFunction("SetContext", pyrit.SetContext, NULL);
    //i want to create windows per pui so i can make buttons like start button, but i also need a popup version then
    // so a plan would be to just make a Window and give it properties like SD_POPUP or SD_WIN
    //
    // or we just register, CreateWindow and CreatePopup.
    //
    // or we add the logic for PyritUI to register function names themself, and save those names as a function which can be called in
    // PyritUI itself and we just tell the parser to e.g set properties like z-order or coordinates, etc
    //
    // havent decided what to choose yet, so this will stay a stupid comment.
}

void PyritUILoad(const char *path)
{
    if (!pyrit.Init()) return;

    //linux has env but sulfurOS not so
    // sulfd config path is used
    if (!path)
    {
        const char *home = getenv("HOME");

        if (
            home &&
            snprintf(PyritUIPath, sizeof(PyritUIPath), "%s/.config/desktop/desktop.pui", home) > 0
        ){
            path = PyritUIPath;
        }
        if (!access(PyritUIPath, F_OK))
        {
            path = PyritUIPath;
        } else if (!access("/root/.config/desktop/desktop.pui", F_OK))
        {
            path = "/root/.config/desktop/desktop.pui";
        } else
            Log(":: error: could not find desktop.pui in $HOME or /root");
        }
    }

    #ifdef __sulfur__
        if (!path) path = PUI_DM_CONFIG;
    #else
        //sulfurOS name is always pc for now
        if (!path) path = "/users/pc/.config/desktop/desktop.pui";
    #endif

    if (!pyrit.LoadFile(path))
    {
        Log("could not load the pui config, coded ui stays");
    }

    #if DEBUGOUT
        //shows what the config pushed, catches stale configs fast
        pyrit.DrawDiag();
    #endif
}
