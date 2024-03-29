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
 * @param step    Step of increase/decrease.
 */
void getnum (int *address, int x, int y, int min, int max, int step)
{
    int key; /* keypress */
    char numstr[6], /* number expressed as a string */
	*format; /* number format */

    /* determine number format based on maximum value */
    if (max < 10)
	format = "%1d";
    else if (max < 100)
	format = "%02d";
    else if (max < 1000)
	format = "%03d";
    else if (max < 10000)
	format = "%04d";
    else
	format = "%05d";

    /* main entry loop */
    scr_ink (scr, 3);
    do {
	sprintf (numstr, format, *address);
	scr_print (scr, x, y, numstr);
	key = getch ();
	if (key == 0) key = -getch ();
	if (key == -72 && *address < max)
	    *address += step;
	else if (key == -80 && *address > min)
	    *address -= step;
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


/**
 * Perform a vertical flip
 */
static void verticalflip (void)
{
    Bitmap *bitmap; /* a pointer to the bitmap */
    int r, /* row counter */
	c, /* column counter */
	t, /* index of top byte to swap */
	b; /* index of bottom byte to swap */
    char p; /* temporary swap storage */
    bitmap = bitmaps[bcursor];
    for (r = 0; r < bitmap->height / 2; ++r)
	for (c = 0; c < bitmap->width / 4; ++c) {
	    t = (r * bitmap->width / 4) + c;
	    b = ((bitmap->height - 1 - r) * bitmap->width / 4) + c;
	    p = bitmap->pixels[t];
	    bitmap->pixels[t] = bitmap->pixels[b];
	    bitmap->pixels[b] = p;
	}
}

/**
 * Perform a horizontal flip
 */
static void horizontalflip (void)
{
    Bitmap *bitmap; /* a pointer to the bitmap */
    int r, /* row counter */
	c, /* column counter */
	l, /* index of left byte to swap */
	i, /* index of right byte to swap */
	b; /* bit counter */
    char l1, /* original left byte */
	l2, /* revised left byte */
	r1, /* original right byte */
	r2; /* revised right byte */
    bitmap = bitmaps[bcursor];
    for (r = 0; r < bitmap->height; ++r)
	for (c = 0; c < (bitmap->width + 4) / 8; ++c) {
	    l = (r * bitmap->width / 4) + c;
	    i = (r * bitmap->width / 4) + (bitmap->width / 4 - 1) - c;
	    l1 = bitmap->pixels[l];
	    l2 = 0;
	    for (b = 0; b < 7; b += 2) {
		l2 = (l2 << 2) | (l1 & 3);
		l1 >>= 2;
	    }
	    r1 = bitmap->pixels[i];
	    r2 = 0;
	    for (b = 0; b < 7; b += 2) {
		r2 = (r2 << 2) | (r1 & 3);
		r1 >>= 2;
	    }
	    bitmap->pixels[l] = r2;
	    bitmap->pixels[i] = l2;
	}
}

/**
 * Perform a diagonal flip - halfway to a rotate.
 */
static void diagonalflip (void)
{
    int w, /* width of bitmap */
	x, /* x counter */
	y, /* y counter */
	p, /* temporary pixel value */
	p1, /* pixel value lower left */
	p2; /* pixel value upper right */
    w = bitmaps[bcursor]->width;
    for (x = 0; x < w; ++x)
	for (y = 0; y < x; ++y) {
	    p = bitmaps[bcursor]->pixels[(x + y * w) / 4];
	    p1 = (p >> (2 * (3 - (x % 4)))) & 3;
	    p = bitmaps[bcursor]->pixels[(y + x * w) / 4];
	    p2 = (p >> (2 * (3 - (y % 4)))) & 3;
	    plot (x, y, p2);
	    plot (y, x, p1);
	}
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
    scr_font (scr, fnt);

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
 * Insert a new bitmap.
 */
static void insert (void)
{
    int w, /* width of new bitmap */
	h; /* height of new bitmap */

    /* get the height and width of the bitmap */
    w = 16;
    h = 16;
    scr_ink (scr, 3);
    scr_print (scr, 0, 192, "Bitmap size: ..x..");
    getnum (&w, 52, 192, 4, 24, 4);
    getnum (&h, 64, 192, 2, 24, 2);

    /* create and clear the bitmap */
    bitmaps[bcursor] = bit_create (w, h);
    clear ();

    /* update the screen */
    showbitmap (bcursor);
    expandbitmap ();
    scr_ink (scr, 0);
    scr_box (scr, 0, 192, 320, 8);
    scr_ink (scr, 3);
}

/**
 * Vertical flip the current bitmap and show it.
 */
static void showverticalflip (void)
{
    verticalflip ();
    showbitmap (bcursor);
    expandbitmap ();
}

/**
 * Horizontal flip the current bitmap and show it.
 */
static void showhorizontalflip (void)
{
    horizontalflip ();
    showbitmap (bcursor);
    expandbitmap ();
}

/**
 * Save the bitmaps.
 */
static void savebitmaps (void)
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
	    strcat (filename, ".bit");
    }
    if (! *filename) return;

    /* save the file */
    if (! (fp = fopen (filename, "wb")))
	return;
    fwrite ("CGA100B", 8, 1, fp);
    for (c = 0; c < 24; ++c)
	if (bitmaps[c])
	    bit_write (bitmaps[c], fp);
    fclose (fp);
}

/**
 * Fill the current bitmap with its ink colour.
 */
static void fill (void)
{
    bit_box
	(bitmaps[bcursor], 0, 0, bitmaps[bcursor]->width,
	 bitmaps[bcursor]->height);
    showbitmap (bcursor);
    expandbitmap ();
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
    else if (key == -73 && bcursor > 0) {
	showbitmap (bcursor--);
	showbitmap (bcursor);
	expandbitmap ();
    }

    /* page down (next bitmap) */
    else if (key == -81 && bcursor < 23) {
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

    /* V - vertical flip */
    else if (bitmaps[bcursor] && toupper (key) == 'V')
	showverticalflip ();

    /* H - horizontal flip */
    else if (bitmaps[bcursor] && toupper (key) == 'H')
	showhorizontalflip ();

    /* R - rotate */
    else if (bitmaps[bcursor]
	     && bitmaps[bcursor]->width == bitmaps[bcursor]->height
	     && key == 'r') {
	diagonalflip ();
	showhorizontalflip ();
    } else if (bitmaps[bcursor] 
	     && bitmaps[bcursor]->width == bitmaps[bcursor]->height
	     && key == 'R') {
	diagonalflip ();
	showverticalflip ();
    }

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

    /* F - fill */
    else if (bitmaps[bcursor] && toupper (key) == 'F')
	fill ();

    /* INS - insert new bitmap */
    else if (! bitmaps[bcursor] && key == -82)
	insert ();

    /* DEL - delete bitmap */
    else if (bitmaps[bcursor] && key == -83) {
	bit_destroy (bitmaps[bcursor]);
	bitmaps[bcursor] = NULL;
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
    savebitmaps ();
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
