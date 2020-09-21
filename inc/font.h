/*======================================================================
 * CGALib - Watcom C Version.
 * Font Module Header.
 *
 * Definitions for the bitmap functions. Also includes the bitmap
 * structure for internal use only.
 * 
 * Released as Public Domain by Damian Gareth Walker, 2020.
 * Created 29-Jun-2020.
 */

#ifndef __FONT_H__
#define __FONT_H__

/*----------------------------------------------------------------------
 * Internal Structures.
 */

/** @struct font holds the data for a single font. */
#ifdef __CGALIB__
struct font {

    /** @var first is the first character in a font */
    int first;

    /** @var lst is the last character in a font */
    int last;

    /** @var pixels is the pixel data for each character */
    char *pixels;
};
#endif

/*----------------------------------------------------------------------
 * Public Level Function Prototypes.
 */

/**
 * Create a new font.
 * @param first is the first character in the font.
 * @param last is the last character in the font.
 * @returns the new font.
 */
Font *fnt_create (int first, int last);

/**
 * Copy a font.
 * @param font is the font to copy.
 * @returns a new font copied from the old.
 */
Font *fnt_copy (Font *src);

/**
 * Read a font from an already open file.
 * @param input is the input file handle.
 * @returns the new font.
 */
Font *fnt_read (FILE *input);

/**
 * Write a font to an already open file.
 * @param font is the font to write.
 * @param output is the output file handle.
 */
void fnt_write (Font *font, FILE *output);

/**
 * Put a bitmap into a font character.
 * @param dst is the destination font.
 * @param src is the source bitmap.
 * @param ch is the character to set.
 */
void fnt_put (Font *dst, Bitmap *src, int ch);

/**
 * Get a bitmap from a font character.
 * @param src is the source font.
 * @param dst is the destination bitmap.
 * @param ch is the character to get.
 */
void fnt_get (Font *src, Bitmap *dst, int ch);

/**
 * Set the font colours permanently.
 * @param font is the font to modify.
 * @param i is the ink colour.
 * @param p is the paper colour.
 * This function offers a quicker alternative to colouring text as it is
 * printed, and may be used if ALL text is going to be a certain colour.
 * It can be used in conjunction with the fnt_copy() function to quickly
 * output text in multiple colours (fnt_copy then apply fnt_colours).
 * Note that this assumes that the colours are already ink 3, paper 0.
 * After changing the font colours, this will no longer be the case
 * and further colour changes will have unpredictable results.
 */
void fnt_colours (Font *font, int i, int p);

/**
 * Destroy a font.
 * @param font is the font to destroy.
 */
void fnt_destroy (Font *font);

#endif
