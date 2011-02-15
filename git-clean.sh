#!/bin/bash

# Removes the file if exist (prints the name out)
remove_file () {
	local p="$1"
	if [ -f "$p" ]; then
		rm -f "$p"
		echo "--> $p"
	fi
}

# Removes directory if exist (prints the name out)
remove_dir () {
	local p="$1"
	if [ -d "$p" ]; then
		rm -rf "$p"
		echo "--> [$p]"
	fi
}

# Removes symbolic link if exists (prints the name out)
remove_slink () {
	local p="$1"
	if [ -L "$p" ]; then
		rm -f "$p"
		echo "--> ~$p"
	fi
}

echo "** Cleaning the build files..."
if [ -f Makefile ]; then
    make clean
fi
cd ut
make -f Makefile.session clean
make -f Makefile.string clean
cd -
remove_dir lib/efi/.deps
remove_dir lib/efi/.libs
remove_dir lib/engine/.deps
remove_dir lib/engine/.libs
remove_dir lib/log/.deps
remove_dir lib/log/.libs
remove_dir lib/mpb/.deps
remove_dir lib/mpb/.libs
remove_dir lib/sgio/.deps
remove_dir lib/sgio/.libs
remove_dir lib/orom/.deps
remove_dir lib/orom/.libs
remove_dir src/.deps
remove_dir src/.libs
remove_dir tools/.deps
remove_dir tools/.libs
remove_dir ut/.deps
remove_dir ut/.libs

echo "** Removing Makefiles..."
remove_file Makefile
remove_file Makefile.in
remove_file src/Makefile
remove_file src/Makefile.in
remove_file lib/Makefile
remove_file lib/Makefile.in
remove_file lib/efi/Makefile
remove_file lib/efi/Makefile.in
remove_file lib/engine/Makefile
remove_file lib/engine/Makefile.in
remove_file lib/log/Makefile
remove_file lib/log/Makefile.in
remove_file lib/sgio/Makefile
remove_file lib/sgio/Makefile.in
remove_file lib/mpb/Makefile
remove_file lib/mpb/Makefile.in
remove_file lib/orom/Makefile
remove_file lib/orom/Makefile.in
remove_file tools/Makefile
remove_file tools/Makefile.in
remove_file include/Makefile
remove_file include/Makefile.in
remove_file ut/Makefile
remove_file ut/Makefile.in
remove_file contrib/dist/debian/Makefile
remove_file contrib/dist/debian/Makefile.in
remove_file contrib/dist/Makefile
remove_file contrib/dist/Makefile.in
remove_file contrib/Makefile
remove_file contrib/Makefile.in
remove_file doc/Makefile
remove_file doc/Makefile.in

echo "** Removing configuration files..."
remove_dir autom4te.cache
remove_dir config
remove_file aclocal.m4
remove_file config.h
remove_file config.h.in
remove_file config.log
remove_file config.status
remove_file configure
remove_file libtool
remove_file stamp-h1
remove_file contrib/dist/pkginfo
remove_file ssi.pc
remove_file config.guess
remove_file compile
remove_file config.sub
remove_file depcomp
remove_file install-sh
remove_slink ltmain.sh
remove_file missing

echo "** Cleaning up 'm4' directory..."
remove_slink m4/libtool.m4
remove_slink m4/ltoptions.m4
remove_slink m4/ltsugar.m4
remove_slink m4/ltversion.m4
remove_slink m4/lt~obsolete.m4

echo "** Cleaning up the backup files..."
find . \( -name "*~" -or -name "*.swp" -or -name "#*#" \) -delete -printf "--> %f"
