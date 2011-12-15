dnl
dnl Configure.ac file for Intel(R) Standard Storage Interface library.
dnl

dnl Autoconf/Automake initialization
AC_PREREQ([2.58])
AC_INIT([Intel(R) Standard Storage Interface library], [], [artur.wojcik@intel.com])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(README)
AC_CONFIG_AUX_DIR(config)

VERSION=0.91
AC_SUBST(VERSION)

AM_INIT_AUTOMAKE(ssi, ${VERSION})
AM_CONFIG_HEADER(config.h)

dnl Set the language we use
CPPFLAGS="-g3 -gdwarf-2 -Wall -Werror -D_GNU_SOURCE"
CFLAGS="-std=gnu99"
CXXFLAGS="-std=gnu++98"

dnl Automake 1.11 - silent build rules
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Initialize maintainer mode
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_C_INLINE
AC_C_BIGENDIAN
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
AC_PROG_MAKE_SET

## AC_ARG_PROGRAMA

dnl Initialize libtool
LT_PREREQ(2.2)
LT_INIT([dlopen disable-static])

PKG_PROG_PKG_CONFIG

dnl Checks for libraries.
SSI_CHECK_SGUTILS
AC_CHECK_TOOL(AR, ar, ar)

dnl Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stdlib.h string.h sys/ioctl.h sys/mount.h sys/time.h memory.h stdint.h unistd.h asm/types.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_SSIZE_T

dnl Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([atexit gettimeofday localtime_r memset munmap strerror])

AC_CONFIG_FILES([Makefile
                 ssi.pc
                 contrib/Makefile
                 contrib/dist/Makefile
                 contrib/dist/debian/Makefile
                 contrib/dist/pkginfo
                 doc/Makefile
                 include/Makefile
                 lib/Makefile
                 lib/efi/Makefile
                 lib/engine/Makefile
                 lib/mpb/Makefile
                 lib/orom/Makefile
                 lib/log/Makefile
                 lib/sgio/Makefile
                 src/Makefile
                 tools/Makefile
                 ut/Makefile])

AC_OUTPUT

echo "
Configuration:

  Source code location:      ${srcdir}
  C Compiler and flags:      ${CC} ${CFLAGS} ${CPPFLAGS}
  C++ Compiler and flags:    ${CXX} ${CXXFLAGS} ${CPPFLAGS}
  Library version:           ${VERSION}
  Common install location:   ${prefix}
  Library install location:  ${libdir}
  Header install location:   ${includedir}
"
