#!/bin/sh
<<LICENSE_BSD

Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

LICENSE_BSD

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
