dnl vim: set sw=4 sts=4 ts=4 noet ft=config foldmethod=marker foldmarker={{{,}}} :
dnl Basic setup
AC_INIT(gmandel, [0.1], ferdy@gentoo.org, gmandel)
AC_PREREQ(2.59)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE(gmandel, 0.1)

dnl Basic stuff
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

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
CHECK_CFLAG([-std=c99])
AC_MSG_RESULT([${cflags_message}])
AC_SUBST([GMANDEL_CFLAGS])
dnl }}}

dnl {{{ GTK+ and GDK
PKG_CHECK_MODULES(PACKAGE, [gtk+-2.0 gdk-2.0])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)
dnl }}}

dnl Makefile generation
AC_OUTPUT(
		  Makefile
		  src/Makefile
		  )
