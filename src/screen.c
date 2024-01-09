/*======================================================================
 * CGALib - Watcom C Version.
 * Screen Handling Module.
 * 
 * Released as Public Domain by Damian Gareth Walker, 2020.
 * Created 29-Jun-2020.
 */

/* define CGALIB for access to internal structures */
#define __CGALIB__

/* headers required for references to data types */
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include "cgalib.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @var base Base address of graphics output. */
static char far *base;

/** @var swidth Physical screen width in bytes. */
static int swidth;

/** @var interleave Number of rows interleaved. */
static int interleave;

/* @var ymult Y coordinate multiplier for calculating pixel address. */
static int ymult;

/* @var ydiv Y coordinate divisor for calculating pixel address. */
static int ydiv;

/*----------------------------------------------------------------------
 * Level 2 Functions.
 */

/**
 * Set the screen parameters according to the screen mode. For some
 * reason, if this is done within the set{,_hercules)_mode functions,
 * the values get reset on leaving. So they're here in a separate
 * function, to be called immediately before trying to update the
 * screen if not already set. That ensures they'll be available at the
 * point they are needed.
 * @param screen The screen data.
 */
void set_screen_parameters (Screen *screen)
{
    /* Hercules screen parameters */
    if (screen->mode == 7) {
	base = (char far *) 0xb0000221;
	swidth = 90;
	interleave = 4;
	ymult = 3;
	ydiv = 2;
    }

    /* CGA/EGA/VGA screen parameters */
    else {
	base = (char far *) 0xb8000000;
	swidth = 80;
	interleave = 2;
	ymult = 1;
	ydiv = 1;
    }
}

/**
 * Set hercules mode.
 */
static void set_hercules_mode (void)
{
    int i; /* index counter */
    char params[12] /* register values */
	= {0x35, 0x2d, 0x2e, 0x07, 0x5b, 0x02,
	   0x57, 0x57, 0x02, 0x03, 0x00, 0x00};

    /* set the mode and deactivate the screen */
    outp (0x3bf, 3);
    outp (0x3b8, 0xa /* 2 */);

    /* set the registers */
    for (i = 0; i < 12; ++i) {
	outp (0x3b4, i);
	outp (0x3b5, params[i]);
    }

    /* clear the screen */
    _fmemset ((char far *) 0xb0000000, 0, 0x7fff);

    /* activate the screen */
    outp (0x3b8, 0xa);
}

/*----------------------------------------------------------------------
 * Level 1 Functions.
 */

/**
 * Attempt to detect Hercules Graphics Card.
 * @return 1 if Hercules, 0 if not.
 */
static int is_hercules (void)
{
    union REGS regs;
    int86 (0x11, &regs, &regs);
    return ((regs.x.ax & 0x30) == 0x30);
}

/**
 * Set the display mode.
 * @params mode is the mode to select.
 */
static void set_mode (Screen *screen)
{
    /* local variables */
    union REGS regs;

    /* set hercules mode */
    if (screen->mode == 7)
	set_hercules_mode ();

    /* set bios-compatible modes */
    else {

	/* use the bios to select the mode */
	regs.h.ah = 0;
	regs.h.al = screen->mode;
	int86 (0x10, &regs, &regs);

    }
}

/**
 * Sets the desired screen colours using the CGA palette register.
 * This works only on actual CGA cards. These palette registers are
 * ignored by EGA and VGA cards, which have independent control of the
 * two or four screen colours.
 */
static void palette_cga (Screen *screen)
{
    /* registers */
    char mode_control;
    char colour_control;

    /* don't do any of this if we're in monochrome mode */
    if (screen->mode != 4 && screen->mode != 5) return;

    /* set the mode according to the palette chosen */
    screen->mode = ((screen->palette % 3) == 2) ? 5 : 4;

    /* initialise the registers */
    mode_control = 0;
    colour_control = 0;

    /* set the mode control register */
    mode_control |= 0x8; /* show display */
    mode_control |= (screen->mode == 5) ? 0x4 : 0; /* greyscale? */
    mode_control |= 0x2; /* graphics mode */

    /* set the colour control register */
    colour_control |= (screen->palette % 3) ? 0x20 : 0; /* palette */
    colour_control |= (screen->palette / 3) ? 0x10 : 0; /* intensity */
    colour_control |= screen->colour; /* colour choice */

    /* set the CGA registers */
    outp (0x3d8, mode_control);
    outp (0x3d9, colour_control);
}

/**
 * Sets the screen colours using the EGA palette registers.
 * Because EGA and VGA cards ignore the CGA palette registers, we need
 * to address these cards directly if we want to see the same colour
 * palette on these cards as we do on a CGA display. This function just
 * simulates the available CGA palettes, it does not support the actual
 * flexibility of a 4-colour EGA/VGA palette.
 */
