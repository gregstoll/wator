/* -*- Mode: C; tab-width: 4 -*- */
/* wator --- Dewdney's Wa-Tor, water torus simulation */

/*-
 * Copyright (c) 1994 by David Bagley.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 * Revision History:
 * 18-Jan-2007: Added vertical option.
 * 01-Nov-2000: Allocation checks
 * 10-May-1997: Compatible with xscreensaver
 * 29-Aug-1995: Efficiency improvements.
 * 12-Dec-1994: Coded from A.K. Dewdney's "The Armchair Universe, Computer
 *              Recreations from the Pages of Scientific American Magazine"
 *              W.H. Freedman and Company, New York, 1988  (Dec 1984 and
 *              June 1985) also used life.c as a guide.
 */


#define MODE_wator
#define DEFAULTS "*delay: 750000 \n" \
	"*cycles: 32767 \n" \
	"*size: 0 \n" \
	"*ncolors: 200 \n" \

# define free_wator 0
# define reshape_wator 0
# define wator_handle_event 0

#define ENTRYPOINT

#include <emscripten.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <random>
#include "automata.h"

std::random_device rd; // Seed generator
std::mt19937 gen(rd()); // Mersenne Twister engine

int NRAND(int n) {
	std::uniform_int_distribution dist(0, n - 1);
	return dist(gen);
}
int LRAND() {
	return NRAND(2);
}

#define MIN(a, b) (std::min(a, b))
#define MAX(a, b) (std::max(a, b))

#ifdef MODE_wator

/*-
 * neighbors of 0 randomizes it between 3, 4, 6, 8, 9, and 12.
 */
#define DEF_NEIGHBORS  "0"      /* choose random value */
#define DEF_VERTICAL "False"

static int  neighbors;
static bool vertical;


#ifdef USE_MODULES
static XrmOptionDescRec opts[] =
{
	{(char *) "-neighbors", (char *) ".wator.neighbors", XrmoptionSepArg, (caddr_t) NULL},
	{(char *) "-vertical", (char *) ".wator.vertical", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+vertical", (char *) ".wator.vertical", XrmoptionNoArg, (caddr_t) "off"}
};

static argtype vars[] =
{
	{(void *) & neighbors, (char *) "neighbors", (char *) "Neighbors", (char *) DEF_NEIGHBORS, t_Int},
	{(void *) & vertical, (char *) "vertical", (char *) "Vertical", (char *) DEF_VERTICAL, t_Bool}
};

static OptionStruct desc[] =
{
	{(char *) "-neighbors num", (char *) "squares 4 or 8, hexagons 6, triangles 3, 9 or 12"},
	{(char *) "-/+vertical", (char *) "change orientation for hexagons and triangles"}
};


ENTRYPOINT ModeSpecOpt wator_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};
ModStruct   wator_description =
{"wator", "init_wator", "draw_wator", "release_wator",
 "refresh_wator", "init_wator", (char *) NULL, &wator_opts,
 750000, 1, 32767, 0, 64, 1.0, "",
 "Shows Dewdney's Water-Torus planet of fish and sharks", 0, NULL};

#endif

/*
#include "bitmaps/fish-0.xbm"
#include "bitmaps/fish-1.xbm"
#include "bitmaps/fish-2.xbm"
#include "bitmaps/fish-3.xbm"
#include "bitmaps/fish-4.xbm"
#include "bitmaps/fish-5.xbm"
#include "bitmaps/fish-6.xbm"
#include "bitmaps/fish-7.xbm"
#include "bitmaps/shark-0.xbm"
#include "bitmaps/shark-1.xbm"
#include "bitmaps/shark-2.xbm"
#include "bitmaps/shark-3.xbm"
#include "bitmaps/shark-4.xbm"
#include "bitmaps/shark-5.xbm"
#include "bitmaps/shark-6.xbm"
#include "bitmaps/shark-7.xbm"
*/

#define FISH 0
#define SHARK 1
#define KINDS 2
#define ORIENTS 4
#define REFLECTS 2
#define BITMAPS (ORIENTS*REFLECTS*KINDS)
#define KINDBITMAPS (ORIENTS*REFLECTS)
#define MINGRIDSIZE 10		/* It is possible for the fish to take over with 3 */
#define MINSIZE 4
#define NEIGHBORKINDS 6

/*
static XImage logo[BITMAPS] =
{
	{0, 0, 0, XYBitmap, (char *) fish0_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) fish1_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) fish2_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) fish3_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) fish4_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) fish5_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) fish6_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) fish7_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) shark0_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) shark1_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) shark2_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) shark3_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) shark4_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) shark5_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) shark6_bits, LSBFirst, 8, LSBFirst, 8, 1},
	{0, 0, 0, XYBitmap, (char *) shark7_bits, LSBFirst, 8, LSBFirst, 8, 1},
};
*/

/* Fish and shark data */
typedef struct {
	char        kind, age, food, direction;
	unsigned long color;
	int         col, row;
} cellstruct;

/* Doubly linked list */
typedef struct _CellList {
	cellstruct  info;
	struct _CellList *previous, *next;
} CellList;

typedef struct {
	bool        painted, vertical;
	int         nkind[KINDS];	/* Number of fish and sharks */
	int         breed[KINDS];	/* Breeding time of fish and sharks */
	int         sstarve;	/* Time the sharks starve if they don't find a fish */
	int         kind;	/* Currently working on fish or sharks? */
	int         xs, ys;	/* Size of fish and sharks */
	int         xb, yb;	/* Bitmap offset for fish and sharks */
	int         pixelmode;
	int         generation;
	int         ncols, nrows, positions;
	int         width, height;
	CellList   *currkind, *babykind, *lastkind[KINDS + 1], *firstkind[KINDS + 1];
	CellList  **arr;	/* 0=empty or pts to a fish or shark */
	int         neighbors, polygon;
	union {
		XPoint      hexagon[6];
		XPoint      triangle[2][3];
	} shape;
} watorstruct;

