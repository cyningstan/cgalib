/*======================================================================
 * CGALib - Watcom C Version.
 * Demonstration Program.
 * 
 * Released as Public Domain by Damian Gareth Walker, 2020.
 * Created 29-Jun-2020.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <i86.h>
#include "cgalib.h"

/*----------------------------------------------------------------------
 * File Level Variables.
 */

/** @var scr is the screen data */
static Screen *scr;

/** @var fnt is an array of fonts */
static Font *fnt;

/** @var bit is an array of bitmaps */
static Bitmap *bit[16];

/** @var map is the map level as a 2D array */
static int map[18][10];

/** @var player_x is the player's X position */
static int player_x;

/** @var player_y is the player's Y position */
static int player_y;

/** @var map_img is the persistent map image */
static Bitmap *map_img;

/** @var level is the number of droids to generate. */
static int droids;

/** @var score is the game score. */
static int score;

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
 * Level 3 Routines.
 */

/**
 * Show the score.
 */
void display_score (void)
{
    char scoreboard[6];
    sprintf (scoreboard, "%05d", score);
    scr_print (scr, 300, 192, scoreboard);
}

/*
 * Level 2 Routines
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
    /* local variables */
    Font *font;
    FILE *fp;
    char header[8];

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
static int load_bitmaps (char *filename)
{
    /* local variables */
    FILE *fp;
    char header[8];
    int c;

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

    /* read the five bitmaps */
    for (c = 0; c < 16; ++c)
        bit[c] = bit_read (fp);
    fclose (fp);
    return 1;
}

/**
 * Move the player.
 * @returns 1 if the level is finished, 0 if not.
 */
static int move_player (void)
{
    int finished, /* true if the player has won or died */
        xd, /* calculated x direction */
        yd, /* calculated y direction */
        x, /* teleport x coordinate */
        y, /* teleport y coordinate */
        key; /* key pressed */

    /* wait for a key */
    finished = 0;
    key = getch ();
    if (key == 0) key = -getch ();
    scr_ink (scr, 0);
    scr_box (scr, 148, 192, 24, 8);
    scr_ink (scr, 3);

    /* determine vertical direction */
    if (key == -71 || key == -72 || key == -73
        || key == '7' || key == '8' || key == '9'
	|| key == 'q' || key == 'w' || key == 'e'
	|| key == 'Q' || key == 'W' || key == 'E')
        yd = -1;
    else if (key == -79 || key == -80 || key == -81
        || key == '1' || key == '2' || key == '3'
	|| key == 'z' || key == 'x' || key == 'c'
	|| key == 'Z' || key == 'X' || key == 'C'
	|| key == 's' || key == 'S')
        yd = 1;
    else
        yd = 0;
    
    /* determine horizontal direction */
    if (key == -71 || key == -75 || key == -79
        || key == '7' || key == '4' || key == '1'
	|| key == 'q' || key == 'a' || key == 'z'
	|| key == 'Q' || key == 'A' || key == 'Z')
        xd = -1;
    else if (key == -73 || key == -77 || key == -81
        || key == '9' || key == '6' || key == '3'
	|| key == 'e' || key == 'd' || key == 'c'
	|| key == 'E' || key == 'D' || key == 'C')
        xd = 1;
    else
        xd = 0;

    /* move the player */
    if ((xd || yd)
        && player_x + xd >= 0 && player_x + xd <= 17
        && player_y + yd >= 0 && player_y + yd <= 9) {
        map[player_x][player_y] = 0;
        bit_put (map_img, bit[0], 16 * player_x, 16 * player_y,
            DRAW_PSET);
        if (map[player_x + xd][player_y + yd] == 0) {
            map[player_x + xd][player_y + yd] = 1;
            bit_put (map_img, bit[2], 16 * (player_x + xd),
                16 * (player_y + yd), DRAW_AND);
            bit_put (map_img, bit[1], 16 * (player_x + xd),
                16 * (player_y + yd), DRAW_OR);
            player_x += xd;
            player_y += yd;
            scr_put (scr, map_img, 16, 16, DRAW_PSET);
        } else
            scr_print (scr, 148, 192, "OUCH!!");
    }

    /* player tried to move into a wall */
    else if (xd || yd)
        scr_print (scr, 148, 192, "OUCH!!");

    /* teleport the player */
    else if (key == 32) {

        /* find an empty place */
        do {
            x = rand () % 18;
            y = rand () % 10;
        } while (map[x][y]);

        /* move the player there */
        map[player_x][player_y] = 0;
        map[x][y] = 1;
        bit_put (map_img, bit[0], 16 * player_x, 16 * player_y,
            DRAW_PSET);
        bit_put (map_img, bit[2], 16 * x, 16 * y, DRAW_AND);
        bit_put (map_img, bit[1], 16 * x, 16 * y, DRAW_OR);
        player_x = x;
        player_y = y;
        scr_put (scr, map_img, 16, 16, DRAW_PSET);

        /* apply and display the score penalty */
        score -= (score > droids / 2) ? droids / 2 : score;
        display_score ();
    }

    /* return true if finished */
    return finished;
}

