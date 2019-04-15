AC_INIT([uROS-Benchmarking], [0.0.1], [amalki@piap.pl], [libbenchmark])
AC_PREREQ([2.68])
AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([inc/config_lib.h])

#Check CC
AM_PROG_AR
AC_PROG_CC

# check headers
AC_CHECK_HEADERS([errno.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([poll.h])
AC_CHECK_HEADERS([strings.h])
AC_CHECK_HEADERS([stdbool.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_HEADERS([sys/param.h])
AC_CHECK_HEADERS([sys/select.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([sys/sysctl.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([sys/inttypes.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([unistd.h])

PKG_CHECK_MODULES([CHECK], [check])

#Initialize libtool
LT_PREREQ([2.4])
LT_INIT

CFLAGS="-g -O2"
LTCFLAGS="$CFLAGS"

#Replace
top_abs_path=`(pwd)`
ac_list_folder_ext=`(for i in $(ls -d ext/*/); do if [[ "$i" != "ext/patches" ]]; then printf ${i}" " ; fi; done)`

AC_SUBST([top_abs_path])
AC_SUBST([ac_list_folder_ext])

### Configuration for openocd
AC_ARG_ENABLE([swd-openocd],
    AS_HELP_STRING([--disable-swd-openocd], [Disable use of openocd to control swd]))

AC_DEFINE_UNQUOTED([BENCHMARKING_TOP_DIR],["$top_abs_path"], 
				[Top soft directory])

AS_IF([test "x$enable_openocd" != "xno"], [
	OPENOCD_SCRIPT_PATH=$top_abs_path/ext/openocd/tcl
	AC_DEFINE_UNQUOTED([SWD_CTRL_OPENOCD_SCRIPT_PATH],["$OPENOCD_SCRIPT_PATH"], 
				[Path to openocd scripts])
	AC_DEFINE([SWD_CTRL_OPENOCD], [], [Activate openOCD])
])

### Enable configuration of openocd
AC_ARG_ENABLE([swd_debug_openocd],
    AS_HELP_STRING([--enable-swd-debug-openocd], [Enable OpenOCD to ouput debug information]))

AS_IF([test "x$enable_swd_debug_openocd" = "xyes"], 
	AC_DEFINE([SWD_CTRL_OPENOCD_DEBUG], [], [Activate openOCD])
])

### Enable debug on openocd
AC_ARG_ENABLE([swd_debug_openocd],
    AS_HELP_STRING([--enable-swd-debug-openocd], [Enable OpenOCD to ouput debug information]))

AS_IF([test "x$enable_swd_debug_openocd" = "xyes"], 
	AC_DEFINE([SWD_CTRL_OPENOCD_DEBUG], [], [Activate openOCD])
])

### Enable debug on openocd
AC_ARG_ENABLE([libini],
    AS_HELP_STRING([--enable-libini], [Enable libini, currently the only one to be used]))

AS_IF([test "x$enable_libini" != "xno"], [
	AC_DEFINE([CONFIG_LIBINI], [], [Activate libini])
])
AC_CONFIG_FILES([Makefile src/Makefile apps/Makefile \
res/configs/execution_config.ini res/configs/memory_heap_config.ini  tools/generic-paths.sh])
AC_OUTPUT