static char plots[NEIGHBORKINDS] =
{
	3, 4, 6, 8, 9, 12	/* Neighborhoods */
};

static watorstruct wators;
//static int  icon_width, icon_height;

#if 0
/*-
 * shape either a bitmap or 0 for circle and 1 for polygon
 * (if triangle shape:  -1, 0, 2 or 3 to differentiate left and right)
 */
drawshape(ModeInfo * mi, int x, int y, int sizex, int sizey,
	  int sides, int shape)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	GC          gc = MI_GC(mi);

	if (sides == 4 && sizex == 0 && sizey == 0) {
		(void) XPutImage(display, window, gc, &logo[shape], 0, 0,
		x - icon_width, y - icon_height / 2, icon_width, icon_height);
	} else if (sizex < 3 || sizey < 3 || (sides == 4 && shape == 1)) {
		XFillRectangle(display, window, gc,
			x - sizex / 2, y - sizey / 2,
			sizex - (sizey > 3), sizey - (sizey > 3));
	} else if (sides == 4 && shape == 0) {
	} else if (sides == 6 && shape == 1) {
	} else if (sides == 6 && shape == 0) {
	} else if (sides == 3 && shape == 1) {
	} else if (sides == 3 && shape == 2) {
	} else if (sides == 3 && shape == -1) {
	} else if (sides == 3 && shape == 0) {
	}
}
#endif

typedef struct CellDataForJS {
	int8_t alive;
	int8_t color;
	int8_t bitmap;
} CellDataForJS;

CellDataForJS* cellDataForJS = nullptr;

extern "C"
EMSCRIPTEN_KEEPALIVE int getCellData() {
	return (int)cellDataForJS;
}

static void
drawcell(int col, int row, unsigned long color, int bitmap,
		bool alive)
{
	/*Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	GC          gc = MI_GC(mi);*/
	watorstruct *wp = &wators;
	unsigned long colour;

	int index = row * (wators.width / wators.xs) + col;
	cellDataForJS[index].alive = alive ? 1 : 0;
	cellDataForJS[index].color = color;
	cellDataForJS[index].bitmap = bitmap;

	// BIG TODO here
	// if (!alive)
	// 	colour = MI_BLACK_PIXEL(mi);
	// else if (MI_NPIXELS(mi) > 2)
	// 	colour = MI_PIXEL(mi, color);
	// else
	// 	colour = MI_WHITE_PIXEL(mi);
	// XSetForeground(display, gc, colour);
	// if (wp->neighbors == 6) {
	// 	int ccol = 2 * col + !(row & 1), crow = 2 * row;

	// 	if (wp->vertical) {
	// 		wp->shape.hexagon[0].x = wp->xb + ccol * wp->xs;
	// 		wp->shape.hexagon[0].y = wp->yb + crow * wp->ys;
	// 	} else {
	// 		wp->shape.hexagon[0].y = wp->xb + ccol * wp->xs;
	// 		wp->shape.hexagon[0].x = wp->yb + crow * wp->ys;
	// 	}
	// 	if (wp->xs == 1 && wp->ys == 1)
	// 		XDrawPoint(display, window, gc,
	// 			wp->shape.hexagon[0].x,
	// 			wp->shape.hexagon[0].y);
	// 	else if (bitmap >= KINDBITMAPS || !alive)
	// 		XFillPolygon(display, window, gc,
	// 			wp->shape.hexagon, 6,
	// 			Convex, CoordModePrevious);
	// 	else {
	// 		int ix = 0, iy = 0, sx, sy;

	// 		XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
	// 		XFillPolygon(display, window, gc,
	// 			wp->shape.hexagon, 6,
	// 			Convex, CoordModePrevious);
	// 		XSetForeground(display, gc, colour);
	// 		if (wp->vertical) {
	// 			wp->shape.hexagon[0].x -= wp->xs;
	// 			wp->shape.hexagon[0].y += wp->ys / 4;
    //                             sx = 2 * wp->xs - 6;
    //                             sy = 2 * wp->ys - 2;
	// 			if (wp->xs <= 6 || wp->ys <= 2) {
	// 				ix = 3;
	// 				iy = 1;
	// 			} else
	// 				ix = 5;
	// 		} else {
	// 			wp->shape.hexagon[0].y -= wp->xs;
	// 			wp->shape.hexagon[0].x += wp->ys / 4;
    //                             sy = 2 * wp->xs - 6;
    //                             sx = 2 * wp->ys - 2;
	// 			if (wp->xs <= 6 || wp->ys <= 2) {
	// 				iy = 3;
	// 				ix = 1;
	// 			} else
	// 				iy = 5;
	// 		}
	// 		if (wp->xs <= 6 || wp->ys <= 2)
	// 			XFillRectangle(display, window, gc,
	// 				wp->shape.hexagon[0].x + ix,
	// 				wp->shape.hexagon[0].y + iy,
	// 				wp->xs, wp->ys);
	// 		else
	// 			XFillArc(display, window, gc,
	// 				wp->shape.hexagon[0].x + ix, 
	// 				wp->shape.hexagon[0].y + iy,
	// 				sx, sy,
	// 				0, 23040);
	// 	}
	// } else if (wp->neighbors == 4 || wp->neighbors == 8) {
	// 	if (wp->pixelmode) {
	// 		if (bitmap >= KINDBITMAPS || (wp->xs <= 2 || wp->ys <= 2) || !alive)
	// 			XFillRectangle(display, window, gc,
	// 				wp->xb + wp->xs * col,
	// 				wp->yb + wp->ys * row,
	// 				wp->xs - (wp->xs > 3),
	// 				wp->ys - (wp->ys > 3));
	// 		else {
	// 			XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
	// 			XFillRectangle(display, window, gc,
	// 				wp->xb + wp->xs * col,
	// 				wp->yb + wp->ys * row,
	// 				wp->xs, wp->ys);
	// 			XSetForeground(display, gc, colour);
	// 			XFillArc(display, window, gc,
	// 				wp->xb + wp->xs * col,
	// 				wp->yb + wp->ys * row,
	// 				wp->xs - 1, wp->ys - 1,
	// 				0, 23040);
	// 		}
	// 	} else
	// 		(void) XPutImage(display, window, gc,
	// 			&logo[bitmap], 0, 0,
	// 			wp->xb + wp->xs * col, wp->yb + wp->ys * row,
	// 			icon_width, icon_height);
	// } else { /* TRI */
	// 	int orient = (col + row) % 2;	/* O left 1 right */
	// 	bool small = (wp->xs <= 3 || wp->ys <= 3);
	// 	int ix = 0, iy = 0;

	// 	if (wp->vertical) {
	// 		wp->shape.triangle[orient][0].x = wp->xb + col * wp->xs;
	// 		wp->shape.triangle[orient][0].y = wp->yb + row * wp->ys;
	// 		if (small)
	// 			wp->shape.triangle[orient][0].x +=
	// 				((orient) ? -1 : 1);
	// 		else
	// 			wp->shape.triangle[orient][0].x +=
	// 				(wp->xs / 2  - 1) * ((orient) ? 1 : -1);
	// 	} else {
	// 		wp->shape.triangle[orient][0].y = wp->xb + col * wp->xs;
	// 		wp->shape.triangle[orient][0].x = wp->yb + row * wp->ys;
	// 		if (small)
	// 			wp->shape.triangle[orient][0].y +=
	// 				((orient) ? -1 : 1);
	// 		else
	// 			wp->shape.triangle[orient][0].y +=
	// 				(wp->xs / 2  - 1) * ((orient) ? 1 : -1);
	// 	}
	// 	if (small)
	// 		XDrawPoint(display, window, gc,
	// 			wp->shape.triangle[orient][0].x,
	// 			wp->shape.triangle[orient][0].y);
	// 	else {
	// 		if (bitmap >= KINDBITMAPS || !alive)
	// 			XFillPolygon(display, window, gc,
	// 				wp->shape.triangle[orient], 3,
	// 				Convex, CoordModePrevious);
	// 		else {
	// 			XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
	// 			XFillPolygon(display, window, gc,
	// 				wp->shape.triangle[orient], 3,
	// 				Convex, CoordModePrevious);
	// 			XSetForeground(display, gc, colour);
	// 			if (wp->vertical) {
	// 				wp->shape.triangle[orient][0].x += -4 * wp->xs / 5 +
    //                                     	((orient) ? wp->xs / 3 : 3 * wp->xs / 5);
    //                                     wp->shape.triangle[orient][0].y += -wp->ys / 2 + 1;
	// 				ix = ((orient) ? -wp->xs / 2 : wp->xs / 2);
	// 			} else {
	// 				wp->shape.triangle[orient][0].y += -4 * wp->xs / 5 +
    //                                     	((orient) ? wp->xs / 3 : 3 * wp->xs / 5);
    //                                     wp->shape.triangle[orient][0].x += -wp->ys / 2 + 1;
	// 				iy = ((orient) ? -wp->xs / 2 : wp->xs / 2);
	// 			}
	// 			XFillArc(display, window, gc,
	// 				wp->shape.triangle[orient][0].x + ix,
	// 				wp->shape.triangle[orient][0].y + iy,
	// 				wp->ys - 3, wp->ys - 3,
	// 				0, 23040);
	// 		}
	// 	}
	// }
}

