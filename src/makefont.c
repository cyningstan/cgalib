/*======================================================================
 * CGALib - Watcom C Version.
 * Font Editor Program.
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
#include <ctype.h>

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

/** @var fnt The font used for the program itself. */
static Font *fnt = NULL;

/** @var editfont The font being edited. */
static Font *editfont = NULL;

/** @var bits The program's bitmaps. */
static Bitmap *bits[9];

/** @var bitmaps The edited font bitmaps. */
static Bitmap *bitmaps[256];

/** @var bcursor The bitmap selector cursor position. */
static int bcursor;

/** @var xcursor The bitmap editor X cursor position. */
static int xcursor;

/** @var ycursor The bitmap editor Y cursor position. */
static int ycursor;

/** @var clipboard The bitmap most recently copied. */
static int clipboard;

/** @var mono 1 if the user wants a monochrome screen. */
static int mono;

/** @var filename The filename. */
static char filename[128];

/** @var palette The current palette. */
static int palette = 4;

/** @var background The current background colour. */
static int background = 0;

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

/**
 * Get a number.
 * @param address Pointer to the number store.
 * @param x       X location on the screen.
 * @param y       Y location on the screen.
 * @param min     Minimum value.
 * @param max     Maximum value.
 */
void getnum (int *address, int x, int y, int min, int max)
{
    int key; /* keypress */
    char numstr[4]; /* number expressed as a string */
    scr_ink (scr, 3);
    do {
	sprintf (numstr, "%03d", *address);
	scr_print (scr, x, y, numstr);
	key = getch ();
	if (key == 0) key = -getch ();
	if (key == -72 && *address < max)
	    ++*address;
	else if (key == -80 && *address > min)
	    --*address;
    } while (key != 13);
}

/**
 * Plot a single pixel.
 * @param x X location
 * @param y Y pixel location
 * @param colour Colour of pixel
 */
