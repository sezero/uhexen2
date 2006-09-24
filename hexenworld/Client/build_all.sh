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
$MAKE_CMD hw || exit 1
$MAKE_CMD clean
$MAKE_CMD glhw || exit 1
$MAKE_CMD clean