static bool
init_kindlist(watorstruct * wp, int kind)
{
	/* Waste some space at the beginning and end of list
	   so we do not have to complicated checks against falling off the ends. */
	if (((wp->lastkind[kind] = (CellList *) malloc(sizeof (CellList))) ==
			nullptr) ||
	    ((wp->firstkind[kind] = (CellList *) malloc(sizeof (CellList))) ==
			 nullptr)) {
		return false;
	}
	wp->firstkind[kind]->previous = wp->lastkind[kind]->next =
		(struct _CellList *) nullptr;
	wp->firstkind[kind]->next = wp->lastkind[kind]->previous =
		(struct _CellList *) nullptr;
	wp->firstkind[kind]->next = wp->lastkind[kind];
	wp->lastkind[kind]->previous = wp->firstkind[kind];
	return true;
}

static bool
addto_kindlist(watorstruct * wp, int kind, cellstruct info)
{
	if ((wp->currkind = (CellList *) malloc(sizeof (CellList))) == nullptr) {
		return false;
	}
	wp->lastkind[kind]->previous->next = wp->currkind;
	wp->currkind->previous = wp->lastkind[kind]->previous;
	wp->currkind->next = wp->lastkind[kind];
	wp->lastkind[kind]->previous = wp->currkind;
	wp->currkind->info = info;
	return true;
}

static void
removefrom_kindlist(watorstruct * wp, CellList * ptr)
{
	ptr->previous->next = ptr->next;
	ptr->next->previous = ptr->previous;
	wp->arr[ptr->info.col + ptr->info.row * wp->ncols] = 0;
	free(ptr);
}

static bool
dupin_kindlist(watorstruct * wp)
{
	CellList   *temp;

	if ((temp = (CellList *) malloc(sizeof (CellList))) == NULL) {
		return false;
	}
	temp->previous = wp->babykind;
	temp->next = wp->babykind->next;
	wp->babykind->next = temp;
	temp->next->previous = temp;
	temp->info = wp->babykind->info;
	wp->babykind = temp;
	return true;
}

