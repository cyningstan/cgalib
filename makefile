# ======================================================================
# CGALib - Watcom C Version.
# Project Makefile.
#
# Released as Public Domain by Damian Gareth Walker, 2020.
# Created 29-Jun-2020.
#

# Don't prompt for deleting after errors as crashes wmake in Windows
.ERASE

# Model
!ifndef MODEL
MODEL = ms
!endif

# Directories
SRCDIR = src
INCDIR = inc
FNTDIR = fnt
BITDIR = bit
DOCDIR = doc
OBJDIR = obj
TGTDIR = cgalib
LIBDIR = $(TGTDIR)/lib
TGTINC = $(TGTDIR)/inc
TGTFNT = $(TGTDIR)/fnt
TGTBIT = $(TGTDIR)/bit
TGTDOC = $(TGTDIR)/doc

LOGDIR = loglib
LOGLIB = $(LOGDIR)\lib
LOGINC = $(LOGDIR)\inc

# Tool commands and their options
CC = wcc
LIB = wlib
LD = wcl
COPTS = -q -0 -W4 -$(MODEL) -I=$(INCDIR) -I=$(LOGINC)
LOPTS = -q
!ifdef __LINUX__
CP = cp
!else
CP = copy
!endif

# Whole project
all : &
	$(TGTDIR)/demo.exe &
	$(TGTDIR)/makefont.exe &
	$(TGTDIR)/makebit.exe &
	$(TGTDIR)/makefont.exe &
	$(TGTINC)/cgalib.h &
	$(TGTINC)/screen.h &
	$(TGTINC)/bitmap.h &
	$(TGTINC)/font.h &
	$(TGTBIT)/demo.bit &
	$(TGTBIT)/makebit.bit &
	$(TGTBIT)/makefont.bit &
	$(TGTFNT)/past.fnt &
	$(TGTFNT)/present.fnt &
	$(TGTFNT)/future.fnt &
	$(TGTDOC)/cgalib.txt

# Demonstration and Utilities
$(TGTDIR)/demo.exe : $(OBJDIR)/demo.o $(TGTDIR)/cgalib.lib
	*$(LD) $(LOPTS) -fe=$@ $<
$(TGTDIR)/makefont.exe : $(OBJDIR)/makefont.o $(TGTDIR)/cgalib.lib $(LOGLIB)\loglib.lib
	*$(LD) $(LOPTS) -fe=$@ $<
$(TGTDIR)/makebit.exe : $(OBJDIR)/makebit.o $(TGTDIR)/cgalib.lib
	*$(LD) $(LOPTS) -fe=$@ $<

# Libraries
$(TGTDIR)/cgalib.lib : &
	$(OBJDIR)/screen.o &
	$(OBJDIR)/bitmap.o &
	$(OBJDIR)/font.o
	*$(LIB) $(LIBOPTS) $@ &
		+-$(OBJDIR)/screen.o &
		+-$(OBJDIR)/bitmap.o &
		+-$(OBJDIR)/font.o

# Header files in the target directory
$(TGTINC)/cgalib.h : $(INCDIR)/cgalib.h
	$(CP) $< $@
$(TGTINC)/screen.h : $(INCDIR)/screen.h
	$(CP) $< $@
$(TGTINC)/bitmap.h : $(INCDIR)/bitmap.h
	$(CP) $< $@
$(TGTINC)/font.h : $(INCDIR)/font.h
	$(CP) $< $@

# Sample files in the target directory
$(TGTBIT)/demo.bit : $(BITDIR)/demo.bit
	$(CP) $< $@
$(TGTBIT)/makebit.bit : $(BITDIR)/makebit.bit
	$(CP) $< $@
$(TGTBIT)/makefont.bit : $(BITDIR)/makefont.bit
	$(CP) $< $@
$(TGTFNT)/past.fnt : $(FNTDIR)/past.fnt
	$(CP) $< $@
$(TGTFNT)/present.fnt : $(FNTDIR)/present.fnt
	$(CP) $< $@
$(TGTFNT)/future.fnt : $(FNTDIR)/future.fnt
	$(CP) $< $@

# Documentation in the target directory
$(TGTDOC)/cgalib.txt : $(DOCDIR)/cgalib.txt
	$(CP) $< $@

# Object files for the executables
$(OBJDIR)/demo.o : $(SRCDIR)/demo.c
	*wcl $(COPTS) -c -fo=$@ $<
$(OBJDIR)/makefont.o : $(SRCDIR)/makefont.c
	*wcl $(COPTS) -c -fo=$@ $<
$(OBJDIR)/makebit.o : $(SRCDIR)/makebit.c
	*wcl $(COPTS) -c -fo=$@ $<

# Object files for the modules (small model)
$(OBJDIR)/screen.o : $(SRCDIR)/screen.c
	*wcl $(COPTS) -c -fo=$@ $<
$(OBJDIR)/bitmap.o : $(SRCDIR)/bitmap.c
	*wcl $(COPTS) -c -fo=$@ $<
$(OBJDIR)/font.o : $(SRCDIR)/font.c
	*wcl $(COPTS) -c -fo=$@ $<
