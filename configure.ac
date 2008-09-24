dnl vim: set sw=4 sts=4 ts=4 noet ft=config foldmethod=marker foldmarker={{{,}}} :
dnl Basic setup
AC_PREREQ(2.59)
AC_INIT([src/gmandel.c])
AC_CONFIG_AUX_DIR(config)

VERSION_MAJOR=0
VERSION_MINOR=1
VERSION_MICRO=0
VERSION_FULL="$VERSION_MAJOR.$VERSION_MINOR.$VERSION_MICRO"
VERSION="$VERSION_FULL"

AC_SUBST([VERSION_MAJOR])
AC_SUBST([VERSION_MINOR])
AC_SUBST([VERSION_MICRO])
AC_SUBST([VERSION_FULL])

AM_INIT_AUTOMAKE(gmandel, [$VERSION_FULL])

dnl Basic stuff
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_STDC_HEADERS

dnl {{{ Info about this build
BUILDUSER=`whoami`
AC_SUBST([BUILDUSER])
BUILDHOST=`hostname`
AC_SUBST([BUILDHOST])
BUILDDATE=`date +%Y-%m-%dT%H:%M:%S%z`
AC_SUBST([BUILDDATE])
GIT_DIR="${ac_top_srcdir:-./}/.git"
if test -d "$GIT_DIR"; then
	GITREV=`GIT_DIR=$GIT_DIR git describe 2>/dev/null`
	if test "x$GITVERSION" = "x"; then
		GITREV=`GIT_DIR=$GIT_DIR git rev-parse --short HEAD`
	fi
else
	GITREV=
fi
AC_SUBST([GITREV])
dnl }}}

dnl {{{ Sane CFLAGS
GMANDEL_CFLAGS="-std=c99"
AC_DEFUN([CHECK_CFLAG], [
                  save_CFLAGS=$CFLAGS
                  CFLAGS="$CFLAGS $GMANDEL_CFLAGS $1 -Werror"
                  AC_COMPILE_IFELSE([
#include <stdio.h>
int main(int argc, char **argv)
{
        char *s = "test";
        printf("%s.\n", s);
        return 0;
}
                  ],
                  [cflag_success=yes],
                  [cflag_success=no])
                  CFLAGS="$save_CFLAGS"
                  if test "x$cflag_success" = "xyes" ; then
                          GMANDEL_CFLAGS="$GMANDEL_CFLAGS $1"
                          cflags_message="${cflags_message} $1"
                  fi
                  ])
AC_MSG_CHECKING([for appropiate compiler flags])
CHECK_CFLAG([-Wall])
CHECK_CFLAG([-Wextra])
CHECK_CFLAG([-Winit-self])
CHECK_CFLAG([-Wredundant-decls])
CHECK_CFLAG([-Winline])
CHECK_CFLAG([-Wunused-function])
# CHECK_CFLAG([-Wunreachable-code])
CHECK_CFLAG([-Wshadow])
AC_MSG_RESULT([${cflags_message}])
AC_SUBST([GMANDEL_CFLAGS])
dnl }}}

dnl {{{ GTK+ and GDK
AC_MSG_CHECKING([whether to use Gtk-OSX])
AC_ARG_WITH([gtk-osx],
            AS_HELP_STRING([--with-gtk-osx], [Use Gtk-OSX]),
            WANT_GTKOSX=$withval,
            WANT_GTKOSX=no)
AC_MSG_RESULT([$WANT_GTKOSX])
AC_SUBST([WANT_GTKOSX])
AM_CONDITIONAL([WANT_GTKOSX], test "x$WANT_GTKOSX" = "xyes")

if test x"$WANT_GTKOSX" = "xno" ; then
   PKG_CHECK_MODULES(gtk, [gtk+-2.0 >= 2.10.0])
   PKG_CHECK_MODULES(gthread, [gthread-2.0 >= 2.10.0])
fi
dnl }}}

dnl Makefile generation
AC_CONFIG_HEADER(config.h)
AC_OUTPUT(
		  Makefile
		  src/Makefile
		  )
