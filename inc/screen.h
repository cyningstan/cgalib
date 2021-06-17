/*======================================================================
 * CGALib - Watcom C Version.
 * Screen Header File.
 *
 * Definitions for the hardware screen functions.
 * 
 * Released as Public Domain by Damian Gareth Walker, 2020.
 * Created 29-Jun-2020.
 */

#ifndef __SCREEN_H__
#define __SCREEN_H__

/*----------------------------------------------------------------------
 * Internal structures.
 */

/** @struct screen holds screen information */
#ifdef __CGALIB__
struct screen {

    /** @var mode is the video mode */
    int mode;

    /** @var palette is the palette number */
    int palette;

    /** @var colour is the palette background colour */
    int colour;

    /** @var ink is the ink colour */
    int ink;

    /** @var paper is the paper colour */
    int paper;

    /** @var font is the current font */
    Font *font;
};
#endif

/*----------------------------------------------------------------------
 * Public Level Function Prototypes.
 */

/**
 * Initialise the screen, and set its initial screen mode.
 * @param m is the screen mode: 4, 5 or 6.
 * @returns the new screen.
 */
Screen *scr_create (int mode);

/**
 * Set the screen palette.
 * @param screen is the screen to affect.
 * @param p is the palette number, 0..5.
 * @param c is the background colour, 0..15.
 */
void scr_palette (Screen *screen, int palette, int colour);

/**
 * Put part of a bitmap onto the screen.
 * @param dst is the screen to affect.
 * @param src is the source bitmap.
 * @param xd is the x coordinate at which the bitmap is to be placed.
 * @param yd is the y coordinate at which the bitmap is to be placed.
 * @param xs is the x coordinate of the source to copy from.
 * @param ys is the y coordinate of the source to copy from.
 * @param w is the width of the section to copy.
 * @param h is the height of the section to copy.
 * @param d is the draw mode.
 */
void scr_putpart (Screen *dst, Bitmap *src, int xd, int yd,
		  int xs, int ys, int w, int h, DrawMode draw);

/**
 * Put a bitmap onto the screen.
 * @param dst is the screen to affect.
 * @param src is the source bitmap.
 * @param x is the x coordinate at which the bitmap is to be placed.
 * @param y is the y coordinate at which the bitmap is to be placed.
 * @param d is the draw mode.
 */
void scr_put (Screen *dst, Bitmap *src, int x, int y, DrawMode draw);

/**
 * Get a bitmap from the screen.
 * @param src is the screen from which the bitmap comes.
 * @param dst is the destination bitmap, whose size is already set.
 * @param x is the x coordinate where the desired pattern lies.
 * @param y is the y coordinate where the desired pattern lies.
 */
void scr_get (Screen *src, Bitmap *dst, int x, int y);

/**
 * Draw a box on the screen, filled in the current ink colour.
 * @param screen is the screen to affect.
 * @param x is the x coordinate of the top left of the box.
 * @param y is the y coordinate of the top left of the box.
 * @param w is the width of the box.
 * @param h is the height of the box.
 * The draw mode is always DRAW_PUT. For anything more sophisticated,
 * the scr_put() function should be used.
 */
void scr_box (Screen *screen, int x, int y, int width, int height);

/**
 * Print some text on the screen in the current ink colour.
 * @param screen is the screen to affect.
 * @param x is the x coordinate where the text should appear.
 * @param y is the y coordinate where the text should appear.
 * @param message is the message to print.
 * The text is printed in a box of the paper colour.
 */
void scr_print (Screen *screen, int x, int y, char *message);

/**
 * Set the ink colour for scr_box and scr_print operations.
 * @param screen is the screen to affect.
 * @param ink is the colour to use, 0..3.
 */
void scr_ink (Screen *screen, int ink);

/**
 * Set the paper colour for scr_print operations.
 * @param screen is the screen to affect.
 * @param paper is the colour to use, 0..3.
 */
void scr_paper (Screen *screen, int paper);

/**
 * Set the font for scr_print operations.
 * @param screen is the screen to affect.
 * @param font is the font to use.
 */
void scr_font (Screen *screen, Font *font);

/**
 * Reset the screen back to text mode which graphic output is finished.
 * @param screen is the screen to affect.
 */
void scr_destroy (Screen *screen);

#endif