static void plot (int x, int y, int colour)
{
    int shift; /* number of bits to shift */
    char mask, /* AND mask for 4-pixel byte */
	p; /* pixel value */
    shift = (2 * (3 - (x % 4)));
    mask = 0xff ^ (3 << shift);
    p = bitmaps[bcursor]->pixels
	[(x + y * bitmaps[bcursor]->width) / 4];
    p &= mask;
    p |= colour << shift;
    bitmaps[bcursor]->pixels
	[(x + y * bitmaps[bcursor]->width) / 4] = p;
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
    xx = 24 * x;
    yy = 24 * y;
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
 * Get font range.
 * @param first Pointer to first character code.
 * @param last  Pointer to last character code.
 * @param fmin  Mininmum first character code.
 * @parma fmax  Maximum first character code.
 * @param lmin  Mininmum last character code.
 * @parma lmax  Maximum last character code.
 */
void getfontrange
(int *first, int *last, int fmin, int fmax, int lmin, int lmax)
{
    scr_ink (scr, 3);
    scr_print (scr, 0, 192, "Character codes: ...-...");
    getnum (first, 68, 192, fmin, fmax);
    getnum (last, 84, 192, *first > lmin ? *first : lmin, lmax);
    scr_ink (scr, 0);
    scr_box (scr, 0, 192, 320, 8);
    scr_ink (scr, 3);
}

/**
 * Load and validate a font.
 * @param filename is the name of the font file.
 * @returns the loaded font.
 * The font file loaded by this function requires an 8-byte header,
 * consisting of the text CGA100F and a null byte. Then follows two
 * bytes determining the first and last character codes supported by
 * the font. After that is the font pixel data.
 */
static Font *load_font (char *filename, int mkbitmaps)
{
    Font *font; /* loaded font */
    FILE *fp; /* file pointer */
    char header[8]; /* font file header */
    int c; /* character counter */

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

    /* convert to bitmaps */
    if (mkbitmaps)
	for (c = 0; c <= 255; ++c) {
	    if (bitmaps[c]) {
		bit_destroy (bitmaps[c]);
		bitmaps[c] = NULL;
	    }
	    if (c >= font->first && c <= font->last) {
		bitmaps[c] = bit_create (4, 8);
		fnt_get (font, bitmaps[c], c);
	    }
	}

    /* return the loaded font */
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
    scr_box (scr, 0, 0, 96, 192);
    scr_ink (scr, 3);
    if (bitmaps[bcursor]) {
	w = bitmaps[bcursor]->width;
	h = bitmaps[bcursor]->height;
	if (xcursor >= w) xcursor = w - 1;
	if (ycursor >= h) ycursor = h - 1;
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
    scr_box (scr, 128 + 12 * (b & 15), 12 * (b / 16), 12, 12);
    scr_ink (scr, 3);
    if (bitmaps[b]) {
	scr_put
	    (scr, bitmaps[b],
	     132 + 12 * (b & 15),
	     2 + 12 * (b / 16),
	     DRAW_PSET);
    }
    if (b == bcursor)
	scr_put
	    (scr, bits[8], 128 + 12 * (b & 15), 12 * (b / 16),
	     DRAW_OR);
}

/**
 * Clear the current bitmap.
 */
static void clear (void)
{
    bit_ink (bitmaps[bcursor], 0);
    bit_box
	(bitmaps[bcursor], 0, 0, bitmaps[bcursor]->width,
	 bitmaps[bcursor]->height);
    bit_ink (bitmaps[bcursor], 3);
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
 * Create a new font to load.
 */
void newfont (void)
{
    int f, /* first character in font */
	l, /* last character in font */
	c; /* character counter */

    /* get the range of characters */
    f = 32;
    l = 127;
    getfontrange (&f, &l, 0, 255, 0, 255);

    /* create and clear the font */
    editfont = fnt_create (f, l);
    for (c = 0; c <= 255; ++c) {
	if (bitmaps[c])
	    bit_destroy (bitmaps[c]);
	bitmaps[c] = NULL;
	if (c >= editfont->first && c <= editfont->last) {
	    bitmaps[c] = bit_create (4, 8);
	    bit_ink (bitmaps[c], 0);
	    bit_box (bitmaps[c], 0, 0, 4, 8);
	    bit_ink (bitmaps[c], 3);
	}
    }

    /* update the screen */
    bcursor = editfont->first;
}

/**
 * Initialise the screen.
 * @param mono is true if mono mode was requested.
 */
void initialise_screen (int mono)
{
    /* initialise screen and assets */
    if (! (scr = scr_create (mono ? 6 : 4)))
        error_handler (1, "Cannot initialise graphics mode!");
    if (! (fnt = load_font ("fnt/present.fnt", 0)))
        error_handler (1, "Cannot load program font");
    if (! (load_bitmaps (bits, "bit/makefont.bit")))
	error_handler (1, "Cannot load program bitmaps");
    scr_font (scr, fnt);
}

/**
 * Change a pixel in the current bitmap.
 * @param colour The colour to paint the pixel
 */
static void changepixel (int colour)
{
    bit_ink (bitmaps[bcursor], colour);
    plot (xcursor, ycursor, colour);
    showbitmap (bcursor);
    expandpixel (xcursor, ycursor);
}

/**
 * Paste the copied bitmap.
 */
static void paste (void)
{
    if (bitmaps[bcursor])
	bit_destroy (bitmaps[bcursor]);
    bitmaps[bcursor] = bit_copy (bitmaps[clipboard]);
    showbitmap (bcursor);
    expandbitmap ();
}

/**
 * Save the bitmaps.
 */
static void save_font (void)
{
    int key, /* keypress */
	len, /* length of string */
	c; /* bitmap counter */
    FILE *fp; /* file pointer */

    /* get the filename */
    if (! *filename) {
	scr_print (scr, 0, 192, "Filename: ");
	do {
	    key = getch ();
	    if (key >= ' ' && key <= '~') {
		len = strlen(filename);
		filename[len] = key;
		filename[len + 1] = '\0';
		scr_print (scr, 40, 192, filename);
	    } else if (key == 8 && *filename) {
		len = strlen(filename);
		filename[len - 1] = '\0';
		len = strlen(filename);
		scr_print (scr, 40 + 4 * len, 192, " ");
	    }
	} while (key != 13);
	if (*filename && ! strchr (filename, '.'))
	    strcat (filename, ".fnt");
    }
    if (! *filename) return;

    /* convert bitmaps to font */
    for (c = editfont->first; c <= editfont->last; ++c)
	fnt_put (editfont, bitmaps[c], c);

    /* save the file */
    if (! (fp = fopen (filename, "wb")))
	return;
    fwrite ("CGA100F", 8, 1, fp);
    fnt_write (editfont, fp);
    fclose (fp);
}

/*----------------------------------------------------------------------
 * Level 1 Routines.
 */

/**
 * Initialise the program.
 * @param mono 1 for monochrome, 0 for colour.
 * @return     1 if successful, 0 on failure.
 */
static int initialise (int argc, char **argv)
{
    int b; /* bitmap counter */

    initialise_args (argc, argv);
    initialise_screen (mono);
    if (*filename && ! (editfont = load_font (filename, 1)))
	error_handler (2, "Cannot load font");
    else if (! *filename)
	newfont ();

    /* initial screen display */
    bcursor = editfont->first;
    if (bitmaps[bcursor])
	expandbitmap ();
    for (b = 0; b <= 255; ++b)
	showbitmap (b);
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
    else if (key == -73 && bcursor > editfont->first) {
	showbitmap (bcursor--);
	showbitmap (bcursor);
	expandbitmap ();
    }

    /* page down (next bitmap) */
    else if (key == -81 && bcursor < editfont->last) {
	showbitmap (bcursor++);
	showbitmap (bcursor);
	expandbitmap ();
    }

    /* 0..3 change pixel colour */
    else if (bitmaps[bcursor] && key >= '0' && key <= '3')
	changepixel (key - '0');

    /* SPACE - change pixel colour */
    else if (bitmaps[bcursor] && key == ' ')
	changepixel (bitmaps[bcursor]->ink);

    /* C - copy */
    else if (bitmaps[bcursor] && (key == 'c' || key == 'C'))
	clipboard = bcursor;

    /* P - paste */
    else if (bitmaps[clipboard] && toupper (key) == 'P')
	paste ();

    /* X - clear */
    else if (bitmaps[bcursor] && toupper (key) == 'X') {
	clear ();
	showbitmap (bcursor);
	expandbitmap ();
    }

    /* [ and ] - change palette */
    else if (key == '[' || key == ']') {
	palette += ((key == ']') - (key == '['));
	if (palette < 0) palette = 0;
	if (palette > 5) palette = 5;
	scr_palette (scr, palette, background);
    }

    /* { and } - change background */
    else if (key == '{' || key == '}') {
	background += ((key == '}') - (key == '{'));
	if (background < 0) background = 0;
	if (background > 15) background = 15;
	scr_palette (scr, palette, background);
    }

    /* ESC (quit) */
    else if (key == 27)
	return 0;

    /* return */
    return 1;
}

/**
 * End the program.
 */
static void end_program (void)
{
    int c; /* counter */
    save_font ();
    for (c = 0; c < 255; ++c)
	if (bitmaps[c])
	    bit_destroy (bitmaps[c]);
    for (c = 0; c < 9; ++c)
	bit_destroy (bits[c]);
    fnt_destroy (editfont);
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
    if (initialise (argc, argv)) {
	while (main_program ());
	end_program ();
    }
}
