#!/bin/sh

UHEXEN2_TOP=..
. $UHEXEN2_TOP/scripts/cross_defs

BIN_FILES="Master/hwmaster.exe Server/hwsv.exe Client/hwcl.exe Client/glhwcl.exe"

if [ "$1" = "strip" ]
then
echo "Stripping all HexenWorld binaries"
	for i in ${BIN_FILES}
	do
	    $STRIPPER ${i}${EXE_EXT}
	done
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

if [ "$1" = "clean" ]
then
$MAKE_CMD -s -C Client clean
$MAKE_CMD -s -C Master clean
$MAKE_CMD -s -C Server clean
exit 0
fi

echo "Building HexenWorld Server"
$MAKE_CMD -C Server $SENDARGS $* || exit 1

echo "" && echo "Building HexenWorld Master Server"
$MAKE_CMD -C Master $SENDARGS $* || exit 1

echo "" && echo "Building HexenWorld Client (Software renderer)"
$MAKE_CMD -C Client $SENDARGS $* hw || exit 1

echo "" && echo "Building HexenWorld Client (OpenGL renderer)"
$MAKE_CMD -C Client clean
$MAKE_CMD -C Client $SENDARGS $* glhw || exit 1

