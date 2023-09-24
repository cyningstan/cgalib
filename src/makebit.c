/*======================================================================
 * CGALib - Watcom C Version.
 * Bitmap Editor Program.
 * 
 * Released as Public Domain by Damian Gareth Walker, 2020.
 * Created 29-Jun-2020.
 */

/*----------------------------------------------------------------------
 * Required Headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* OS specific headers */
#include <conio.h>
#include <i86.h>

/* project headers */
#include "cgalib.h"

/*----------------------------------------------------------------------
 * File Level Variables.
 */

/** @var scr The screen object. */
static Screen *scr = NULL;

/** @var fnt The font object. */
static Font *fnt = NULL;

/** @var bits The program's bitmaps. */
static Bitmap *bits[9];

/** @var bitmaps The edited bitmaps. */
static Bitmap *bitmaps[24];

/** @var bcursor The bitmap selector cursor position. */
static int bcursor;

/** @var xcursor The bitmap editor X cursor position. */
static int xcursor;

/** @var ycursor The bitmap editor Y cursor position. */
static int ycursor;

/** @var mono 1 if the user wants a monochrome screen. */
static int mono;

/** @var filename The filename */
static char filename[128];

/*----------------------------------------------------------------------
 * Service Routines.
 */

/**
 * Error Handler.
 * @param errorlevel is the error level to return to the OS.
 * @param message is the message to print.
 */
void error_handler (int errorlevel, char *message)
{
    if (scr)
        scr_destroy (scr);
    puts (message);
    exit (errorlevel);
}

/*----------------------------------------------------------------------
 * Level 4 Routines
 */

/**
 * Expand a single pixel on the editor grid.
 * @param bitnum The bitmap number.
 * @param x      The x position.
 * @param y      The y position.
 */
static void expandpixel (int x, int y)
{
    int w, /* pointer to bitmap width */
	h, /* pointer to bitmap height */
	c, /* colour of pixel */
	xx, /* x position of enlarged pixel on screen */
	yy; /* y position of enlarged pixel on screen */
    char p; /* pointer to pixel block */
    w = bitmaps[bcursor]->width;
    h = bitmaps[bcursor]->height;
    xx = 8 * (12 - w / 2) + 8 * x;
    yy = 8 * (12 - h / 2) + 8 * y;
    p = bitmaps[bcursor]->pixels[(x + y * w) / 4];
    c = (p >> (2 * (3 - (x % 4)))) & 3;
    if (x == xcursor && y == ycursor)
	scr_put (scr, bits[4 + c], xx, yy, DRAW_PSET);
    else
	scr_put (scr, bits[c], xx, yy, DRAW_PSET);
}

/*----------------------------------------------------------------------
 * Level 3 Routines
 */

/**
 * Load and validate a font.
 * @param filename is the name of the font file.
 * @returns the loaded font.
 * The font file loaded by this function requires an 8-byte header,
 * consisting of the text CGA100F and a null byte. Then follows two
 * bytes determining the first and last character codes supported by
 * the font. After that is the font pixel data.
 */
static Font *load_font (char *filename)
{
    Font *font; /* loaded font */
    FILE *fp; /* file pointer */
    char header[8]; /* font file header */

    /* attempt to open the file, and read and verify the header */
    if (! (fp = fopen (filename, "rb")))
        return NULL;
    else if (! fread (header, 8, 1, fp)) {
        fclose (fp);
        return NULL;
    } else if (strcmp (header, "CGA100F")) {
        fclose (fp);
        return NULL;
    }

    /* read the font and return it */
    font = fnt_read (fp);
    fclose (fp);
    return font;
}

/**
 * Load the bitmaps
 * @param filename is the bimap file.
 */
static int load_bitmaps (Bitmap **bitmaps, char *filename)
{
    FILE *fp; /* file pointer */
    char header[8]; /* bitmap file header */
    int c; /* counter */
    Bitmap *bit; /* pointer to temporary bitmap */

    /* attempt to open the file, and read and verify the header */
    if (! (fp = fopen (filename, "rb")))
        return 0;
    else if (! fread (header, 8, 1, fp)) {
        fclose (fp);
        return 0;
    } else if (strcmp (header, "CGA100B")) {
        fclose (fp);
        return 0;
    }

    /* read the bitmaps */
    c = 0;
    while (c < 24 && (bit = bit_read (fp))) {
	bitmaps[c] = bit;
	++c;
    }
    while (c < 24)
	bitmaps[c++] = NULL;

    /* return */
    return 1;
}

