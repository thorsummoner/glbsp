#
# glBSP Makefile for MacOSX Plugin
#

MAIN=.

OUTNAME=libglbsp.a

CC=gcc
CFLAGS=-O3 -Wall -DGLBSP_PLUGIN -DMACOSX -DINLINE_G=inline

OBJS=$(MAIN)/analyze.o  \
     $(MAIN)/blockmap.o \
     $(MAIN)/glbsp.o    \
     $(MAIN)/level.o    \
     $(MAIN)/node.o     \
     $(MAIN)/reject.o   \
     $(MAIN)/seg.o      \
     $(MAIN)/system.o   \
     $(MAIN)/util.o     \
     $(MAIN)/wad.o


# ----- TARGETS ------------------------------------------------------

all:    $(OUTNAME)

clean:
	rm -f $(OUTNAME) $(MAIN)/*.o

$(OUTNAME): $(OBJS)
	libtool -static -o $(OUTNAME) - $(OBJS)
	ranlib $(OUTNAME)

.PHONY: all clean