/*-
 * new fish at end of list, this rotates who goes first, young fish go last
 * this most likely will not change the feel to any real degree
 */
static void
cutfrom_kindlist(watorstruct * wp)
{
	wp->babykind = wp->currkind;
	wp->currkind = wp->currkind->previous;
	wp->currkind->next = wp->babykind->next;
	wp->babykind->next->previous = wp->currkind;
	wp->babykind->next = wp->lastkind[KINDS];
	wp->babykind->previous = wp->lastkind[KINDS]->previous;
	wp->babykind->previous->next = wp->babykind;
	wp->babykind->next->previous = wp->babykind;
}

static void
reattach_kindlist(watorstruct * wp, int kind)
{
	wp->currkind = wp->lastkind[kind]->previous;
	wp->currkind->next = wp->firstkind[KINDS]->next;
	wp->currkind->next->previous = wp->currkind;
	wp->lastkind[kind]->previous = wp->lastkind[KINDS]->previous;
	wp->lastkind[KINDS]->previous->next = wp->lastkind[kind];
	wp->lastkind[KINDS]->previous = wp->firstkind[KINDS];
	wp->firstkind[KINDS]->next = wp->lastkind[KINDS];
}

static void
flush_kindlist(watorstruct * wp, int kind)
{
	while (wp->lastkind[kind]->previous != wp->firstkind[kind]) {
		wp->currkind = wp->lastkind[kind]->previous;
		wp->currkind->previous->next = wp->lastkind[kind];
		wp->lastkind[kind]->previous = wp->currkind->previous;
		/* wp->arr[wp->currkind->info.col + wp->currkind->info.row * wp->ncols] = 0; */
		free(wp->currkind);
	}
}