/**
 * Expand a bitmap onto the editor display.
 * @param bitnum Bitmap number.
 */
static void expandbitmap (void)
{
    int w, /* pointer to bitmap width */
	h, /* pointer to bitmap height */
	x, /* x pixel position */
	y; /* y pixel position */
    scr_ink (scr, 0);
    scr_box (scr, 0, 0, 192, 192);
    scr_ink (scr, 3);
    if (bitmaps[bcursor]) {
	w = bitmaps[bcursor]->width;
	h = bitmaps[bcursor]->height;
	for (x = 0; x < w; ++x)
	    for (y = 0; y < h; ++y)
		expandpixel (x, y);
    }
}

/**
 * Show a bitmap in the side panel.
 */
static void showbitmap (int b) {
    scr_ink (scr, 0);
    scr_box (scr, 192 + 32 * (b & 3), 32 * (b / 4), 32, 32);
    scr_ink (scr, 3);
    if (bitmaps[b]) {
	scr_put
	    (scr, bitmaps[b],
	     192 + (16 - bitmaps[b]->width / 2) + 32 * (b & 3),
	     (16 - bitmaps[b]->height / 2) + 32 * (b / 4),
	     DRAW_PSET);
    }
    if (b == bcursor)
	scr_put
	    (scr, bits[8], 192 + 32 * (b & 3), 32 * (b / 4),
	     DRAW_OR);
}

/*----------------------------------------------------------------------
 * Level 2 Routines.
 */

static void initialise_args (int argc, char **argv)
{
    while (argc-- > 1)
	if (! stricmp (argv[argc], "-m"))
	    mono = 1;
	else
	    strcpy (filename, argv[argc]);
}

/**
 * Initialise the screen.
 * @param mono is true if mono mode was requested.
 */
void initialise_screen (int mono)
{
    int b; /* bitmap counter */

    /* initialise screen and assets */
    if (! (scr = scr_create (mono ? 6 : 4)))
        error_handler (1, "Cannot initialise graphics mode!");
    if (! (fnt = load_font ("fnt/present.fnt")))
        error_handler (1, "Cannot load font");
    if (! (load_bitmaps (bits, "bit/makebit.bit")))
	error_handler (1, "Cannot load program bitmaps");

    /* initial screen display */
    if (bitmaps[bcursor])
	expandbitmap ();
    for (b = 0; b < 24; ++b)
	showbitmap (b);
}

/**
 * Change a pixel in the current bitmap.
 * @param colour The colour to paint the pixel
 */
static void changepixel (int colour)
{
    int shift, /* number of bits to shift */
	mask; /* bit mask */
    char p; /* pixel group */

    /* change the bitmap */
    shift = (2 * (3 - (xcursor % 4)));
    mask = 0xff ^ (3 << shift);
    p = bitmaps[bcursor]->pixels
	[(xcursor + ycursor * bitmaps[bcursor]->width) / 4];
    p &= mask;
    p |= colour << shift;
    bitmaps[bcursor]->pixels
	[(xcursor + ycursor * bitmaps[bcursor]->width) / 4] = p;

    /* update the bitmap and the pixel */
    showbitmap (bcursor);
    expandpixel (xcursor, ycursor);
}

/*----------------------------------------------------------------------
 * Level 1 Routines.
 */

/**
 * Convert the source .scr to the program bitmaps.
 */
