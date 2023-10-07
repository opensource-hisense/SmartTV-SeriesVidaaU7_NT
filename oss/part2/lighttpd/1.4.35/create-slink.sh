#!/bin/sh

rm -f libfcgi.so
rm -f libfcgi++.so
rm -f libfcgi.so.0
rm -f libfcgi++.so.0
rm -f libjansson.so
rm -f libjansson.so.4
rm -f libpcrecpp.so
rm -f libpcrecpp.so.0
rm -f libpcreposix.so
rm -f libpcreposix.so.0
rm -f libpcre.so
rm -f libpcre.so.1

ln -s libfcgi.so.0.0.0 libfcgi.so
ln -s libfcgi.so.0.0.0 libfcgi.so.0
ln -s libfcgi++.so.0.0.0 libfcgi++.so
ln -s libfcgi++.so.0.0.0 libfcgi++.so.0
ln -s libjansson.so.4.6.0 libjansson.so
ln -s libjansson.so.4.6.0 libjansson.so.4
ln -s libpcrecpp.so.0.0.0 libpcrecpp.so
ln -s libpcrecpp.so.0.0.0 libpcrecpp.so.0
ln -s libpcreposix.so.0.0.2 libpcreposix.so
ln -s libpcreposix.so.0.0.2 libpcreposix.so.0
ln -s libpcre.so.1.2.1 libpcre.so
ln -s libpcre.so.1.2.1 libpcre.so.1
