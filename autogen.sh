#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="libpinyin"

(test -f $srcdir/configure.ac \
  && test -f $srcdir/README ) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

which gnome-autogen.sh || {
    echo "You need to install gnome-common from the GNOME CVS"
    exit 1
}

(test -f $srcdir/ChangeLog) || {
    touch $srcdir/ChangeLog
}

CFLAGS=${CFLAGS-"-Wall -Werror"}

ACLOCAL_FLAGS="$ACLOCAL_FLAGS"
REQUIRED_AUTOMAKE_VERSION=1.8

. gnome-autogen.sh "$@"
