#!/bin/sh

LIBTOOL_REQUIRED_VERSION=1.3.4
AUTOCONF_REQUIRED_VERSION=2.13
AUTOMAKE_REQUIRED_VERSION=1.4
PROJECT="DirectFB"

make_version ()
{
    major=`echo $1 | cut -d '.' -f 1`
    minor=`echo $1 | cut -d '.' -f 2`
    micro=`echo $1 | cut -d '.' -f 3`

    expr $major \* 65536 + $minor \* 256 + $micro
}

check_version ()
{
    ver=`make_version $1.0.0`
    req=`make_version $2.0.0`

    if test $ver -ge $req; then
        echo "yes (version $1)"
    else
        echo "Too old (found version $1)!"
        DIE=1
    fi
}

echo -n "checking for libtool >= $LIBTOOL_REQUIRED_VERSION ... "
if (libtoolize --version) < /dev/null > /dev/null 2>&1; then
    VER=`libtoolize --version \
         | grep libtool | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $LIBTOOL_REQUIRED_VERSION
else
    echo
    echo "  You must have libtool installed to compile $PROJECT."
    echo "  Install the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    DIE=1;
fi

echo -n "checking for autoconf >= $AUTOCONF_REQUIRED_VERSION ... "
if (autoconf --version) < /dev/null > /dev/null 2>&1; then
    VER=`autoconf --version \
         | head -n1 | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOCONF_REQUIRED_VERSION
else
    echo
    echo "  You must have autoconf installed to compile $PROJECT."
    echo "  Download the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    DIE=1;
fi

echo -n "checking for automake >= $AUTOMAKE_REQUIRED_VERSION ... "
if (automake --version) < /dev/null > /dev/null 2>&1; then
    VER=`automake --version \
         | grep automake | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOMAKE_REQUIRED_VERSION
else
    echo
    echo "  You must have automake installed to compile $PROJECT."
    echo "  Get ftp://ftp.cygnus.com/pub/home/tromey/automake-1.4p1.tar.gz"
    echo "  (or a newer version if it is available)"
    DIE=1
fi

if [ "$DIE" = 1 ]; then
    echo
    echo "Please install/upgrade the missing tools and call me again."
    echo
else
    echo
    echo "Autotool version satisfied!"
    echo
fi
