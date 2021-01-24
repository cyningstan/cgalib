/*======================================================================
 * CGALib - Watcom C Version.
 * Font Module.
 *
 * Released as Public Domain by Damian Gareth Walker, 2020.
 * Created 29-Jun-2020.
 */

/* define CGALIB macro for access to internal structures */
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
 * Create a new font.
 * @param first is the first character in the font.
 * @param last is the last character in the font.
 * @returns the new font.
 */
Font *fnt_create (int first, int last)
{
    /* local variables */
    Font *font; /* the font to return */

    /* attempt to allocate memory */
    if (! (font = malloc (sizeof (Font))))
        return NULL;
    if (! (font->pixels = malloc (8 * (last - first + 1)))) {
        free (font);
        return NULL;
    }

    /* set the font information */
    font->first = first;
    font->last = last;

    /* return the font */
    return font;
}

/**
 * Copy a font.
 * @param font is the font to copy.
 * @returns a new font copied from the old.
 */
Font *fnt_copy (Font *src)
{
    /* local variables */
    Font *dst; /* destination font */

    /* attempt to reserve memory */
    if (! (dst = malloc (sizeof (Font))))
        return NULL;
    if (! (dst->pixels = malloc (8 * (src->last - src->first + 1)))) {
        free (dst);
        return NULL;
    }

    /* set the font information */
    dst->first = src->first;
    dst->last = src->last;
    memcpy (dst->pixels, src->pixels, 8 * (src->last - src->first + 1));

    /* return the font */
    return dst;
}

/**
 * Read a font from an already open file.
 * @param input is the input file handle.
 * @returns the new font.
 */
Font *fnt_read (FILE *input)
{
    /* local variables */
    Font *font; /* the font to return */
    char f; /* first character code read from file */
    char l; /* last character code read from file */

    /* read first and last character codes */
    if (! fread (&f, 1, 1, input))
        return NULL;
    if (! fread (&l, 1, 1, input))
        return NULL;

    /* attempt to allocate memory */
    if (! (font = malloc (sizeof (Font))))
        return NULL;
    if (! (font->pixels = malloc (8 * (l - f + 1)))) {
        free (font);
        return NULL;
    }

    /* attempt to read the pixel data */
    if (! fread (font->pixels, 8 * (l - f + 1), 1, input)) {
        free (font->pixels);
        free (font);
        return NULL;
    }

    /* set the other font information */
    font->first = f;
    font->last = l;

    /* return the font */
    return font;
}

/**
 * Write a font to an already open file.
 * @param font is the font to write.
 * @param output is the output file handle.
 */
void fnt_write (Font *font, FILE *output)
{
    if (! fwrite (&font->first, 1, 1, output))
        return;
    if (! fwrite (&font->last, 1, 1, output))
        return;
    if (! fwrite (font->pixels, 8 * (font->last - font->first + 1), 1,
        output))
        return;
}

/**
 * Put a bitmap into a font character.
 * @param dst is the destination font.
 * @param src is the source bitmap.
 * @param ch is the character to set.
 */
void fnt_put (Font *dst, Bitmap *src, int ch)
{
    memcpy (dst->pixels + 8 * (ch - dst->first), src->pixels, 8);
}

/**
 * Get a bitmap from a font character.
 * @param src is the source font.
 * @param dst is the destination bitmap.
 * @param ch is the character to get.
 */
void fnt_get (Font *src, Bitmap *dst, int ch)
{
    memcpy (dst->pixels, src->pixels + 8 * (ch - src->first), 8);
}

/**
 * Set the font colours permanently.
 * @param font is the font to modify.
 * @param ink is the ink colour.
 * @param paper is the paper colour.
 * This function offers a quicker alternative to colouring text as it is
 * printed, and may be used if ALL text is going to be a certain colour.
 * It can be used in conjunction with the fnt_copy() function to quickly
 * output text in multiple colours (fnt_copy then apply fnt_colours).
 * Note that this assumes that the colours are already ink 3, paper 0.
 * After changing the font colours, this will no longer be the case
 * and further colour changes will have unpredictable results.
 */
void fnt_colours (Font *font, int ink, int paper)
{
    /* local variables */
    int ch; /* character count */
    int r; /* row count */
    char p; /* paper mask */
    char i; /* ink mask */
    char b; /* background of recoloured character */
    char f; /* foreground of recoloured character */

    /* work out the ink and paper masks */
    i = ink * 0x55;
    p = paper * 0x55;

    /* loop through all the characters */
    for (ch = 0; ch < font->last - font->first; ++ch)
        for (r = 0; r < 8; ++r) {
            f = font->pixels[8 * ch + r] & i;
            b = (0xff ^ font->pixels[8 * ch + r]) & p;
            font->pixels[8 * ch + r] = f | b;
        }
}

/**
 * Destroy a font.
 * @param font is the font to destroy.
 */
void fnt_destroy (Font *font)
{
    if (font) {
        if (font->pixels)
            free (font->pixels);
        free (font);
    }
}
