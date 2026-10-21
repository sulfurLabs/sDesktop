//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (c) 2026 sulfurLabs
//
// PROJECT: sDesktop
// FILE: cursor.h
//

#ifndef CURSOR_H
#define CURSOR_H 1

#define T 0x00000000u
#define B 0xFF101010u
#define W 0xFFFFFFFFu
#define X 0xFF808080u  // gray for resize arrows

static const unsigned int NormalCursor[12 * 16] =
{
    B,T,T,T,T,T,T,T,T,T,T,T,
    B,B,T,T,T,T,T,T,T,T,T,T,
    B,W,B,T,T,T,T,T,T,T,T,T,
    B,W,W,B,T,T,T,T,T,T,T,T,
    B,W,W,W,B,T,T,T,T,T,T,T,
    B,W,W,W,W,B,T,T,T,T,T,T,
    B,W,W,W,W,W,B,T,T,T,T,T,
    B,W,W,W,W,W,W,B,T,T,T,T,
    B,W,W,W,W,W,W,W,B,T,T,T,
    B,W,W,W,W,W,W,W,W,B,T,T,
    B,W,W,W,W,W,W,W,W,W,B,T,
    B,W,W,W,W,B,B,B,B,B,B,T,
    B,W,W,W,B,T,T,T,T,T,T,T,
    B,W,W,B,T,T,T,T,T,T,T,T,
    B,W,B,T,T,T,T,T,T,T,T,T,
    B,B,T,T,T,T,T,T,T,T,T,T,
};

static const unsigned int HresizeCursor[16 * 16] =
{
	T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
	T,T,T,T,T,T,B,T,T,B,T,T,T,T,T,T,
	T,T,T,T,T,B,B,T,T,B,B,T,T,T,T,T,
	T,T,T,T,B,W,B,T,T,B,W,B,T,T,T,T,
	T,T,T,B,W,W,B,T,T,B,W,W,B,T,T,T,
	T,T,B,W,W,W,B,T,T,B,W,W,W,B,T,T,
	T,B,W,W,W,W,B,B,B,B,W,W,W,W,B,T,
	B,W,W,W,W,W,W,W,W,W,W,W,W,W,W,B,
	B,W,W,W,W,W,W,W,W,W,W,W,W,W,W,B,
	T,B,W,W,W,W,B,B,B,B,W,W,W,W,B,T,
	T,T,B,W,W,W,B,T,T,B,W,W,W,B,T,T,
	T,T,T,B,W,W,B,T,T,B,W,W,B,T,T,T,
	T,T,T,T,B,W,B,T,T,B,W,B,T,T,T,T,
	T,T,T,T,T,B,B,T,T,B,B,T,T,T,T,T,
	T,T,T,T,T,T,B,T,T,B,T,T,T,T,T,T,
	T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

static const unsigned int VresizeCursor[16 * 16] =
{
	T,T,T,T,T,T,T,B,B,T,T,T,T,T,T,T,
	T,T,T,T,T,T,B,W,W,B,T,T,T,T,T,T,
	T,T,T,T,T,B,W,W,W,W,B,T,T,T,T,T,
	T,T,T,T,B,W,W,W,W,W,W,B,T,T,T,T,
	T,T,T,B,W,W,W,W,W,W,W,W,B,T,T,T,
	T,T,B,W,W,W,W,W,W,W,W,W,W,B,T,T,
	T,B,B,B,B,B,B,W,W,B,B,B,B,B,B,T,
	T,T,T,T,T,T,B,W,W,B,T,T,T,T,T,T,
	T,T,T,T,T,T,B,W,W,B,T,T,T,T,T,T,
	T,B,B,B,B,B,B,W,W,B,B,B,B,B,B,T,
	T,T,B,W,W,W,W,W,W,W,W,W,W,B,T,T,
	T,T,T,B,W,W,W,W,W,W,W,W,B,T,T,T,
	T,T,T,T,B,W,W,W,W,W,W,B,T,T,T,T,
	T,T,T,T,T,B,W,W,W,W,B,T,T,T,T,T,
	T,T,T,T,T,T,B,W,W,B,T,T,T,T,T,T,
	T,T,T,T,T,T,T,B,B,T,T,T,T,T,T,T,
};

static const unsigned int TLBRresizeCursor[16 * 16] =
{
	T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
	T,B,B,B,B,B,B,B,B,B,B,T,T,T,T,T,
	T,B,W,W,W,W,W,W,W,W,B,T,T,T,T,T,
	T,B,W,W,W,W,W,W,W,B,T,T,T,T,T,T,
	T,B,W,W,W,W,W,W,B,T,T,T,T,T,T,T,
	T,B,W,W,W,W,W,B,T,T,T,T,T,B,B,T,
	T,B,W,W,W,W,W,W,B,T,T,T,B,W,B,T,
	T,B,W,W,W,B,W,W,W,B,T,B,W,W,B,T,
	T,B,W,W,B,T,B,W,W,W,B,W,W,W,B,T,
	T,B,W,B,T,T,T,B,W,W,W,W,W,W,B,T,
	T,B,B,T,T,T,T,T,B,W,W,W,W,W,B,T,
	T,T,T,T,T,T,T,B,W,W,W,W,W,W,B,T,
	T,T,T,T,T,T,B,W,W,W,W,W,W,W,B,T,
	T,T,T,T,T,B,W,W,W,W,W,W,W,W,B,T,
	T,T,T,T,T,B,B,B,B,B,B,B,B,B,B,T,
	T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,

};

static const unsigned int TRBLresizeCursor[16 * 16] =
{
	T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
	T,T,T,T,T,B,B,B,B,B,B,B,B,B,B,T,
	T,T,T,T,T,B,W,W,W,W,W,W,W,W,B,T,
	T,T,T,T,T,T,B,W,W,W,W,W,W,W,B,T,
	T,T,T,T,T,T,T,B,W,W,W,W,W,W,B,T,
	T,B,B,T,T,T,T,T,B,W,W,W,W,W,B,T,
	T,B,W,B,T,T,T,B,W,W,W,W,W,W,B,T,
	T,B,W,W,B,T,B,W,W,W,B,W,W,W,B,T,
	T,B,W,W,W,B,W,W,W,B,T,B,W,W,B,T,
	T,B,W,W,W,W,W,W,B,T,T,T,B,W,B,T,
	T,B,W,W,W,W,W,B,T,T,T,T,T,B,B,T,
	T,B,W,W,W,W,W,W,B,T,T,T,T,T,T,T,
	T,B,W,W,W,W,W,W,W,B,T,T,T,T,T,T,
	T,B,W,W,W,W,W,W,W,W,B,T,T,T,T,T,
	T,B,B,B,B,B,B,B,B,B,B,T,T,T,T,T,
	T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

#undef T
#undef B
#undef W
#undef X

// cursor sizes
#define CUR_NORMAL_W  12
#define CUR_NORMAL_H  16
#define CUR_RESIZE_W  16
#define CUR_RESIZE_H  16

//cursor types so we know which texture to draw
#define CUR_NORMAL      0
#define CUR_HRESIZE     1
#define CUR_VRESIZE     2
#define CUR_TLBRRESIZE  3
#define CUR_TRBLRESIZE  4

#endif
