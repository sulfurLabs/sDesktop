#!/bin/sh
#
# NOTE:
# do NOT run if you have NOT installed sDesktop

set -eu

BIN_DIR="/usr/local/bin"
INITTAB="/etc/inittab"

if [ "$(id -u)" = "0" ]; then
    printf "enter the username for \n"
    printf "where to install: "
    read -r INSTALL_USER
else
    INSTALL_USER="$(id -un)"
fi

if [ -z "$INSTALL_USER" ] || [ "$INSTALL_USER" = "root" ]; then
    echo ":: error: could NOT find the normal user"
    echo "          use sudo to run this script or specify a user"
    exit 1
fi

INSTALL_HOME="$(awk -F: -v user="$INSTALL_USER" '$1 == user { print $6 }' /etc/passwd)"

if [ -z "$INSTALL_HOME" ] || [ ! -d "$INSTALL_HOME" ]; then
    echo ":: error: could NOT find home directory for $INSTALL_USER"
    exit 1
fi

CONFIG_DIR="$INSTALL_HOME/.config/desktop"

if [ "$(id -u)" != "0" ]; then
    echo "you need to run this uninstaller as root/ using sudo"
    exit 1
fi

echo "=> uninstalling sDesktop..."
echo "=> removing desktop files..."

if [ -f "$BIN_DIR/sDesktop-linux" ]; then
    rm -f "$BIN_DIR/sDesktop-linux"
fi
if [ -f "$BIN_DIR/admenu" ]; then
    rm -f "$BIN_DIR/admenu"
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
    echo ":: removing ADmenu entry manually..."

    if [ -f "$INITTAB" ]; then
        sed -i '\|^tty1::respawn:/usr/local/bin/admenu$|d' "$INITTAB"
    fi
fi

echo ""
echo ":: sDesktop successfully uninstalled"
echo "   user:      $INSTALL_USER"
echo "   binary:    removed"
echo "   resources: removed"
echo "   menu:      removed"
echo "   inittab:   restored"
echo ""
echo " > you can reboot now"
