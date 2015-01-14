#!/bin/sh

UHEXEN2_TOP=../..
. $UHEXEN2_TOP/scripts/cross_defs.morphos

if test "$1" = "strip"; then
	echo "Stripping all HexenWorld binaries"
	$STRIPPER -S server/hwsv client/hwcl client/glhwcl
	exit 0
fi

HOST_OS=`uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'`
case "$HOST_OS" in
freebsd|openbsd|netbsd)
	MAKE_CMD=gmake ;;
linux)	MAKE_CMD=make ;;
*)	MAKE_CMD=make ;;
esac

if test "$1" = "clean"; then
	$MAKE_CMD -C client clean
	$MAKE_CMD -C server clean
	exit 0
fi

echo "Building HexenWorld Server"
$MAKE_CMD -C server $* || exit 1
$MAKE_CMD -s -C server clean

echo "" && echo "Building HexenWorld Client (Software renderer)"
$MAKE_CMD -C client hw $* || exit 1
$MAKE_CMD -s -C client clean

echo "" && echo "Building HexenWorld Client (OpenGL renderer)"
$MAKE_CMD -C client glhw $* || exit 1
$MAKE_CMD -s -C client clean

