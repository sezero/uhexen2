#!/bin/sh

HOST_OS=`uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'`
case "$HOST_OS" in
freebsd|openbsd|netbsd)
	MAKE_CMD=gmake ;;
linux)	MAKE_CMD=make ;;
*)	MAKE_CMD=make ;;
esac

$MAKE_CMD clean
$MAKE_CMD h2 $* || exit 1

$MAKE_CMD localclean
$MAKE_CMD glh2 USE_X86_ASM=no $* || exit 1

$MAKE_CMD -C server clean
$MAKE_CMD -C server $* || exit 1

$MAKE_CMD clean
$MAKE_CMD -C server clean

