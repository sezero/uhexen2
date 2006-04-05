#!/bin/sh

UHEXEN2_TOP=..
. $UHEXEN2_TOP/scripts/cross_defs

if [ "$1" = "strip" ]
then
$STRIPPER h2.exe glh2.exe h2mp.exe glh2mp.exe
exit 0
fi

if [ "$1" = "all" ]
then
make clean
make $SENDARGS $2 $3 $4 h2
make clean
make $SENDARGS $2 $3 $4 glh2
make clean
exit 0
fi

exec make $SENDARGS $*

