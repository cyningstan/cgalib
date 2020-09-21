# CGALIB for OpenWatcom C

CGALIB is a relatively fast, specialised C graphics library. It targets the CGA graphics adapter, specifically the 4-colour 320x200 CGA graphics modes, although it will also emulate these modes on the 640x200 screen using dithering, for clarity on monochrome devices. For speed it organises the screen into 4x2 pixel cells, making it more suitable for turn-based games than for action games that require pixel-perfect positioning.

It differs from the graphics libraries provided with various C compilers in the following particulars:

1. It's limited to 320x200 4-colour graphics. This allows it to make various assumptions about the screen geometry, making it simpler and faster than more generalised libraries.

2. It emulates the CGA palette registers on EGA and newer hardware, so that palette changes are consistent across different video systems (no more cyan/magenta on EGA/VGA displays unless you really want it).

3. It can emulate the 320x200 video modes in 640x200 "high resolution" mode in monochrome using dithering. This means that  it is easy for the developer to provide an option for clearer graphics on monochrome machines like early handhelds and laptops, or on other displays where the chosen colours might be unclear,

4. It allows graphics to be manipulated off the screen, without the need for secondary graphics pages in hardware. This allows graphics to be pre-rendered for instant display on slower CPUs.

CGALIB is provided in source and binary formats, for both the OpenWatcom and Personal C Compilers. It includes a couple of rudimentary utilities for creating bitmap and font files, and it is supplied with a sample demonstration game.

This repository is the source code for using with OpenWatcom C. Full documentation on building and using the library is provided in the 'doc' directory.
