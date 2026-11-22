#!/bin/sh
#
# NOTE:
# do NOT run if you have NOT installed sDesktop

set -eu

CONFIG_DIR="$HOME/.config/desktop"
BIN_DIR="/usr/local/bin"
INITTAB="/etc/inittab"

if [ "$(id -u)" != "0" ]; then
    echo "you need to run this uninstaller as root/ using sudo"
    exit 1
fi

echo "=> uninstalling sDesktop..."
echo "=> removing desktop files..."

if [ -f "$BIN_DIR/sDesktop-linux" ]; then
    rm -f "$BIN_DIR/sDesktop-linux"
fi
if [ -f "$BIN_DIR/desktop-menu" ]; then
    rm -f "$BIN_DIR/desktop-menu"
fi

echo "=> removing resources..."

if [ -d "$CONFIG_DIR" ]; then
    rm -rf "$CONFIG_DIR"
fi

echo "=> restoring \"inittab\"..."

if [ -f "$INITTAB.bak" ]; then
    cp "$INITTAB.bak" "$INITTAB"
    rm -f "$INITTAB.bak"
else
    echo ":: warning: inittab backup wasn't found"
    echo ":: removing desktop-menu entry manually..."

    if [ -f "$INITTAB" ]; then
        sed -i '\|^tty1::respawn:/usr/local/bin/desktop-menu$|d' "$INITTAB"
    fi
fi

echo ""
echo ":: sDesktop successfully uninstalled"
echo "   binary:    removed"
echo "   resources: removed"
echo "   menu:      removed"
echo "   inittab:   restored"
echo ""
echo " > you can reboot now"
