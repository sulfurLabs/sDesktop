#!/bin/sh
#
# NOTE:
# this is the AvoryOS install, and will not work on every
# linux distribution, do NOT run without making sure
# the marked directories/files are the same on your system
# [!CHECK!]

set -eu

if [ "$(id -u)" = "0" ]; then
    printf "enter a username for\n"
    printf "where to uninstall: "
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
RELEASE="https://github.com/sulfurLabs/sDesktop/releases/download/v0.1"
BIN_DIR="/usr/local/bin" # !CHECK!
INITTAB="/etc/inittab" # !CHECK!
TMP_DIR="/tmp/sdesktop-install-$$"

if [ "$(id -u)" != "0" ]; then
    echo "you need to run this installer as root/ using sudo"
    exit 1
fi

download() {
     # !CHECK!
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL -o "$2" "$1"
    elif command -v wget >/dev/null 2>&1; then
        wget -q -O "$2" "$1"
    elif command -v busybox >/dev/null 2>&1; then
        busybox wget -q -O "$2" "$1"
    else
        echo ":: error: curl or wget is required, please install it"
        exit 1
    fi
}

extract() {
    if command -v unzip >/dev/null 2>&1; then
        unzip -o "$1" -d "$2"
    elif command -v busybox >/dev/null 2>&1; then
        busybox unzip -o "$1" -d "$2"
    else
        echo ":: error: unzip is required, please install it"
        exit 1
    fi
}

cleanup() {
    rm -rf "$TMP_DIR"
}

trap cleanup EXIT
trap 'exit 1' HUP INT TERM

echo "=> installing sDesktop..."

#TODO:
# check if sDesktop is already installed
# if yes, update it and print a message
mkdir -p "$TMP_DIR"
mkdir -p "$CONFIG_DIR"
mkdir -p "$BIN_DIR"

#only if not installed yet
echo "=> downloading \"sDesktop-linux\"..."
download "$RELEASE/sDesktop-linux" "$TMP_DIR/sDesktop-linux"

echo "=> downloading \"ADmenu\"..."
download "$RELEASE/admenu" "$TMP_DIR/admenu"

echo "=> downloading \"resources.zip\"..."
download "$RELEASE/resources.zip" "$TMP_DIR/resources.zip"

echo "=> installing desktop files..." # or updating
cp "$TMP_DIR/sDesktop-linux" "$BIN_DIR/sDesktop-linux"
chmod +x "$BIN_DIR/sDesktop-linux"

echo "=> extracting \"resources\"..."
extract "$TMP_DIR/resources.zip" "$CONFIG_DIR"
chown -R "$INSTALL_USER" "$CONFIG_DIR"

echo "=> installing \"ADmenu\"..." # or updating
cp "$TMP_DIR/admenu" "$BIN_DIR/admenu"
chmod +x "$BIN_DIR/admenu"

echo "=> updating \"inittab\"..."

if [ -f "$INITTAB" ]; then
    cp "$INITTAB" "$INITTAB.bak"

    if grep -q '^tty1::respawn:' "$INITTAB"; then
        sed -i 's|^tty1::respawn:.*|tty1::respawn:/usr/local/bin/admenu|' "$INITTAB"
    else
        printf '\ntty1::respawn:/usr/local/bin/admenu\n' >> "$INITTAB"
    fi
else
    echo ":: error: /etc/inittab wasn't found;"
    echo "          may happend because your system is Older/Newer OR,"
    echo "          is A AvoryOS Distribution OR,"
    echo "          is NOT AvoryOS"
    exit 1
fi

echo ""
echo ":: sDesktop successfully installed :D"
echo "   user:      $INSTALL_USER"
echo "   binary:    $BIN_DIR/sDesktop-linux"
echo "   resources: $CONFIG_DIR/"
echo "   menu:      $BIN_DIR/admenu"
echo "   backup:    $INITTAB.bak"
echo ""
echo " > you can reboot, \"ADmenu\" should automatically start"
