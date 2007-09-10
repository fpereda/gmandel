CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = $(PACKAGE_CFLAGS)
AM_CFLAGS = $(GMANDEL_CFLAGS) -D_XOPEN_SOURCE=600

SUBDIRS = .

bin_PROGRAMS = gmandel

gmandel_SOURCES = gmandel.c xfuncs.h \
                  color.h color.c \
                  gui_about.c gui_about.h \
                  gui_progress.c gui_progress.h \
                  gui_save.c gui_save.h \
                  mandelbrot.c mandelbrot.h \
                  paint.c paint.h \
                  stack.c stack.h \
                  parallel_paint.h
gmandel_LDADD = $(PACKAGE_LIBS) -lm

if ENABLE_THREADS

gmandel_SOURCES += parallel_paint.c
gmandel_LDADD   += -lpthread

endif

# vim: set et:
