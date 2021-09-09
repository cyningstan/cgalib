/*======================================================================
 * CGALib - Watcom C Version.
 * Bitmap Module.
 *
 * Definitions for the bitmap functions. Also includes the bitmap
 * structure for internal use only.
 * 
 * Released as Public Domain by Damian Gareth Walker, 2020.
 * Created 30-Jun-2020.
 */

/* default CGALIB macro for access to internal structures */
#define __CGALIB__

/* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgalib.h"

/*----------------------------------------------------------------------
 * Public Level Functions.
 */

/**
 * Create a bitmap.
 * @param width is the width of the bitmap.
 * @param height is the height of the bitmap.
 * @returns a new Bitmap.
 */
Bitmap *bit_create (int width, int height)
{
    /* local variables */
    Bitmap *bitmap;

    /* reserve memory */
    if (! (bitmap = malloc (sizeof (Bitmap))))
        return NULL;
    if (! (bitmap->pixels = malloc (width / 4 * height))) {
        free (bitmap);
        return NULL;
    }

    /* initialise the data */
    bitmap->width = width;
    bitmap->height = height;
    bitmap->ink = 3;
    bitmap->paper = 0;
    bitmap->font = NULL;

    /* return the bitmap */
    return bitmap;
}

/**
 * Create a bitmap by copying another.
 * @param src is the bitmap to copy.
 * @returns a new Bitmap.
 */
Bitmap *bit_copy (Bitmap *src)
{
    /* local variables */
    Bitmap *dst;

    /* reserve memory */
    if (! (dst = malloc (sizeof (Bitmap))))
        return NULL;
    if (! (dst->pixels = malloc (src->width / 4 * src->height))) {
        free (dst);
        return NULL;
    }

    /* initialise the data */
    dst->width = src->width;
    dst->height = src->height;
    dst->ink = src->ink;
    dst->paper = src->paper;
    dst->font = src->font;
    memcpy (dst->pixels, src->pixels, src->width / 4 * src->height);

    /* return the bitmap */
    return dst;

}

/**
 * Read a bitmap from an already-open file.
 * @param input is the input file handle.
 * @returns a new Bitmap.
 */
Bitmap *bit_read (FILE *input)
{
    /* local variables */
    Bitmap *bitmap; /* the bitmap to return */
    int w; /* the width read from a file */
    int h; /* the height read from a file */

    /* attempt to read the width and height */
    if (! fread (&w, 2, 1, input))
        return NULL;
    if (! fread (&h, 2, 1, input))
        return NULL;
    
    /* reserve memory for bitmap and its pixels */
    if (! (bitmap = malloc (sizeof (Bitmap))))
        return NULL;
    if (! (bitmap->pixels = malloc (w / 4 * h))) {
        free (bitmap);
        return NULL;
    }

    /* attempt to read the pixels */
    if (! fread (bitmap->pixels, w / 4 * h, 1, input)) {
        free (bitmap->pixels);
        free (bitmap);
        return NULL;
    }

    /* set the rest of the variables in the bitmap */
    bitmap->width = w;
    bitmap->height = h;
    bitmap->ink = 3;
    bitmap->paper = 0;
    bitmap->font = NULL;

    /* return the bitmap */
    return bitmap;
}

/**
 * Write a bitmap to an already-open file.
 * @param bitmap is the bitmap to write.
 * @param output is the file handle to write to.
 */
void bit_write (Bitmap *bitmap, FILE *output)
{
    if (! fwrite (&bitmap->width, 2, 1, output))
        return;
    if (! fwrite (&bitmap->height, 2, 1, output))
        return;
    if (! fwrite (bitmap->pixels, bitmap->width / 4 * bitmap->height, 1,
        output))
        return;
}

/**
 * Put part of a bitmap onto another bitmap.
 * @param dst is the bitmap to affect.
 * @param src is the source bitmap.
 * @param xd is the x coordinate at which the bitmap is to be placed.
 * @param yd is the y coordinate at which the bitmap is to be placed.
 * @param xs is the x coordinate of the source to copy from.
 * @param ys is the y coordinate of the source to copy from.
 * @param w is the width of the section to copy.
 * @param h is the height of the section to copy.
 * @param d is the draw mode.
 */
void bit_putpart (Bitmap *dst, Bitmap *src, int xd, int yd,
		  int xs, int ys, int w, int h, DrawMode draw)
{
    /* local variables */
    char *d; /* address to copy data to */
    char *s; /* address to copy data from */
    int r; /* row counter */
    int b; /* byte counter */

    /* copy the pixels */
    for (r = 0; r < h; ++r)

        /* DRAW_PSET can be copied by a quicker method */
        if (draw == DRAW_PSET) {
            d = dst->pixels + xd / 4 + (yd + r) * (dst->width / 4);
            s = src->pixels + (xs / 4) + (src->width / 4) * (ys + r);
            _fmemcpy (d, s, w / 4);
        }
        
        /* the other draw operations need doing byte by byte */
        else 
            for (b = 0; b < w / 4; ++b) {
                d = dst->pixels + b + xd / 4
                    + (yd + r) * (dst->width / 4);
                s = b + src->pixels + (xs / 4)
		    + (src->width) / 4 * (ys + r);
                switch (draw) {
                    case DRAW_PRESET:
                        *d = ~*s;
                        break;
                    case DRAW_AND:
                        *d &= *s;
                        break;
                    case DRAW_OR:
                        *d |= *s;
                        break;
                    case DRAW_XOR:
                        *d ^= *s;
                        break;
                }
            }
}

/**
 * Put one bitmap onto another, with clipping.
 * @param dst is the destination bitmap.
 * @param src is the source bitmap.
 * @param x is the y coordinate on the destination bitmap.
 * @param y is the x coordinate on the destination bitmap.
 * @param draw is the drawing mode to use.
 */
void bit_put (Bitmap *dst, Bitmap *src, int x, int y, DrawMode draw)
{
    /* local variables */
    char *d; /* address to copy data to */
    char *s; /* address to copy data from */
    int r; /* row counter */
    int b; /* byte counter */

    /* copy the pixels */
    for (r = 0; r < src->height; ++r)

        /* DRAW_PSET can be copied by a quicker method */
        if (draw == DRAW_PSET) {
            d = dst->pixels + x / 4 + (y + r) * (dst->width / 4);
            s = src->pixels + src->width / 4 * r;
            _fmemcpy (d, s, src->width / 4);
        }
        
        /* the other draw operations need doing byte by byte */
        else 
            for (b = 0; b < src->width / 4; ++b) {
                d = dst->pixels + b + x / 4
                    + (y + r) * (dst->width / 4);
                s = b + src->pixels + src->width / 4 * r;
                switch (draw) {
                    case DRAW_PRESET:
                        *d = ~*s;
                        break;
                    case DRAW_AND:
                        *d &= *s;
                        break;
                    case DRAW_OR:
                        *d |= *s;
                        break;
                    case DRAW_XOR:
                        *d ^= *s;
                        break;
                }
            }
}

/**
 * Get one bitmap from another.
 * @param src is the source bitmap.
 * @param dst is the destination bitmap.
 * @param x is the x coordinate on the source bitmap.
 * @param y is the y coordinate on the source bitmap.
 */
void bit_get (Bitmap *src, Bitmap *dst, int x, int y)
{
    /* local variables */
    char *s; /* address to copy data from */
    char *d; /* address to copy data to */
    int r; /* row counter */

    /* copy the pixels */
    for (r = 0; r < dst->height; ++r) {
        s = src->pixels + x / 4 + (y + r) * (src->width / 4);
        d = dst->pixels + dst->width / 4 * r;
        _fmemcpy (d, s, dst->width / 4);
    }
}

/**
 * Draw a box onto a Bitmap, filled in the current ink colour.
 * @param bitmap is the destination bitmap.
 * @param x is the x coordinate of the top left of the box.
 * @param y is the y coordinate of the top left of the box.
 * @param width is the width of the box.
 * @param height is the height of the box.
 * The draw mode is always DRAW_PUT. For anything more sophisticated,
 * the scr_put() function should be used.
 */
void bit_box (Bitmap *bitmap, int x, int y, int width, int height)
{
    /* local variables */
    char v; /* value to write to the screen */
    int r; /* row counter */
    char *d; /* address to write to */

    /* determine the byte value that will fill the box */
    v = bitmap->ink * 0x55;

    /* fill each individual row */
    for (r = 0; r < height; ++r) {
        d = bitmap->pixels + x / 4 + (y + r) * (bitmap->width / 4);
        _fmemset (d, v, width / 4);
    }
}

/**
 * Print some text onto a bitmap in the current ink colour.
 * @param bitmap is the destination bitmap.
 * @param x is the x coordinate where the text should appear.
 * @param y is the y coordinate where the text should appear.
 * @param message is the message to print.
 * The text is printed in a box of the paper colour.
 */
void bit_print (Bitmap *bitmap, int x, int y, char *message)
{
    /* local variables */
    int b; /* character pointer */
    int r; /* row of character */
    char v; /* value of byte in character */
    char *d; /* pointer to destination byte on screen */

    /* only try to print if a font is selected */
    if (! bitmap->font) return;

    /* print each character */
    for (b = 0; message[b]; ++b)
        for (r = 0; r < 8; ++r) {
            d = bitmap->pixels + b + x / 4 +
                (y + r) * (bitmap->width / 4);
            v = bitmap->font->pixels[r + 8
                * (message[b] - bitmap->font->first)];
            if (bitmap->ink != 3 || bitmap->paper != 0)
                v = (v & 0x55 * bitmap->ink)
                    | ((0xff ^ v) & 0x55 * bitmap->paper);
            *d = v;
        }
}

/**
 * Set the ink colour for scr_box and scr_print operations.
 * @param bitmap is the bitmap to set the colour for.
 * @param ink is the colour to use, 0..3.
 */
void bit_ink (Bitmap *bitmap, int ink)
{
    bitmap->ink = ink;
}

/**
 * Set the paper colour for scr_print operations.
 * @param bitmap is the bitmap to set the colour for.
 * @param paper is the colour to use, 0..3.
 */
void bit_paper (Bitmap *bitmap, int paper)
{
    bitmap->paper = paper;
}

/**
 * Set the font for scr_print operations.
 * @param bitmap is the bitmap to set the font for.
 * @param font is the font to use.
 */
void bit_font (Bitmap *bitmap, Font *font)
{
    bitmap->font = font;
}

/**
 * Destroy a bitmap.
 * @param bitmap is the bitmap to destroy.
 */
void bit_destroy (Bitmap *bitmap)
{
    if (bitmap) {
        if (bitmap->pixels)
            free (bitmap->pixels);
        free (bitmap);
    }
}
