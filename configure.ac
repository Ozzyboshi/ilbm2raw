AC_INIT([ilbm2raw],[0.1])
AM_INIT_AUTOMAKE([silent-rules])
AC_CONFIG_SRCDIR([src/ilbm2raw.c])
AC_PROG_CC([gcc])

AC_CHECK_PROG([dd],[dd],[yes],[no],,)
if test "${bitmap_conv}" = no; then
	AC_MSG_ERROR([Unable to find dd utility,])
fi

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT


