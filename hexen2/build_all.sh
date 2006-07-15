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
$MAKE_CMD h2
$MAKE_CMD clean
$MAKE_CMD glh2
$MAKE_CMD clean
$MAKE_CMD -f Makefile.sv
$MAKE_CMD clean

