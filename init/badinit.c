//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: badinit.c
//

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef SYS_init_module
#   define SYS_init_module 175
#endif

static void put(const char *s)
{
    write(1, s, strlen(s)); //had some problems with printf and couldnt find out why
}

//order matters cuz some need others first
// tbh i just tried until it stopped complaining
// so if anyone finds more then yea order matters._.
static const char *mods[] =
{
    "/lib/input/psmouse.ko",
    "/lib/input/i2c-mux.ko",
    "/lib/input/i2c-smbus.ko",
    "/lib/input/i2c-i801.ko", //should be for older laptops like i have
    "/lib/input/elan_i2c.ko", //elan pads for ASUS and smth else too iirc
    "/lib/input/i2c-hid.ko",
    "/lib/input/i2c-hid-acpi.ko", // modern pads talk hid over i2c or smth
    "/lib/input/hid-multitouch.ko", //multitouch like my lenovo laptop has
    "/lib/input/atkbd.ko", //ps2 keyboards like qemu has
    "/lib/input/usbhid.ko",
    "/lib/input/mousedev.ko", //ps2 fallback needs this
    0
};

static void insmod(const char *path)
{
    struct stat st;
    int fd = open(path, O_RDONLY);
    if (fd < 0) return;

    //if a file is too big
    //
    #define MAXFS 8 * 1024 * 1024
    if (fstat(fd, &st) < 0 || st.st_size <= 0 || st.st_size > MAXFS)
    {
        close(fd);
        return;
    }

    char *img = malloc(st.st_size);
    if (!img)
    {
        close(fd);
        return;
    }

    long off = 0;
    while (off < st.st_size)
    {
        long n = read(fd, img + off, st.st_size - off);
        if (n <= 0) break;
        off += n;
    }

    close(fd);

    if (off != st.st_size)
    {
        free(img);
        return; // if we dont have all of them we skip
    }

    put("init: loading ");
    put(path);
    if
        (
            //empty strings for no options also idk which options i would need
            syscall(SYS_init_module, img, st.st_size, ""
        ) != 0
    ){
        put(" failed\n");
    }
    else
    {
        put(" ok\n");
    }

    free(img);
}

int main(void)
{
    put(":: sDesktop for linux badinit\n");

    for (int i = 0; mods[i]; i++) insmod(mods[i]);

    // the desktop handles all now
    char *argv[] =
    {
        "/sDesktop",
        0
    };

    for (;;)
    {
        pid_t pid = fork();

        if (pid == 0)
        {
            execv("/sDesktop", argv);

            put("init: exec failed \n");
            _exit(1);
        }

        if (pid < 0)
        {
            put("init: fork failed \n");
            sleep(5);
            continue;
        }

        int status = 0;
        waitpid(pid, &status, 0);
        put("init: desktop is gone, restarting\n");
        sleep(1);
    }

    return 0;
}
