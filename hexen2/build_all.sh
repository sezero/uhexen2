#!/bin/sh

HOST_OS=`uname`

case "$HOST_OS" in
FreeBSD)
	MAKE=gmake
	;;
OpenBSD)
	MAKE=gmake
	;;
NetBSD)
	MAKE=gmake
	;;
Linux)
	MAKE=make
	;;
*)
	MAKE=make
	;;
esac

$MAKE clean
$MAKE h2
$MAKE clean
$MAKE glh2
$MAKE clean
$MAKE -f Makefile.sv
$MAKE clean

