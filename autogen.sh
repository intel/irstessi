#!/bin/sh
# Run this to generate all the initial makefiles, etc.

# Don't ignore errors.
set -e

# Make it possible to specify path in the environemnt
: ${AUTOCONF=autoconf}
: ${AUTOHEADER=autoheader}
: ${ACLOCAL=aclocal}
: ${AUTOMAKE=automake}
: ${LIBTOOLIZE=libtoolize}

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

(
# Some shells don't propagate "set -e" to subshells.
set -e

# The autoconf cache (version after 2.52) is not reliable yet.
cd "$srcdir"
rm -rf autom4te.cache

$LIBTOOLIZE

ACLOCAL_INCLUDES="-I m4"

# Sime old version of GNU build tools fail to set error codes.
# Check if all required files are present before continue to next setep.
$ACLOCAL $ACLOCAL_INCLUDES $ACLOCAL_FLAGS
test -f aclocal.m4 || \
    { echo "**Error**: aclocal failed to generate aclocal.m4" >&2; exit 1; }

$AUTOHEADER || exit 1
test -f config.h.in || \
    { echo "**Error**: autoheader failed to generate config.h.in" >&2; exit 1; }

$AUTOCONF || exit 1
test -f configure || \
    { echo "**Error**: autoconf failed to generate configure" >&2; exit 1; }

$AUTOMAKE --add-missing --copy
test -f Makefile.in || \
    { echo "**Error**: automake failed to generate Makefile.in" >&2; exit 1; }

) || exit 1

./configure
