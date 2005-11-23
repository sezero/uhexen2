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

if [ "$1" = "clean" ]
then
make -s -C Client clean
make -s -C Master clean
make -s -C Server clean
exit 0
fi

echo "Building HexenWorld Server"
make -C Server $SENDARGS $*

echo "" && echo "Building HexenWorld Master Server"
make -C Master $SENDARGS $*

echo "" && echo "Building HexenWorld Client (Software renderer)"
make -C Client $SENDARGS $* hw

echo "" && echo "Building HexenWorld Client (OpenGL renderer)"
make -C Client clean
make -C Client $SENDARGS $* glhw

