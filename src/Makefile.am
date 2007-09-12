CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS = $(GMANDEL_CFLAGS) -D_XOPEN_SOURCE=600 $(gtk_CFLAGS)

SUBDIRS = .

bin_PROGRAMS = gmandel

gmandel_SOURCES = gmandel.c xfuncs.h gui.h \
                  color.c color.h \
                  gui_about.c gui_about.h \
                  gui_actions.c gui_actions.h \
                  gui_callbacks.c gui_callbacks.h \
                  gui_menu.c gui_menu.h \
                  gui_progress.c gui_progress.h \
                  gui_save.c gui_save.h \
                  gui_status.c gui_status.h \
                  mandelbrot.c mandelbrot.h \
                  paint.c paint.h \
                  stack.c stack.h \
                  parallel_paint.h
gmandel_LDADD = $(gtk_LIBS) -lm

if ENABLE_THREADS

gmandel_SOURCES += parallel_paint.c
gmandel_LDADD   += -lpthread

endif

# vim: set et:
