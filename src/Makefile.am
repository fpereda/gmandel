CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS = $(GMANDEL_CFLAGS) -D_XOPEN_SOURCE=600 $(gtk_CFLAGS)

SUBDIRS = .

bin_PROGRAMS = gmandel

gmandel_SOURCES = gmandel.c xfuncs.h gui.h \
                  color.c color.h \
                  gfract.c gfract.h \
                  gui_about.c gui_about.h \
                  gui_actions.c gui_actions.h \
                  gui_menu.c gui_menu.h \
                  gui_progress.c gui_progress.h \
                  gui_report.c gui_report.h \
                  gui_save.c gui_save.h \
                  gui_state.c gui_state.h \
                  gui_status.c gui_status.h \
                  mandelbrot.c mandelbrot.h \
                  color_filter.c color_filter.h \
                  gui_callbacks.c gui_callbacks.h \
                  mupoint.c mupoint.h \
                  stack.c stack.h \
                  parallel_paint.h
gmandel_LDADD = $(gtk_LIBS) -lm -lgthread-2.0

if ENABLE_THREADS

gmandel_SOURCES += parallel_paint.c
gmandel_LDADD   += -lpthread

endif

# vim: set et:
