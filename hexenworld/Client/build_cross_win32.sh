#!/bin/sh

UHEXEN2_TOP=../..
. $UHEXEN2_TOP/scripts/cross_defs

if [ "$1" = "strip" ]
then
$STRIPPER hwcl.exe glhwcl.exe
exit 0
fi

if [ "$1" = "all" ]
then
make clean
make $SENDARGS $2 $3 $4 glhw
make clean
make $SENDARGS $2 $3 $4 hw
make clean
exit 0
fi

exec make $SENDARGS $*