/**
 * Move the droids.
 * @returns 1 if the level is finished, 0 if not.
 */
int move_droids (void)
{
    /* local variables */
    int x, y, xd, yd, c, finished, new_map[18][10];

    /* initialise the new map */
    for (x = 0; x < 18; ++x)
        for (y = 0; y < 10; ++y)
            new_map[x][y] = map[x][y] * (map[x][y] != 3);

    /* move all the robots on the map */
    finished = 0;
    for (x = 0; x < 18; ++x)
        for (y = 0; y < 10; ++y)
            if (map[x][y] == 3) {

                /* update the old position with whatever will be here */
                bit_put (map_img, bit[0], 16 * x, 16 * y, DRAW_PSET);
                bit_put (map_img, bit[new_map[x][y] + 1], 16 * x,
                    16 * y, DRAW_AND);
                bit_put (map_img, bit[new_map[x][y]], 16 * x, 16 * y,
                    DRAW_OR);

                /* work out the direction of movement */
                xd = (player_x > x) - (player_x < x);
                yd = (player_y > y) - (player_y < y);

                /* moving on to a blank square or the player */
                if (new_map[x + xd][y + yd] == 0
                    || new_map[x + xd][y + yd] == 1) {
                    new_map[x + xd][y + yd] = 3;
                    bit_put (map_img, bit[4], 16 * (x + xd),
                        16 * (y + yd), DRAW_AND);
                    bit_put (map_img, bit[3], 16 * (x + xd),
                        16 * (y + yd), DRAW_OR);
                    if (x + xd == player_x && y + yd == player_y)
                        finished = 1;
                }

                /* crashing into another droid */
                else if (new_map[x + xd][y + yd] == 3) {
                    new_map[x + xd][y + yd] = 5;
                    bit_put (map_img, bit[6], 16 * (x + xd),
                        16 * (y + yd), DRAW_AND);
                    bit_put (map_img, bit[5], 16 * (x + xd),
                        16 * (y + yd), DRAW_OR);
                    score += 2;
                }

                /* crashing into debris */
                else if (new_map[x + xd][y + yd] == 5)
                    score += 1;
            }
    
    /* copy the new map over the old one */
    c = 0;
    for (x = 0; x < 18; ++x)
        for (y = 0; y < 10; ++y) {
            map[x][y] = new_map[x][y];
            c += (map[x][y] == 3);
        }
    if (!c)
        finished = 1;

    /* redraw the screen and return */
    scr_put (scr, map_img, 16, 16, DRAW_PSET);
    display_score ();
    return finished;
}

/*----------------------------------------------------------------------
 * Level 1 Routines.
 */

/**
 * Initialise the screen.
 * @param mono is true if mono mode was requested.
 */
void initialise_screen (int mono)
{
    if (! (scr = scr_create (mono ? 6 : 4)))
        error_handler (1, "Cannot initialise graphics mode!");
    scr_palette (scr, 5, 7);
    if (! (fnt = load_font ("fnt/future.fnt")))
        error_handler (1, "Cannot load font");
    if (! load_bitmaps ("bit/demo.bit"))
        error_handler (1, "Cannot load bitmaps");
}

/**
 * Initialise a game.
 */