static int
positionOfNeighbor(watorstruct * wp, int n, int col, int row)
{
	int dir = n * (360 / wp->neighbors);

	if (wp->polygon == 4 || wp->polygon == 6) {
		switch (dir) {
		case 0:
			col = (col + 1 == wp->ncols) ? 0 : col + 1;
			break;
		case 45:
			col = (col + 1 == wp->ncols) ? 0 : col + 1;
			row = (!row) ? wp->nrows - 1 : row - 1;
			break;
		case 60:
			if (!(row & 1))
				col = (col + 1 == wp->ncols) ? 0 : col + 1;
			row = (!row) ? wp->nrows - 1 : row - 1;
			break;
		case 90:
			row = (!row) ? wp->nrows - 1 : row - 1;
			break;
		case 120:
			if (row & 1)
				col = (!col) ? wp->ncols - 1 : col - 1;
			row = (!row) ? wp->nrows - 1 : row - 1;
			break;
		case 135:
			col = (!col) ? wp->ncols - 1 : col - 1;
			row = (!row) ? wp->nrows - 1 : row - 1;
			break;
		case 180:
			col = (!col) ? wp->ncols - 1 : col - 1;
			break;
		case 225:
			col = (!col) ? wp->ncols - 1 : col - 1;
			row = (row + 1 == wp->nrows) ? 0 : row + 1;
			break;
		case 240:
			if (row & 1)
				col = (!col) ? wp->ncols - 1 : col - 1;
			row = (row + 1 == wp->nrows) ? 0 : row + 1;
			break;
		case 270:
			row = (row + 1 == wp->nrows) ? 0 : row + 1;
			break;
		case 300:
			if (!(row & 1))
				col = (col + 1 == wp->ncols) ? 0 : col + 1;
			row = (row + 1 == wp->nrows) ? 0 : row + 1;
			break;
		case 315:
			col = (col + 1 == wp->ncols) ? 0 : col + 1;
			row = (row + 1 == wp->nrows) ? 0 : row + 1;
			break;
		default:
			// TODO
			break;
			//(void) fprintf(stderr, "wrong direction %d\n", dir);
		}
	} else if (wp->polygon == 3) {
		if ((col + row) & 1) {	/* right */
			switch (dir) {
			case 0:
				col = (!col) ? wp->ncols - 1 : col - 1;
				break;
			case 30:
			case 40:
				col = (!col) ? wp->ncols - 1 : col - 1;
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				break;
			case 60:
				col = (!col) ? wp->ncols - 1 : col - 1;
				if (row + 1 == wp->nrows)
					row = 1;
				else if (row + 2 == wp->nrows)
					row = 0;
				else
					row = row + 2;
				break;
			case 80:
			case 90:
				if (row + 1 == wp->nrows)
					row = 1;
				else if (row + 2 == wp->nrows)
					row = 0;
				else
					row = row + 2;
				break;
			case 120:
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				break;
			case 150:
			case 160:
				col = (col + 1 == wp->ncols) ? 0 : col + 1;
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				break;
			case 180:
				col = (col + 1 == wp->ncols) ? 0 : col + 1;
				break;
			case 200:
			case 210:
				col = (col + 1 == wp->ncols) ? 0 : col + 1;
				row = (!row) ? wp->nrows - 1 : row - 1;
				break;
			case 240:
				row = (!row) ? wp->nrows - 1 : row - 1;
				break;
			case 270:
			case 280:
				if (!row)
					row = wp->nrows - 2;
				else if (!(row - 1))
					row = wp->nrows - 1;
				else
					row = row - 2;
				break;
			case 300:
				col = (!col) ? wp->ncols - 1 : col - 1;
				if (!row)
					row = wp->nrows - 2;
				else if (!(row - 1))
					row = wp->nrows - 1;
				else
					row = row - 2;
				break;
			case 320:
			case 330:
				col = (!col) ? wp->ncols - 1 : col - 1;
				row = (!row) ? wp->nrows - 1 : row - 1;
				break;
			default:
				// TODO
				break;
				//(void) fprintf(stderr, "wrong direction %d\n", dir);
			}
		} else {	/* left */
			switch (dir) {
			case 0:
				col = (col + 1 == wp->ncols) ? 0 : col + 1;
				break;
			case 30:
			case 40:
				col = (col + 1 == wp->ncols) ? 0 : col + 1;
				row = (!row) ? wp->nrows - 1 : row - 1;
				break;
			case 60:
				col = (col + 1 == wp->ncols) ? 0 : col + 1;
				if (!row)
					row = wp->nrows - 2;
				else if (row == 1)
					row = wp->nrows - 1;
				else
					row = row - 2;
				break;
			case 80:
			case 90:
				if (!row)
					row = wp->nrows - 2;
				else if (row == 1)
					row = wp->nrows - 1;
				else
					row = row - 2;
				break;
			case 120:
				row = (!row) ? wp->nrows - 1 : row - 1;
				break;
			case 150:
			case 160:
				col = (!col) ? wp->ncols - 1 : col - 1;
				row = (!row) ? wp->nrows - 1 : row - 1;
				break;
			case 180:
				col = (!col) ? wp->ncols - 1 : col - 1;
				break;
			case 200:
			case 210:
				col = (!col) ? wp->ncols - 1 : col - 1;
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				break;
			case 240:
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				break;
			case 270:
			case 280:
				if (row + 1 == wp->nrows)
					row = 1;
				else if (row + 2 == wp->nrows)
					row = 0;
				else
					row = row + 2;
				break;
			case 300:
				col = (col + 1 == wp->ncols) ? 0 : col + 1;
				if (row + 1 == wp->nrows)
					row = 1;
				else if (row + 2 == wp->nrows)
					row = 0;
				else
					row = row + 2;
				break;
			case 320:
			case 330:
				col = (col + 1 == wp->ncols) ? 0 : col + 1;
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				break;
			default:
				// TODO
				break;
				//(void) fprintf(stderr, "wrong direction %d\n", dir);
			}
		}
#if 0
	} else {
		int orient = ((row & 1) * 2 + col) % 4;
		switch (orient) { /* up, down, left, right */
		case 0:
			switch (dir) {
			case 0:
				col++;
				break;
			case 51: /* 7 */
			case 72: /* 5 */
				col = (col + 2 >= wp->ncols) ? 0 : col + 2;
				break;
			case 102: /* 7 corner */
				col = (col + 3 >= wp->ncols) ? 1 : col + 3;
				row = (row == 0) ? wp->nrows - 1 : row - 1;
				break;
			case 144: /* 5 */
			case 153: /* 7 */
				col++;
				row = (row == 0) ? wp->nrows - 1 : row - 1;
				break;
			case 204: /* 7 */
			case 216: /* 5 */
				row = (row == 0) ? wp->nrows - 1 : row - 1;
				break;
			case 255: /* 7 */
				col = (col == 0) ? wp->ncols - 1 : col - 1;
				row = (row == 0) ? wp->nrows - 1 : row - 1;
				break;
			case 288: /* 5 */
			case 306: /* 7 */
				col = (col == 0) ? wp->ncols - 1 : col - 1;
				break;
			default:
				// TODO
				// (void) fprintf(stderr, "wrong direction %d\n", dir);
			}
			break;
		case 1:
			switch (dir) {
			case 0:
				col--;
				break;
			case 51: /* 7 */
			case 72: /* 5 */
				col = (col == 1) ? wp->ncols - 1 : col - 2;
				break;
			case 102: /* 7 */
				col = (col == 1) ? wp->ncols - 2 : col - 3;
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				break;
			case 144: /* 5 */
			case 153: /* 7 */
				col--;
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				break;
			case 204: /* 7 */
			case 216: /* 5 */
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				break;
			case 255: /* 7 */
				col = (col + 1 >= wp->ncols) ? 0 : col + 1;
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				break;
			case 288: /* 5 */
			case 306: /* 7 */
				col = (col + 1 >= wp->ncols) ? 0 : col + 1;
				break;
			default:
			// TODO
				//(void) fprintf(stderr, "wrong direction %d\n", dir);
			}
			break;
		case 2:
			switch (dir) {
			case 0:
				col = (col + 1 >= wp->ncols) ? 0 : col + 1;
				break;
			case 51: /* 7 */
			case 72: /* 5 */
				row = (row == 0) ? wp->nrows - 1 : row - 1;
				col++;
				break;
			case 102: /* 7 */
				col = (col == 0) ? wp->ncols - 1 : col - 1;
				row = (row == 0) ? wp->nrows - 1 : row - 1;
				break;
			case 144: /* 5 */
			case 153: /* 7 */
				col = (col == 0) ? wp->ncols - 2 : col - 2;
				break;
			case 204: /* 7 */
			case 216: /* 5 */
				col = (col == 0) ? wp->ncols - 1 : col - 1;
				break;
			case 255: /* 7 */
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				col = (col == 0) ? wp->ncols - 1 : col - 1;
				break;
			case 288: /* 5 */
			case 306: /* 7 */
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				break;
			default:
			// TODO
				//(void) fprintf(stderr, "wrong direction %d\n", dir);
			}
			break;
		case 3:
			switch (dir) {
			case 0:
				col--;
				break;
			case 51: /* 7 */
			case 72: /* 5 */
				col = (col == 0) ? wp->ncols - 1 : col - 1;
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				break;
			case 102: /* 7 */
				col = (col + 1 >= wp->ncols) ? 0 : col + 1;
				row = (row + 1 == wp->nrows) ? 0 : row + 1;
				break;
			case 144: /* 5 */
			case 153: /* 7 */
				col = (col + 2 >= wp->ncols) ? 1 : col + 2;
				break;
			case 204: /* 7 */
			case 216: /* 5 */
				col = (col + 1 >= wp->ncols) ? 0 : col + 1;
				break;
			case 255: /* 7 */
				col = (col + 1 >= wp->ncols) ? 0 : col + 1;
				row = (row == 0) ? wp->nrows - 1 : row - 1;
				break;
			case 288: /* 5 */
			case 306: /* 7 */
				row = (row == 0) ? wp->nrows - 1 : row - 1;
				break;
			default:
			// TODO
				//(void) fprintf(stderr, "wrong direction %d\n", dir);
			}
			break;
		default:
		// TODO
			//(void) fprintf(stderr, "wrong orient %d\n", orient);
		}
#endif
	}
	return (row * wp->ncols + col);
}

