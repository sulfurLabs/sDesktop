#!/usr/bin/env bash
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (c) 2026 sulfurLabs
#
# PROJECT: sDesktop
# FILE: buildimage.sh
#

set -euo pipefail

SRC="./include/resources"
OUT="./build/sDesktop.cpio"

CPIO="cpio"
if [ "$(uname -s)" = "Darwin" ] && command -v gcpio >/dev/null 2>&1; then
	CPIO="gcpio"
fi

if [ ! -d "$SRC" ]; then
    echo "error: $SRC not found"
    exit 1
fi


(
	cd "$SRC"

	find . \
		\( -name ".DS_Store" \
		-o -name "._*" \
		-o -name ".Spotlight-V100" \
		-o -name ".Trashes" \
		-o -name ".fseventsd" \
		-o -name ".TemporaryItems" \
		-o -name ".VolumeIcon.icns" \
		-o -name ".AppleDouble" \) -prune -o -print \
	| sort \
	| "$CPIO" -o -H newc -R 0:0
) > "$OUT"