static void palette_ega (Screen *screen)
{
    union REGS regs;
    static int background[16] = { /* background colours */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
    };
    static int foreground[6][3] = { /* table of 320x200 mode colours */
        {0x02, 0x04, 0x06}, /* mode 4 palette 0 bright 0 */
        {0x03, 0x05, 0x07}, /* mode 4 palette 1 bright 0 */
        {0x03, 0x04, 0x07}, /* mode 5 palette 2 bright 0 */
        {0x3a, 0x3c, 0x3e}, /* mode 4 palette 0 bright 1 */
        {0x3b, 0x3d, 0x3f}, /* mode 4 palette 1 bright 1 */
        {0x3b, 0x3c, 0x3f}, /* mode 5 palette 2 bright 1 */
    };
    int fgcount; /* count of foreground colours */

    /* don't do any of this if we're in monochrome mode */
    if (screen->mode != 4 && screen->mode != 5) return;

    /* background colour */
    regs.w.ax = 0x1000;
    regs.w.bx = 0x100 * background[screen->colour];
    int86 (0x10, &regs, &regs);

    /* foreground colours */
    for (fgcount = 0; fgcount <= 2; ++fgcount) {
        regs.w.ax = 0x1000;
        regs.w.bx = 0x100 * foreground[screen->palette][fgcount]
            + fgcount + 1;
        int86 (0x10, &regs, &regs);
    }
}

/*----------------------------------------------------------------------
 * Public Level Functions.
 */

/**
 * Initialise the screen, and set its initial screen mode.
 * @param mode is the screen mode: 4, 5 or 6.
 * @returns the new screen.
 */
Screen *scr_create (int mode)
{
    /* local variables */
    Screen *screen;

    /* initialise the screen parameters */
    if (! (screen = malloc (sizeof (Screen))))
        return NULL;
    screen->mode = is_hercules () ? 7 : mode;
    screen->palette = (mode == 5) ? 5 : 4;
    screen->colour = 0;
    screen->ink = 3;
    screen->paper = 0;
    screen->font = NULL;

    /* Set the screen mode and palette */
    set_mode (screen);
    palette_cga (screen);
    palette_ega (screen);

    /* return the new screen */
    return screen;
}

/**
 * Set the screen palette.
 * @param screen is the screen to affect.
 * @param p is the palette number, 0..5.
 * @param c is the background colour, 0..15.
 */
