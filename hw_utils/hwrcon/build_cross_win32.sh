#!/bin/sh

UHEXEN2_TOP=../..
. $UHEXEN2_TOP/scripts/cross_defs

if [ "$1" = "strip" ]
then
$STRIPPER hwrcon.exe hwterm.exe
exit 0
fi

exec make $SENDARGS $*

