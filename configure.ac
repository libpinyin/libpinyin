#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.


AC_PREREQ(2.60)
AC_INIT(libpinyin, 0.2.99, alexepico@gmail.com)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_GNU_SOURCE

# Init libtool
AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_LIB_LTDL
AC_SUBST(LIBTOOL_DEPS)

# Init gettext
ALL_LINGUAS="zh_CN zh_TW"#"ko ja de fr it"
AM_GNU_GETTEXT

# Check intltool
#AC_PROG_INTLTOOL
IT_PROG_INTLTOOL([0.33], [no-xml])

GETTEXT_PACKAGE="libpinyin"
AC_SUBST(GETTEXT_PACKAGE)

# libtool option to control which symbols are exported
# right now, symbols starting with _ are not exported
LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[[^_]].*"'
AC_SUBST(LIBTOOL_EXPORT_OPTIONS)

# Checks for libraries.
PKG_CHECK_MODULES(GLIB2, [glib-2.0 >= 2.4.0])

GLIB2_CPPFLAGS=`$PKG_CONFIG --cflags glib-2.0`

AC_SUBST(GLIB2_CPPFLAGS)

GLIB2_LDFLAGS=`$PKG_CONFIG --libs glib-2.0`

AC_SUBST(GLIB2_LDFLAGS)

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
AC_CHECK_FUNCS([gettimeofday memmove memset setlocale])

AC_CHECK_HEADERS([libintl.h string.h])

AC_CHECK_HEADER([db.h], [], AC_MSG_ERROR([Cannot find Berkeley DB library version 4]))

AC_SEARCH_LIBS([db_create], [db], [], AC_MSG_ERROR([Cannot find Berkeley DB library version 4]))


AC_CONFIG_FILES([Makefile 
		 po/Makefile.in
		 intl/Makefile
		 src/Makefile
                 src/include/Makefile
                 src/storage/Makefile
		 src/segment/Makefile
		 src/training/Makefile
		 src/lookup/Makefile
		 tests/Makefile
                 tests/include/Makefile
                 tests/storage/Makefile
		 tests/lookup/Makefile
		 utils/Makefile
                 utils/storage/Makefile])
AC_OUTPUT
