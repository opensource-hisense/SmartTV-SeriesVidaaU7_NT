#!/bin/sh

libtoolize -f
echo "Running autoconf & automake"
autoreconf --install
autoconf
automake --add-missing

rm -rf compile config.h aclocal.m4 config.guess INSTALL config.status libtool
find -type f -name "Makefile" -exec rm -rf {} \;
find -type f -name "*.la" -exec rm -rf {} \;
find -type f -name "*.lo" -exec rm -rf {} \;
find -type d -name ".deps" -exec rm -rf {} \; -prune
find -type d -name ".libs" -exec rm -rf {} \; -prune
git add -u .

find -type f -name "Makefile.in" -exec git add -f \{\} \;
git add -f -n --ignore-missing configure config.h.in config.sub install-sh decomp missing
cp --remove-destination $(realpath ltmain.sh) .
git add ltmain.sh

git status
echo " (Makefile.in / configure) generated! Please commit to gerrit!"
