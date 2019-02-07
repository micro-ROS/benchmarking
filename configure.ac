AC_INIT([uROS-Benchmarking], [0.0.1], [amalki@piap.pl], [libbenchmark])
AC_PREREQ([2.68])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_MACRO_DIR([m4])

#Check CC
AM_PROG_AR
AC_PROG_CC

#Initialize libtool
LT_PREREQ([2.4])
LT_INIT

CFLAGS="-g -O0"
LTCFLAGS="$CFLAGS"

AC_CONFIG_FILES([Makefile src/Makefile apps/Makefile])


AC_OUTPUT

