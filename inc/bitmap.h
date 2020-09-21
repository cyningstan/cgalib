/*======================================================================
 * CGALib - Watcom C Version.
 * Bitmap Module Header.
 *
 * Definitions for the bitmap functions. Also includes the bitmap
 * structure for internal use only.
 * 
 * Released as Public Domain by Damian Gareth Walker, 2020.
 * Created 29-Jun-2020.
 */

#ifndef __BITMAP_H__
#define __BITMAP_H__

/*----------------------------------------------------------------------
 * Internal Structures.
 */

/** @struct bitmap holds the information for a bitmap */
#ifdef __CGALIB__
struct bitmap {

    /** @var width is the width of the bitmap in pixels */
    int width;

    /** @var height is the height of the bitmap in pixels */
    int height;

    /** @var ink is the ink colour used in the bitmap */
    int ink;

    /** @var paper is the paper colour used in the bitmap */
    int paper;

    /** @ar font is the font used for bit_print() operations */
    Font *font;

    /** @var pixels is a pointer to the pixel data */
    char *pixels;

};
#endif

/*----------------------------------------------------------------------
 * Public Level Function Prototypes.
 */

/**
 * Create a bitmap.
 * @param w is the width of the bitmap.
 * @param h is the height of the bitmap.
 * @returns a new Bitmap.
 */
Bitmap *bit_create (int w, int h);

/**
 * Create a bitmap by copying another.
 * @param src is the bitmap to copy.
 * @returns a new Bitmap.
 */
Bitmap *bit_copy (Bitmap *src);

/**
 * Read a bitmap from an already-open file.
 * @param input is the input file handle.
 * @returns a new Bitmap.
 */
Bitmap *bit_read (FILE *input);

/**
 * Write a bitmap to an already-open file.
 * @param bitmap is the bitmap to write.
 * @param output is the file handle to write to.
 */
void bit_write (Bitmap *bitmap, FILE *output);

/**
 * Put one bitmap onto another, with clipping.
 * @param dst is the destination bitmap.
 * @param src is the source bitmap.
 * @param x is the y coordinate on the destination bitmap.
 * @param y is the x coordinate on the destination bitmap.
 * @param d is the drawing mode to use.
 */
void bit_put (Bitmap *dst, Bitmap *src, int x, int y, DrawMode draw);

/**
 * Get one bitmap from another.
 * @param src is the source bitmap.
 * @param dst is the destination bitmap.
 * @param x is the x coordinate on the source bitmap.
 * @param y is the y coordinate on the source bitmap.
 */
void bit_get (Bitmap *src, Bitmap *dst, int x, int y);

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
void bit_box (Bitmap *bitmap, int x, int y, int width, int height);

/**
 * Print some text onto a bitmap in the current ink colour.
 * @param bitmap is the destination bitmap.
 * @param x is the x coordinate where the text should appear.
 * @param y is the y coordinate where the text should appear.
 * @param message is the message to print.
 * The text is printed in a box of the paper colour.
 */
void bit_print (Bitmap *bitmap, int x, int y, char *message);

/**
 * Set the ink colour for scr_box and scr_print operations.
 * @param bitmap is the bitmap to set the colour for.
 * @param ink is the colour to use, 0..3.
 */
void bit_ink (Bitmap *bitmap, int ink);

/**
 * Set the paper colour for scr_print operations.
 * @param bitmap is the bitmap to set the colour for.
 * @param paper is the colour to use, 0..3.
 */
void bit_paper (Bitmap *bitmap, int paper);

/**
 * Set the font for scr_print operations.
 * @param bitmap is the bitmap to set the font for.
 * @param font is the font to use.
 */
void bit_font (Bitmap *bitmap, Font *font);

/**
 * Destroy a bitmap.
 * @param bitmap is the bitmap to destroy.
 */
void bit_destroy (Bitmap *bitmap);

#endif