static void
free_wator_screen(watorstruct *wp)
{
	int kind;

	if (wp == NULL) {
		return;
	}
	for (kind = 0; kind <= KINDS; kind++) {
		if (wp->firstkind[kind] != NULL) {
			flush_kindlist(wp, kind);
			free(wp->firstkind[kind]);
			wp->firstkind[kind] = (struct _CellList *) NULL;
		}
		if (wp->lastkind[kind] != NULL) {
			free(wp->lastkind[kind]);
			wp->lastkind[kind] = (struct _CellList *) NULL;
		}
	}
	if (wp->arr != NULL) {
		free(wp->arr);
		wp->arr = (CellList **) NULL;
	}
	wp = NULL;
}

extern "C"
EMSCRIPTEN_KEEPALIVE void
init_wator(int xSize, int ySize, unsigned int seed)
{
	int         i, col, row, colrow, kind;
	cellstruct  info;
	watorstruct *wp = &wators;
	gen.seed(seed);

	wp->generation = 0;
	if (!wp->firstkind[0]) {	/* Genesis */
		//icon_width = fish0_width;
		//icon_height = fish0_height;
		/* Set up what will be a 'triply' linked list.
		   doubly linked list, doubly linked to an array */
		for (kind = FISH; kind <= KINDS; kind++)
			if (!init_kindlist(wp, kind)) {
				free_wator_screen(wp);
				return;
			}
		/*for (i = 0; i < BITMAPS; i++) {
			logo[i].width = icon_width;
			logo[i].height = icon_height;
			logo[i].bytes_per_line = (icon_width + 7) / 8;
		}*/
	} else			/* Exterminate all  */
		for (i = FISH; i <= KINDS; i++)
			flush_kindlist(wp, i);
	// TODO
	wp->vertical = false;
	/*if (MI_IS_FULLRANDOM(mi)) {
		wp->vertical = (bool) (LRAND() & 1);
	} else {
		wp->vertical = vertical;
	}*/
	// TODO
	wp->width = xSize;
	wp->height = ySize;
	if (wp->width < 2)
		wp->width = 2;
	if (wp->height < 2)
		wp->height = 2;

	for (i = 0; i < NEIGHBORKINDS; i++) {
		if (neighbors == plots[i]) {
			wp->neighbors = neighbors;
			break;
		}
		if (i == NEIGHBORKINDS - 1) {
#if 0
			wp->neighbors = plots[NRAND(NEIGHBORKINDS)];
			wp->neighbors = (LRAND() & 1) ? 4 : 8;
#else
			wp->neighbors = 4;
#endif
			break;
		}
	}

	/*if (wp->neighbors == 6) {
		int nccols, ncrows, sides;

		wp->polygon = 6;
		if (!wp->vertical) {
			wp->height = 100;
			wp->width = 100;
		}
		if (wp->width < 8)
			wp->width = 8;
		if (wp->height < 8)
			wp->height = 8;
		if (size < -MINSIZE)
			wp->ys = NRAND(MIN(-size, MAX(MINSIZE, MIN(wp->width, wp->height) /
				      MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
		else if (size < MINSIZE) {
			if (!size)
				wp->ys = MAX(MINSIZE, MIN(wp->width, wp->height) / MINGRIDSIZE);
			else
				wp->ys = MINSIZE;
		} else
			wp->ys = MIN(size, MAX(MINSIZE, MIN(wp->width, wp->height) /
					       MINGRIDSIZE));
		wp->xs = wp->ys;
		wp->pixelmode = true;
		nccols = MAX(wp->width / wp->xs - 2, 2);
		ncrows = MAX(wp->height / wp->ys - 1, 4);
		wp->ncols = nccols / 2;
		wp->nrows = 2 * (ncrows / 4);
		wp->xb = (wp->width - wp->xs * nccols) / 2 + wp->xs / 2;
		wp->yb = (wp->height - wp->ys * (ncrows / 2) * 2) / 2 +
			wp->ys / 3;
		for (sides = 0; sides < 6; sides++) {
			if (wp->vertical) {
				wp->shape.hexagon[sides].x =
					(wp->xs - 1) * hexagonUnit[sides].x;
				wp->shape.hexagon[sides].y =
					((wp->ys - 1) * hexagonUnit[sides].y /
					2) * 4 / 3;
			} else {
				wp->shape.hexagon[sides].y =
					(wp->xs - 1) * hexagonUnit[sides].x;
				wp->shape.hexagon[sides].x =
					((wp->ys - 1) * hexagonUnit[sides].y /
					2) * 4 / 3;
			}
		}
	}*/
	if (wp->neighbors == 4 || wp->neighbors == 8) {
		wp->polygon = 4;
		if (wp->width < 2)
			wp->width = 2;
		if (wp->height < 2)
			wp->height = 2;
		wp->pixelmode = true;
		wp->xs = 1;
		wp->ys = 1;
		wp->xs = wp->ys;
		wp->ncols = MAX(wp->width / wp->xs, 2);
		wp->nrows = MAX(wp->height / wp->ys, 2);
		wp->xb = (wp->width - wp->xs * wp->ncols) / 2;
		wp->yb = (wp->height - wp->ys * wp->nrows) / 2;
	}
	//  else {		/* TRI */
	// 	int orient, sides;

	// 	wp->polygon = 3;
	// 	if (!wp->vertical) {
	// 		wp->height = 100;
	// 		wp->width = 100;
	// 	}
	// 	if (wp->width < 2)
	// 		wp->width = 2;
	// 	if (wp->height < 2)
	// 		wp->height = 2;
	// 	if (size < -MINSIZE)
	// 		wp->ys = NRAND(MIN(-size, MAX(MINSIZE, MIN(wp->width, wp->height) /
	// 			      MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
	// 	else if (size < MINSIZE) {
	// 		if (!size)
	// 			wp->ys = MAX(MINSIZE, MIN(wp->width, wp->height) / MINGRIDSIZE);
	// 		else
	// 			wp->ys = MINSIZE;
	// 	} else
	// 		wp->ys = MIN(size, MAX(MINSIZE, MIN(wp->width, wp->height) /
	// 				       MINGRIDSIZE));
	// 	wp->xs = (int) (1.52 * wp->ys);
	// 	wp->pixelmode = true;
	// 	wp->ncols = (MAX(wp->width / wp->xs - 1, 2) / 2) * 2;
	// 	wp->nrows = (MAX(wp->height / wp->ys - 1, 2) / 2) * 2;
	// 	wp->xb = (wp->width - wp->xs * wp->ncols) / 2 + wp->xs / 2;
	// 	wp->yb = (wp->height - wp->ys * wp->nrows) / 2 + wp->ys / 2;
	// 	for (orient = 0; orient < 2; orient++) {
	// 		for (sides = 0; sides < 3; sides++) {
	// 			if (wp->vertical) {
	// 				wp->shape.triangle[orient][sides].x =
	// 					(wp->xs - 2) * triangleUnit[orient][sides].x;
	// 				wp->shape.triangle[orient][sides].y =
	// 					(wp->ys - 2) * triangleUnit[orient][sides].y;
	// 			} else {
	// 				wp->shape.triangle[orient][sides].y =
	// 					(wp->xs - 2) * triangleUnit[orient][sides].x;
	// 				wp->shape.triangle[orient][sides].x =
	// 					(wp->ys - 2) * triangleUnit[orient][sides].y;
	// 			}
	// 		}
	// 	}
	// }

	wp->positions = wp->ncols * wp->nrows;

	if (wp->arr != NULL)
		free(wp->arr);
	if ((wp->arr = (CellList **) calloc(wp->positions,
			sizeof (CellList *))) == NULL) {
		free_wator_screen(wp);
		return;
	}
	free (cellDataForJS);
	cellDataForJS = (CellDataForJS*) calloc(xSize * ySize, sizeof(CellDataForJS));

	/* Play G-d with these numbers */
	wp->nkind[FISH] = wp->positions / 3;
	wp->nkind[SHARK] = wp->nkind[FISH] / 10;
	wp->kind = FISH;
	if (!wp->nkind[SHARK])
		wp->nkind[SHARK] = 1;
	// TODO - is this right?
	wp->breed[FISH] = 1; //MI_COUNT(mi);
	wp->breed[SHARK] = 10;
	if (wp->breed[FISH] < 1)
		wp->breed[FISH] = 1;
	else if (wp->breed[FISH] > wp->breed[SHARK])
		wp->breed[FISH] = 4;
	wp->sstarve = 3;

	// TODO
	//MI_CLEARWINDOW(mi);
	wp->painted = false;

	for (kind = FISH; kind <= SHARK; kind++) {
		i = 0;
		while (i < wp->nkind[kind]) {
			col = NRAND(wp->ncols);
			row = NRAND(wp->nrows);
			colrow = col + row * wp->ncols;
			if (!wp->arr[colrow]) {
				i++;
				info.kind = kind;
				info.age = NRAND(wp->breed[kind]);
				info.food = NRAND(wp->sstarve);
				info.direction = NRAND(KINDBITMAPS) + kind * KINDBITMAPS;
				// TODO?
				info.color = NRAND(8);
				info.col = col;
				info.row = row;
				if (!addto_kindlist(wp, kind, info)) {
					free_wator_screen(wp);
					return;
				}
				wp->arr[colrow] = wp->currkind;
				drawcell(col, row,
					 wp->currkind->info.color, wp->currkind->info.direction, true);
			}
		}
	}
}

