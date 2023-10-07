procmem_linux_x86_port
======================

procmem is available in android (Source: https://android.googlesource.com/platform/system/extras/+/master/procmem/). I derived the source code from Android and &amp; ported to Ubuntu Linux. Except the Makefile, rest of the files are almost same. Hope it helps.
[How to build]
export CROSS_COMPILE=/mtkoss/gnuarm/gcc-arm-linux-gnu-5.5.0-ubuntu/x86_64/bin/arm-linux-gnueabi- ;  mosesq make clean ; mosesq make procmem

$(CROSS_COMPILE)readelf -p .comment procrank