static void convert (void)
{
    Screen *screen;
    FILE *fp;
    char header[8];
    char *bytes;
    int b;
    Bitmap *bitmap;

    /* initialise the screen */
    if (! (screen = scr_create (4))) {
        printf ("Cannot initialise graphics mode!\n");
        exit (1);
    }

    /* load a screen and display it */
    if (! (fp = fopen ("pic/makebit.pic", "rb"))) {
        printf ("Cannot load %s.\n", "pic/makebit.pic");
        exit (1);
    }
    if (! fread (header, 7, 1, fp)) {
        printf ("Cannot read header from %s.\n", "pic/makebit.pic");
        exit (1);
    }
    if (! (bytes = malloc (16192))) {
        printf ("Cannot reserve memory for screen.\n");
        exit (1);
    }
    if (! fread (bytes, 16192, 1, fp)) {
        printf ("Cannot read bytes from %s.\n", "pic/makebit.pic");
        exit (1);
    }
    fclose (fp);
    _fmemcpy ((char far *) 0xb8000000, bytes, 16192);

    if (! (fp = fopen ("bit/makebit.bit", "wb"))) {
        printf ("Cannot create bitmap file %s.\n", "bit/makebit.bit");
        exit (1);
    }
    if (! fwrite ("CGA100B", 8, 1, fp)) {
        printf ("Cannot write header to bitmap file.\n");
        exit (1);
    }
    for (b = 0; b < 8; ++b) {
	if (! (bitmap = bit_create (8, 8))) {
	    printf ("Cannot create bitmap.\n");
	    exit (1);
	}
        scr_get (screen, bitmap, 8 * b, 0);
        bit_write (bitmap, fp);
	bit_destroy (bitmap);
    }
    if (! (bitmap = bit_create (32, 32))) {
	printf ("Cannot create bitmap.\n");
	exit (1);
    }
    scr_get (screen, bitmap, 0, 8);
    bit_write (bitmap, fp);
    bit_destroy (bitmap);
    fclose (fp);

    /* clean up at the end */
    free (bytes);
    bit_destroy (bitmap);
    scr_destroy (screen);
}

/**
 * Initialise a game.
 * @param mono 1 for monochrome, 0 for colour.
 * @return     1 if successful, 0 on failure.
 */
static int initialise (int argc, char **argv)
{
    initialise_args (argc, argv);
    if (*filename && ! load_bitmaps (bitmaps, filename))
	error_handler (2, "Cannot load bitmaps");
    initialise_screen (mono);
    return 1;
}

/**
 * Main program loop.
 * @return 0 when finished, 1 to continue.
 */
static int main_program (void)
{
    int key; /* key press */

    /* get keypress */
    key = getch ();
    if (key == 0) key = -getch ();

    /* cursor left */
    if (bitmaps[bcursor]
	&& key == -75 && xcursor > 0) {
	expandpixel (xcursor--, ycursor);
	expandpixel (xcursor, ycursor);
    }

    /* cursor right */
    else if (bitmaps[bcursor]
	       && key == -77
	       && xcursor < bitmaps[bcursor]->width - 1) {
	expandpixel (xcursor++, ycursor);
	expandpixel (xcursor, ycursor);
    }

    /* cursor up */
    else if (bitmaps[bcursor]
	       && key == -72 && ycursor > 0) {
	expandpixel (xcursor, ycursor--);
	expandpixel (xcursor, ycursor);
    }

    /* cursor down */
    else if (bitmaps[bcursor]
	       && key == -80
	       && ycursor < bitmaps[bcursor]->height - 1) {
	expandpixel (xcursor, ycursor++);
	expandpixel (xcursor, ycursor);
    }

    /* page up (previous bitmap) */
    else if (key == -73
	&& bcursor > 0) {
	showbitmap (bcursor--);
	showbitmap (bcursor);
	expandbitmap ();
    }

    /* page down (next bitmap) */
    else if (key == -81
	&& bcursor < 23) {
	showbitmap (bcursor++);
	showbitmap (bcursor);
	expandbitmap ();
    }

    /* 0..3 change pixel colour */
    else if (bitmaps[bcursor] && key >= '0' && key <= '3') {
	changepixel (key - '0');
    }

    /* ESC (quit) */
    else if (key == 27) {
	return 0;
    }

    /* return */
    return 1;
}

/**
 * End the program.
 */
static void end_program (void)
{
    int c; /* counter */
    for (c = 0; c < 24; ++c)
	bit_destroy (bitmaps[c]);
    for (c = 0; c < 9; ++c)
	bit_destroy (bits[c]);
    scr_destroy (scr);
}

/*----------------------------------------------------------------------
 * Top Level Routine.
 */

/**
 * Main program.
 * @param argc is the number of command line argumets.
 * @param argv is the command line arguments.
 * No return value as exit () is used to terminate abnormally.
 */
void main (int argc, char **argv)
{
    if (argc == 2 && ! strcmp (argv[1], "-b"))
	convert ();
    else if (initialise (argc, argv)) {
	while (main_program ());
	end_program ();
    }
}
