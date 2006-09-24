#!/bin/sh

HOST_OS=`uname`

case "$HOST_OS" in
FreeBSD|OpenBSD|NetBSD)
	MAKE_CMD=gmake
	;;
Linux)
	MAKE_CMD=make
	;;
*)
	MAKE_CMD=make
	;;
esac

$MAKE_CMD clean
$MAKE_CMD h2 || exit 1
$MAKE_CMD clean
$MAKE_CMD glh2 || exit 1
$MAKE_CMD clean
$MAKE_CMD -f Makefile.sv || exit 1
$MAKE_CMD clean

