CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = $(PACKAGE_CFLAGS)
AM_CFLAGS = $(GMANDEL_CFLAGS)

SUBDIRS = .

bin_PROGRAMS = gmandel

gmandel_SOURCES = gmandel.c \
                  color.h   \
                  mandelbrot.c mandelbrot.h \
                  paint.c paint.h
gmandel_LDADD = $(PACKAGE_LIBS) -lm

# vim: set et:
