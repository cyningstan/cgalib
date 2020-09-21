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
    int b;
    Bitmap *bitmap;

    /* check the parameters */
    if (argc != 3) {
        printf ("Usage: %s INFILE.PIC OUTFILE.BIT\n", argv[0]);
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

    if (! (bitmap = bit_create (16, 16))) {
        printf ("Cannot create bitmap.\n");
        exit (1);
    }
    if (! (fp = fopen (argv[2], "wb"))) {
        printf ("Cannot create bitmap file %s.\n", argv[2]);
        exit (1);
    }
    if (! fwrite ("CGA100B", 8, 1, fp)) {
        printf ("Cannot write header to bitmap file.\n");
        exit (1);
    }
    for (b = 0; b < 20; ++b) {
        scr_get (screen, bitmap, 16 * b, 0);
        bit_write (bitmap, fp);
    }
    fclose (fp);

    /* clean up at the end */
    free (bytes);
    bit_destroy (bitmap);
    scr_destroy (screen);
}
