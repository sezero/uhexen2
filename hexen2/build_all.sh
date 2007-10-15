#!/bin/sh

HOST_OS=`uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'`

case "$HOST_OS" in
freebsd|openbsd|netbsd)
	MAKE_CMD=gmake
	;;
linux)
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

