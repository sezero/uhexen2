#!/bin/sh

UHEXEN2_TOP=..
. $UHEXEN2_TOP/scripts/cross_defs

if test "$1" = "strip"; then
	echo "Stripping all HexenWorld binaries"
	$STRIPPER Master/hwmaster.exe	\
		Server/hwsv.exe	\
		Client/hwcl.exe	\
		Client/glhwcl.exe
	exit 0
fi

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

if test "$1" = "clean"; then
	$MAKE_CMD -s -C Client clean
	$MAKE_CMD -s -C Master clean
	$MAKE_CMD -s -C Server clean
	exit 0
fi

echo "Building HexenWorld Server"
$MAKE_CMD -C Server $* || exit 1

echo "" && echo "Building HexenWorld Master Server"
$MAKE_CMD -C Master $* || exit 1

echo "" && echo "Building HexenWorld Client (Software renderer)"
$MAKE_CMD -C Client $* hw || exit 1

echo "" && echo "Building HexenWorld Client (OpenGL renderer)"
$MAKE_CMD -C Client clean
$MAKE_CMD -C Client $* glhw || exit 1

