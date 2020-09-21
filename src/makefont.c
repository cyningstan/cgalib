/*======================================================================
 * CGALib - Watcom C Version.
 * Font maker utility.
 * 
 * Extracts a 96-character ASCII font from a PIC file. Currently the
 * characters must be 4x8 pixels and in 6 rows of 16 at the top left
 * of the image.
 * 
 * Released as Public Domain by Damian Gareth Walker, 2020.
 * Created 29-Jun-2020.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "cgalib.h"

void main (int argc, char **argv)
{
    Screen *screen;
    FILE *fp;
    char header[8];
    char *bytes;
    Font *font;
    int ch, r, c;
    Bitmap *bitmap;

    /* check the parameters */
    if (argc != 3) {
        printf ("Usage: %s INFILE.PIC OUTFILE.FNT\n", argv[0]);
        exit (0);
    }

    /* initialise the screen */
    if (! (screen = scr_create (4))) {
        printf ("Cannot initialise graphics mode!\n");
        exit (1);
    }

    /* load a screen and display it */
    if (! (fp = fopen (argv[1], "rb"))) {
        printf ("Cannot load %s.\n", argv[1]);
        exit (1);
    }
    if (! fread (header, 7, 1, fp)) {
        printf ("Cannot read header from %s.\n", argv[1]);
        exit (1);
    }
    if (! (bytes = malloc (16192))) {
        printf ("Cannot reserve memory for screen.\n");
        exit (1);
    }
    if (! fread (bytes, 16192, 1, fp)) {
        printf ("Cannot read bytes from %s.\n", argv[1]);
        exit (1);
    }
    fclose (fp);
    _fmemcpy ((char far *) 0xb8000000, bytes, 16192);
    getch ();

    /* extract all the characters into a font */
    if (! (font = fnt_create (32, 127))) {
        printf ("Cannot create font.\n");
        exit (1);
    }
    if (! (bitmap = bit_create (4, 8))) {
        printf ("Cannot create bitmap.\n");
        exit (1);
    }
    if (! (fp = fopen (argv[2], "wb"))) {
        printf ("Cannot create font file %s.\n", argv[2]);
        exit (1);
    }
    if (! fwrite ("CGA100F", 8, 1, fp)) {
        printf ("Cannot write header to font file.\n");
        exit (1);
    }
    for (ch = 32; ch <= 127; ++ch) {
        r = (ch - 32) / 16;
        c = (ch - 32) % 16;
        scr_get (screen, bitmap, 4 * c, 8 * r);
        fnt_put (font, bitmap, ch);
    }
    fnt_write (font, fp);
    fclose (fp);

    /* clean up at the end */
    free (bytes);
    bit_destroy (bitmap);
    fnt_destroy (font);
    scr_destroy (screen);
}
