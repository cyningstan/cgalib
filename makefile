# ======================================================================
# CGALib - Watcom C Version.
# Project Makefile.
#
# Released as Public Domain by Damian Gareth Walker, 2020.
# Created 29-Jun-2020.
#

# Don't prompt for deleting after errors as crashes wmake in Windows
.ERASE

# Directories
SRCDIR = src
INCDIR = inc
FNTDIR = fnt
BITDIR = bit
DOCDIR = doc
OBJDIR = obj
OMSDIR = $(OBJDIR)/ms
OMMDIR = $(OBJDIR)/mm
OMCDIR = $(OBJDIR)/mc
OMLDIR = $(OBJDIR)/ml
OMHDIR = $(OBJDIR)/mh
TGTDIR = cgalib
LIBDIR = $(TGTDIR)/lib
TGTINC = $(TGTDIR)/inc
TGTFNT = $(TGTDIR)/fnt
TGTBIT = $(TGTDIR)/bit
TGTDOC = $(TGTDIR)/doc

# Tool commands and their options
CC = wcc
LIB = wlib
LD = wcl
COPTS = -q -0 -W4 -I=$(INCDIR)
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
	$(TGTDIR)/cga-ms.lib &
	$(TGTDIR)/cga-mm.lib &
	$(TGTDIR)/cga-mc.lib &
	$(TGTDIR)/cga-ml.lib &
	$(TGTDIR)/cga-mh.lib &
	$(TGTINC)/cgalib.h &
	$(TGTINC)/screen.h &
	$(TGTINC)/bitmap.h &
	$(TGTINC)/font.h &
	$(TGTBIT)/demo.bit &
	$(TGTFNT)/past.fnt &
	$(TGTFNT)/present.fnt &
	$(TGTFNT)/future.fnt &
	$(TGTDOC)/cgalib.txt

# Demonstration and Utilities
$(TGTDIR)/demo.exe : $(OBJDIR)/demo.o $(TGTDIR)/cga-ms.lib
	$(LD) $(LOPTS) -fe=$@ $<
$(TGTDIR)/makefont.exe : $(OBJDIR)/makefont.o $(TGTDIR)/cga-ms.lib
	$(LD) $(LOPTS) -fe=$@ $<
$(TGTDIR)/makebit.exe : $(OBJDIR)/makebit.o $(TGTDIR)/cga-ms.lib
	$(LD) $(LOPTS) -fe=$@ $<

# Libraries
$(TGTDIR)/cga-ms.lib : &
	$(OMSDIR)/screen.o &
	$(OMSDIR)/bitmap.o &
	$(OMSDIR)/font.o
	$(LIB) $(LIBOPTS) $@ &
		+-$(OMSDIR)/screen.o &
		+-$(OMSDIR)/bitmap.o &
		+-$(OMSDIR)/font.o
$(TGTDIR)/cga-mm.lib : &
	$(OMMDIR)/screen.o &
	$(OMMDIR)/bitmap.o &
	$(OMMDIR)/font.o
	$(LIB) $(LIBOPTS) $@ &
		+-$(OMMDIR)/screen.o &
		+-$(OMMDIR)/bitmap.o &
		+-$(OMMDIR)/font.o
$(TGTDIR)/cga-mc.lib : &
	$(OMCDIR)/screen.o &
	$(OMCDIR)/bitmap.o &
	$(OMCDIR)/font.o
	$(LIB) $(LIBOPTS) $@ &
		+-$(OMCDIR)/screen.o &
		+-$(OMCDIR)/bitmap.o &
		+-$(OMCDIR)/font.o
$(TGTDIR)/cga-ml.lib : &
	$(OMLDIR)/screen.o &
	$(OMLDIR)/bitmap.o &
	$(OMLDIR)/font.o
	$(LIB) $(LIBOPTS) $@ &
		+-$(OMLDIR)/screen.o &
		+-$(OMLDIR)/bitmap.o &
		+-$(OMLDIR)/font.o
$(TGTDIR)/cga-mh.lib : &
	$(OMHDIR)/screen.o &
	$(OMHDIR)/bitmap.o &
	$(OMHDIR)/font.o
	$(LIB) $(LIBOPTS) $@ &
		+-$(OMHDIR)/screen.o &
		+-$(OMHDIR)/bitmap.o &
		+-$(OMHDIR)/font.o

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
	wcl $(COPTS) -c -fo=$@ $< -i=$(INCDIR)
$(OBJDIR)/makefont.o : $(SRCDIR)/makefont.c
	wcl $(COPTS) -c -fo=$@ $< -i=$(INCDIR)
$(OBJDIR)/makebit.o : $(SRCDIR)/makebit.c
	wcl $(COPTS) -c -fo=$@ $< -i=$(INCDIR)

# Object files for the modules (small model)
$(OMSDIR)/screen.o : $(SRCDIR)/screen.c
	wcl $(COPTS) -ms -c -fo=$@ $< -i=$(INCDIR)
$(OMSDIR)/bitmap.o : $(SRCDIR)/bitmap.c
	wcl $(COPTS) -ms -c -fo=$@ $< -i=$(INCDIR)
$(OMSDIR)/font.o : $(SRCDIR)/font.c
	wcl $(COPTS) -ms -c -fo=$@ $< -i=$(INCDIR)

# Object files for the modules (medium model)
$(OMMDIR)/screen.o : $(SRCDIR)/screen.c
	wcl $(COPTS) -mm -c -fo=$@ $< -i=$(INCDIR)
$(OMMDIR)/bitmap.o : $(SRCDIR)/bitmap.c
	wcl $(COPTS) -mm -c -fo=$@ $< -i=$(INCDIR)
$(OMMDIR)/font.o : $(SRCDIR)/font.c
	wcl $(COPTS) -mm -c -fo=$@ $< -i=$(INCDIR)

# Object files for the modules (compact model)
$(OMCDIR)/screen.o : $(SRCDIR)/screen.c
	wcl $(COPTS) -mc -c -fo=$@ $< -i=$(INCDIR)
$(OMCDIR)/bitmap.o : $(SRCDIR)/bitmap.c
	wcl $(COPTS) -mc -c -fo=$@ $< -i=$(INCDIR)
$(OMCDIR)/font.o : $(SRCDIR)/font.c
	wcl $(COPTS) -mc -c -fo=$@ $< -i=$(INCDIR)

# Object files for the modules (large model)
$(OMLDIR)/screen.o : $(SRCDIR)/screen.c
	wcl $(COPTS) -ml -c -fo=$@ $< -i=$(INCDIR)
$(OMLDIR)/bitmap.o : $(SRCDIR)/bitmap.c
	wcl $(COPTS) -ml -c -fo=$@ $< -i=$(INCDIR)
$(OMlDIR)/font.o : $(SRCDIR)/font.c
	wcl $(COPTS) -ml -c -fo=$@ $< -i=$(INCDIR)

# Object files for the modules (huge model)
$(OMHDIR)/screen.o : $(SRCDIR)/screen.c
	wcl $(COPTS) -mh -c -fo=$@ $< -i=$(INCDIR)
$(OMHDIR)/bitmap.o : $(SRCDIR)/bitmap.c
	wcl $(COPTS) -mh -c -fo=$@ $< -i=$(INCDIR)
$(OMHDIR)/font.o : $(SRCDIR)/font.c
	wcl $(COPTS) -mh -c -fo=$@ $< -i=$(INCDIR)
