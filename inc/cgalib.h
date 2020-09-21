/*======================================================================
 * CGALib - Watcom C Version.
 * Main Header File.
 *
 * Provides a central header file for programs using CGALib.
 * Includes the other headers for all functionality.
 * 
 * Released as Public Domain by Damian Gareth Walker, 2020.
 * Created 29-Jun-2020.
 */

#ifndef __CGALIB_H__
#define __CGALIB_H__

#include <stdio.h>

/* Structure Type Definitions */
typedef struct screen Screen;
typedef struct bitmap Bitmap;
typedef struct font Font;

/* Enum Type Definitions */
typedef enum {
    DRAW_PSET,
    DRAW_PRESET,
    DRAW_AND,
    DRAW_OR,
    DRAW_XOR
} DrawMode;

/* included headers */
#include "screen.h"
#include "bitmap.h"
#include "font.h"

#endif
