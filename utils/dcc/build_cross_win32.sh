#!/bin/sh

UHEXEN2_TOP=../..
. $UHEXEN2_TOP/scripts/cross_defs

if [ "$1" = "strip" ]
then
	echo "Stripping dhcc.exe"
	$STRIPPER bin/dhcc.exe
	exit 0
fi

if [ "$1" = "clean" ]
then
	make -s clean
	exit 0
fi

exec make $SENDARGS $*