void scr_palette (Screen *screen, int palette, int colour)
{
    /* update the palette and colour in the screen data */
    screen->palette = palette;
    screen->colour = colour;

    /* update the palette and colour on the screen */
    palette_cga (screen);
    palette_ega (screen);
}

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
		  int xs, int ys, int w, int h, DrawMode draw)
{
    /* local variables */
    char far *d; /* address to copy data to */
    char *s; /* address to copy data from */
    int r; /* row counter */
    int b; /* byte counter */
    int ydc; /* destination y coordinate */

    /* dst is not used but here for future proofing */
    dst = dst; /* shut the compiler up, hopefully */

    /* ensure the screen parameters are set up */
    if (! ydiv)
	set_screen_parameters (dst);

    /* copy the pixels */
    for (r = 0; r < h; ++r)

        /* DRAW_PSET can be copied by a quicker method */
        if (draw == DRAW_PSET) {
	    ydc = (yd + r) * ymult / ydiv;
	    d = xd / 4
		+ 0x2000 * (ydc % interleave)
		+ swidth * (ydc / interleave)
		+ base;
            s = src->pixels + (xs / 4) + (src->width / 4) * (ys + r);
            _fmemcpy (d, s, w / 4);
        }
        
	/* the other draw operations need doing byte by byte */
        else 
            for (b = 0; b < w / 4; ++b) {
		ydc = (yd + r) * ymult / ydiv;
		d = b
		    + xd / 4
		    + 0x2000 * (ydc % interleave)
		    + swidth * (ydc / interleave)
		    + base;
                s = b + src->pixels + (xs / 4)
		    + (src->width / 4) * (ys + r);
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
 * Put a bitmap onto the screen.
 * @param dst is the screen to affect.
 * @param src is the source bitmap.
 * @param x is the x coordinate at which the bitmap is to be placed.
 * @param y is the y coordinate at which the bitmap is to be placed.
 * @param d is the draw mode.
 */
void scr_put (Screen *dst, Bitmap *src, int x, int y, DrawMode draw)
{
    /* local variables */
    char far *d; /* address to copy data to */
    char *s; /* address to copy data from */
    int r; /* row counter */
    int b; /* byte counter */
    int yd; /* destination y coordinate */

    /* dst is not used but here for future proofing */
    dst = dst; /* shut the compiler up, hopefully */

    /* ensure the screen parameters are set up */
    if (! ydiv)
	set_screen_parameters (dst);

    /* copy the pixels */
    for (r = 0; r < src->height; ++r)

        /* DRAW_PSET can be copied by a quicker method */
        if (draw == DRAW_PSET) {
	    yd = (y + r) * ymult / ydiv;
	    d = x / 4
		+ 0x2000 * (yd % interleave)
		+ swidth * (yd / interleave)
		+ base;
            s = src->pixels + src->width / 4 * r;
            _fmemcpy (d, s, src->width / 4);
        }
        
        /* the other draw operations need doing byte by byte */
        else 
            for (b = 0; b < src->width / 4; ++b) {
		yd = (y + r) * ymult / ydiv;
		d = b + x / 4
		    + 0x2000 * (yd % interleave)
		    + swidth * (yd / interleave)
		    + base;
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
 * Get a bitmap from the screen.
 * @param src is the screen from which to take the bitmap.
 * @param dst is the destination bitmap, whose size is already set.
 * @param x is the x coordinate where the desired pattern lies.
 * @param y is the y coordinate where the desired pattern lies.
 */
void scr_get (Screen *src, Bitmap *dst, int x, int y)
{
    /* local variables */
    char far *s; /* address to copy data from */
    char *d; /* address to copy data to */
    int r; /* row counter */
    int ys; /* y source coordinate */

    /* src is here for future proofing */
    src = src; /* shut the compiler up, hopefully */

    /* ensure the screen parameters are set up */
    if (! ydiv)
	set_screen_parameters (src);

    /* copy the pixels */
    for (r = 0; r < dst->height; ++r) {
 	ys = (y + r) * ymult / ydiv;
	s = x / 4
	    + 0x2000 * (ys % interleave)
	    + swidth * (ys / interleave)
	    + base;
        d = dst->pixels + dst->width / 4 * r;
        _fmemcpy (d, s, dst->width / 4);
    }
}

/**
 * Draw a box on the screen, filled in the current ink colour.
 * @param screen is the screen to affect.
 * @param x is the x coordinate of the top left of the box.
 * @param y is the y coordinate of the top left of the box.
 * @param width is the width of the box.
 * @param height is the height of the box.
 * The draw mode is always DRAW_PUT. For anything more sophisticated,
 * the scr_put() function should be used.
 */
void scr_box (Screen *screen, int x, int y, int width, int height)
{
    /* local variables */
    char v; /* value to write to the screen */
    int r; /* row counter */
    char far *d; /* address to write to */
    int yd; /* y destination coordinate */

    /* ensure the screen parameters are set up */
    if (! ydiv)
	set_screen_parameters (screen);

    /* determine the byte value that will fill the box */
    v = screen->ink * 0x55;

    /* fill each individual row */
    for (r = 0; r < height; ++r) {
	yd = (y + r) * ymult / ydiv;
	d = x / 4
	    + 0x2000 * (yd % interleave)
	    + swidth * (yd / interleave)
	    + base;
        _fmemset (d, v, width / 4);
    }
}

/**
 * Print some text on the screen in the current ink colour.
 * @param screen is the screen to affect.
 * @param x is the x coordinate where the text should appear.
 * @param y is the y coordinate where the text should appear.
 * @param message is the message to print.
 * The text is printed in a box of the paper colour.
 */
void scr_print (Screen *screen, int x, int y, char *message)
{
    /* local variables */
    int b; /* character pointer */
    int r; /* row of character */
    int yd; /* destination y coordinate */
    char v; /* value of byte in character */
    char far *d; /* pointer to destination byte on screen */

    /* only try to print if a font is selected */
    if (! screen->font) return;

    /* ensure the screen parameters are set up */
    if (! ydiv)
	set_screen_parameters (screen);

    /* print each character */
    for (b = 0; message[b]; ++b)
        for (r = 0; r < 8; ++r) {
	    yd = (y + r) * ymult / ydiv;
	    d = b + x / 4
		+ 0x2000 * (yd % interleave)
		+ swidth * (yd / interleave)
		+ base;
            v = screen->font->pixels[r + 8
                * (message[b] - screen->font->first)];
            if (screen->ink != 3 || screen->paper != 0)
                v = (v & 0x55 * screen->ink)
                    | ((0xff ^ v) & 0x55 * screen->paper);
            *d = v;
        }
}

/**
 * Set the ink colour for scr_box and scr_print operations.
 * @param screen is the screen to affect.
 * @param ink is the colour to use, 0..3.
 */
void scr_ink (Screen *screen, int ink)
{
    screen->ink = ink;
}

/**
 * Set the paper colour for scr_print operations.
 * @param screen is the screen to affect.
 * @param paper is the colour to use, 0..3.
 */
void scr_paper (Screen *screen, int paper)
{
    screen->paper = paper;
}

/**
 * Set the font for scr_print operations.
 * @param screen is the screen to affect.
 * @param font is the font to use.
 */
void scr_font (Screen *screen, Font *font)
{
    screen->font = font;
}

/**
 * Reset the screen back to text mode which graphic output is finished.
 * @param screen is the screen to affect.
 */
void scr_destroy (Screen *screen)
{
    screen->mode = 3;
    set_mode (screen);
    free (screen);
}
