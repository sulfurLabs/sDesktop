//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: os.h
//

#pragma once

//oss/OSS/Oss (operating system specific)
//
#ifdef __sulfur__
    #include <sys/fb.h>
    #include <sys/input.h>
    #include <fcntl.h>
    #include <unistd.h>

    #define GREETER "sulfurOS" //the greeting message

    #define FB_DEV "/dev/fb0"
    #define MS_DEV "/dev/mouse"
    #define KB_DEV "/dev/kbd0"

#elif defined(__linux__)
    //linux specific code
    #include <linux/fb.h>
    #include <fcntl.h>
    #include <sys/mount.h>
    #include <sys/stat.h>
    #include <unistd.h>

    #define GREETER "Linux"

    #define FB_DEV "/dev/fb0"
    #define MS_DEV "/dev/input/mice"
    #define KB_DEV ""
/*
    static inline int OssOpenKeyboard(void)
    {
        //input is uhh.. a bit different on linux... ._.
        return 0;
    }
*/
#endif