void initialise_game (void)
{
    /* local variables */
    Bitmap *hidden; /* hidden copy of the screen */
    int x, y; /* X and Y game coordinates */
    int walls[8] = {8, 8, 8, 8, 8, 8, 14, 15}; /* random wall pieces */

    /* display please wait message */
    scr_font (scr, fnt);
    scr_print (scr, 132, 192, "Please wait...");

    /* prepare the game field */
    if (! (hidden = bit_create (320, 192)))
        error_handler (1, "Out of memory creating game screen");
    bit_put (hidden, bit[7], 0, 0, DRAW_PSET);
    bit_put (hidden, bit[9], 304, 0, DRAW_PSET);
    bit_put (hidden, bit[12], 0, 176, DRAW_PSET);
    bit_put (hidden, bit[13], 304, 176, DRAW_PSET);
    for (x = 1; x < 19; ++x) {
        bit_put (hidden, bit[walls[rand () % 8]], 16 * x, 0, DRAW_PSET);
        bit_put (hidden, bit[walls[rand () % 8]], 16 * x, 176,
            DRAW_PSET);
    }
    for (y = 1; y < 11; ++y) {
        bit_put (hidden, bit[10], 0, 16 * y, DRAW_PSET);
        bit_put (hidden, bit[11], 304, 16 * y, DRAW_PSET);
    }
    bit_ink (hidden, 0);
    bit_box (hidden, 16, 16, 288, 160);
    scr_put (scr, hidden, 0, 0, DRAW_PSET);
    bit_destroy (hidden);

    /* create the map image */
    if (! (map_img = bit_create (288, 160)))
    error_handler (1, "Out of memory creating game map");

    /* initialise score and such */
    score = 0;
    droids = 12;
}

/**
 * Initialise a single play level
 */
void initialise_level (void)
{
    /* local variables */
    int x, y, /* temporary x and y coordinates */
        c; /* counter for droids */

    /* (re-)display please wait message */
    scr_font (scr, fnt);
    scr_print (scr, 132, 192, "Please wait...");

    /* clear the map */
    for (x = 0; x < 18; ++x)
        for (y = 0; y < 10; ++y)
            map[x][y] = 0;

    /* place the player */
    player_x = rand () % 18;
    player_y = rand () % 10;
    map[player_x][player_y] = 1;

    /* place the droids */
    for (c = 0; c < droids; ++c) {
        do {
            x = rand () % 18;
            y = rand () % 10;
        } while (abs (x - player_x) + abs (y - player_y) < 8
            || map[x][y] != 0);
        map[x][y] = 3;
    }

    /* prepare the game map */
    for (x = 0; x < 18; ++x)
        for (y = 0; y < 10; ++y) {
            bit_put (map_img, bit[0], 16 * x, 16 * y, DRAW_PSET);
            if (map[x][y]) {
                bit_put (map_img, bit[map[x][y] + 1], x * 16, y * 16,
                    DRAW_AND);
                bit_put (map_img, bit[map[x][y]], x * 16, y * 16,
                    DRAW_OR);
            }
        }
    scr_put (scr, map_img, 16, 16, DRAW_PSET);

    /* clear the "please wait" message and display the score */
    scr_ink (scr, 0);
    scr_box (scr, 132, 192, 56, 8);
    scr_ink (scr, 3);
    display_score ();
}

/**
 * Play a single level through.
 */
void play_level (void)
{
    int finished; /* true if the player has won or died */

    /* move player and droids till the level is done */
    do {
        finished = move_player ();
        if (! finished)
            finished = move_droids ();
    } while (! finished);
}

/**
 * Process the end of a level
 */
int end_level (void)
{
    /* display the victory or defeat message */
    if (map[player_x][player_y] == 1)
        scr_print (scr, 132, 192, "Level cleared!");
    else
        scr_print (scr, 128, 192, "You are defeated");
    if (getch () == 0)
        getch ();
    scr_ink (scr, 0);
    scr_box (scr, 128, 192, 64, 8);
    scr_ink (scr, 3);
    ++droids;

    /* return true if player is dead */
    return map[player_x][player_y] != 1;
}

/**
 * Process the end of a game
 */
int end_game (void)
{
    /* local variables */
    int key; /* key that the player pressed */

    /* print the prompt, and get the key */
    scr_print (scr, 124, 192, "Play again (Y/N) ?");
    do {
        key = getch ();
    } while (key != 'Y' && key != 'y' && key != 'N' && key != 'n');
    scr_ink (scr, 0);
    scr_box (scr, 124, 192, 72, 8);
    scr_ink (scr, 3);
    bit_destroy (map_img);

    /* return true if key is 'N' to quit */
    return (key == 'N' || key == 'n');
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
    /* local variables */
    int dead, quit;

    /* initialise */
    initialise_screen (argc == 2 && ! strcmp (argv[1], "-m"));
    srand (time (NULL));

    /* game loop */
    do {
        initialise_game ();
        do {
            initialise_level ();
            play_level ();
            dead = end_level ();
        } while (! dead);
        quit = end_game ();
    } while (! quit);
    scr_destroy (scr);
}