extern "C"
EMSCRIPTEN_KEEPALIVE void
draw_wator()
{
	int         col, row;
	int         colrow, cr, position;
	int         i, numok;
	struct {
		int         pos, dir;
	} acell[12];
	watorstruct *wp = &wators;

	if (wp->arr == NULL)
		return;

	//MI_IS_DRAWN(mi) = true;
	wp->painted = true;
	/* Alternate updates, fish and sharks live out of phase with each other */
	wp->kind = (wp->kind + 1) % KINDS;
	{
		wp->currkind = wp->firstkind[wp->kind]->next;

		while (wp->currkind != wp->lastkind[wp->kind]) {
			col = wp->currkind->info.col;
			row = wp->currkind->info.row;
			colrow = col + row * wp->ncols;
			numok = 0;
			if (wp->kind == SHARK) {	/* Scan for fish */
				for (i = 0; i < wp->neighbors; i++) {
					position = positionOfNeighbor(wp, i, col, row);
					if (wp->arr[position] && wp->arr[position]->info.kind == FISH) {
						acell[numok].pos = position;
						acell[numok++].dir = i;
					}
				}
				if (numok) {	/* No thanks, I'm a vegetarian */
					i = NRAND(numok);
					wp->nkind[FISH]--;
					cr = acell[i].pos;
					removefrom_kindlist(wp, wp->arr[cr]);
					wp->arr[cr] = wp->currkind;
					if (wp->neighbors == 4) {
						wp->currkind->info.direction = (5 - acell[i].dir) % ORIENTS +
							((NRAND(REFLECTS)) ? 0 : ORIENTS) + wp->kind * KINDBITMAPS;
					} else if (wp->neighbors == 8) {
						wp->currkind->info.direction = (char) (5 - (acell[i].dir / 2 +
											    ((acell[i].dir % 2) ? LRAND() & 1 : 0))) % ORIENTS +
							((NRAND(REFLECTS)) ? 0 : ORIENTS) + wp->kind * KINDBITMAPS;
					} else
						wp->currkind->info.direction = wp->kind * KINDBITMAPS;
					wp->currkind->info.col = acell[i].pos % wp->ncols;
					wp->currkind->info.row = acell[i].pos / wp->ncols;
					wp->currkind->info.food = wp->sstarve;
					drawcell(wp->currkind->info.col, wp->currkind->info.row,
						 wp->currkind->info.color, wp->currkind->info.direction, true);
					if (++(wp->currkind->info.age) >= wp->breed[wp->kind]) {	/* breed */
						cutfrom_kindlist(wp);	/* This rotates out who goes first */
						wp->babykind->info.age = 0;
						if (!dupin_kindlist(wp)) {
							free_wator_screen(wp);
							return;
						}
						wp->arr[colrow] = wp->babykind;
						wp->babykind->info.col = col;
						wp->babykind->info.row = row;
						wp->babykind->info.age = -1;	/* Make one a little younger */
#if 0
						if (MI_NPIXELS(mi) > 2 && (LRAND() & 1))
							/* A color mutation */
							if (++(wp->babykind->info.color) >= MI_NPIXELS(mi))
								wp->babykind->info.color = 0;
#endif
						wp->nkind[wp->kind]++;
					} else {
						wp->arr[colrow] = 0;
						drawcell(col, row, 0, 0, false);
					}
				} else {
					if (wp->currkind->info.food-- < 0) {	/* Time to die, Jaws */
						/* back up one or else in void */
						wp->currkind = wp->currkind->previous;
						removefrom_kindlist(wp, wp->arr[colrow]);
						wp->arr[colrow] = 0;
						drawcell(col, row, 0, 0, false);
						wp->nkind[wp->kind]--;
						numok = -1;	/* Want to escape from next if */
					}
				}
			}
			if (!numok) {	/* Fish or shark search for a place to go */
				for (i = 0; i < wp->neighbors; i++) {
					position = positionOfNeighbor(wp, i, col, row);
					if (!wp->arr[position]) {	/* Found an empty spot */
						acell[numok].pos = position;
						acell[numok++].dir = i;
					}
				}
				if (numok) {	/* Found a place to go */
					i = NRAND(numok);
					wp->arr[acell[i].pos] = wp->currkind;
					if (wp->neighbors == 4) {
						wp->currkind->info.direction = (5 - acell[i].dir) % ORIENTS +
							((NRAND(REFLECTS)) ? 0 : ORIENTS) + wp->kind * KINDBITMAPS;
					} else if (wp->neighbors == 8) {
						wp->currkind->info.direction = (char) (5 - (acell[i].dir / 2 +
											    ((acell[i].dir % 2) ? LRAND() & 1 : 0))) % ORIENTS +
							((NRAND(REFLECTS)) ? 0 : ORIENTS) + wp->kind * KINDBITMAPS;
					} else
						wp->currkind->info.direction = wp->kind * KINDBITMAPS;
					wp->currkind->info.col = acell[i].pos % wp->ncols;
					wp->currkind->info.row = acell[i].pos / wp->ncols;
					drawcell(
						 wp->currkind->info.col, wp->currkind->info.row,
						 wp->currkind->info.color, wp->currkind->info.direction, true);
					if (++(wp->currkind->info.age) >= wp->breed[wp->kind]) {	/* breed */
						cutfrom_kindlist(wp);	/* This rotates out who goes first */
						wp->babykind->info.age = 0;
						if (!dupin_kindlist(wp)) {
							free_wator_screen(wp);
							return;
						}
						wp->arr[colrow] = wp->babykind;
						wp->babykind->info.col = col;
						wp->babykind->info.row = row;
						wp->babykind->info.age = -1;	/* Make one a little younger */
						wp->nkind[wp->kind]++;
					} else {
						wp->arr[colrow] = 0;
						drawcell(col, row, 0, 0, false);
					}
				} else {
					/* I'll just sit here and wave my tail so you know I am alive */
					wp->currkind->info.direction =
						(wp->currkind->info.direction + ORIENTS) % KINDBITMAPS +
						wp->kind * KINDBITMAPS;
					drawcell(col, row, wp->currkind->info.color,
					 wp->currkind->info.direction, true);
				}
			}
			wp->currkind = wp->currkind->next;
		}
		reattach_kindlist(wp, wp->kind);
	}

	if ((wp->nkind[FISH] >= wp->positions) ||
	    (!wp->nkind[FISH] && !wp->nkind[SHARK]) ||
		// TODO - return that we're done I guess?

		// TODO??
	    //wp->generation >= MI_CYCLES(mi)) {
		false) {
		// TODO??
		//init_wator(mi);
		//init_wator(100);
	}
	if (wp->kind == SHARK)
		wp->generation++;
}

ENTRYPOINT void
release_wator()
{
	//if (wators != NULL) {
		free_wator_screen(&wators);
		//free(wators);
		//wators = (watorstruct *) NULL;
	//}
}

#endif /* MODE_wator */
