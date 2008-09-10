CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS = $(GMANDEL_CFLAGS) -D_XOPEN_SOURCE=600 \
            $(gtk_CFLAGS) $(gthread_CFLAGS)

SUBDIRS = .

bin_PROGRAMS = gmandel

gmandel_SOURCES = gmandel.c xfuncs.h gui.h gfract_engines.h \
                  color.c color.h \
                  color_filter.c color_filter.h \
                  gfract.c gfract.h \
                  gui_about.c gui_about.h \
                  gui_actions.c gui_actions.h \
                  gui_callbacks.c gui_callbacks.h \
                  gui_menu.c gui_menu.h \
                  gui_progress.c gui_progress.h \
                  gui_report.c gui_report.h \
                  gui_save.c gui_save.h \
                  gui_state.c gui_state.h \
                  gui_status.c gui_status.h \
                  julia.c julia.h \
                  mandelbrot.c mandelbrot.h \
                  mupoint.c mupoint.h
gmandel_LDADD = $(gtk_LIBS) $(gthread_LIBS) -lm

# vim: set et:
