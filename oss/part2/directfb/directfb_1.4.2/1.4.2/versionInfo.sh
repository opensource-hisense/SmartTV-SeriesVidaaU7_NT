#!/bin/sh

#get the last change-id, and insert the info to lib
rm -rf version_info

BUILDHASH=`git log --pretty=format:"%h" -1`
BUILDDATE=`date +%Y%m%d%H`

echo "LIBCODE:DFB" >> version_info
echo "LIBVER:1.4.2" >> version_info
echo "BUILDHASH:${BUILDHASH}" >> version_info
echo "BUILDDATE:${BUILDDATE}" >> version_info