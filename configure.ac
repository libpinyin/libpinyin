#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.


m4_define([libpinyin_major_version], [0])
m4_define([libpinyin_minor_version], [9])
m4_define([libpinyin_micro_version], [93])
m4_define([libpinyin_abi_current], [4])
m4_define([libpinyin_abi_revision], [0])

m4_define([libpinyin_version],
          [libpinyin_major_version.libpinyin_minor_version.libpinyin_micro_version])
m4_define([libpinyin_binary_version],
          [libpinyin_abi_current.libpinyin_abi_revision])

AC_PREREQ(2.60)
AC_INIT([libpinyin], [libpinyin_version], [https://github.com/libpinyin/libpinyin/issues/new])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Define a string for binary compatibility
m4_define([lt_current], [libpinyin_abi_current])
m4_define([lt_revision], [libpinyin_abi_revision])
LT_VERSION_INFO="lt_current:lt_revision"
AC_SUBST(LT_VERSION_INFO)

LIBPINYIN_BINARY_VERSION="libpinyin_binary_version"
AC_SUBST(LIBPINYIN_BINARY_VERSION)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_GNU_SOURCE

# Init libtool
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

# libtool option to control which symbols are exported
# right now, symbols starting with _ are not exported
LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[[^_]].*"'
AC_SUBST(LIBTOOL_EXPORT_OPTIONS)

# Checks for libraries.
PKG_CHECK_MODULES(GLIB2, [glib-2.0 >= 2.4.0])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([locale.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_MMAP
AC_CHECK_FUNCS([gettimeofday memmove memset setlocale])

AC_CHECK_HEADERS([libintl.h string.h])

AC_CHECK_HEADER([db.h], [], AC_MSG_ERROR([Cannot find Berkeley DB library version 4]))

AC_SEARCH_LIBS([db_create], [db], [], AC_MSG_ERROR([Cannot find Berkeley DB library version 4]))


AC_CONFIG_FILES([libpinyin.pc
                 libpinyin.spec
		 Makefile
                 doc/Makefile
		 data/Makefile
		 src/Makefile
                 src/include/Makefile
                 src/storage/Makefile
		 src/lookup/Makefile
		 tests/Makefile
                 tests/include/Makefile
                 tests/storage/Makefile
		 tests/lookup/Makefile
		 utils/Makefile
                 utils/storage/Makefile
                 utils/segment/Makefile
                 utils/training/Makefile
])

AC_OUTPUT

AC_MSG_RESULT([
Build options:
    Version                     $VERSION
    Install prefix              $prefix
])
