#!/bin/sh

UHEXEN2_TOP=../..
. $UHEXEN2_TOP/scripts/cross_defs

if [ "$1" = "strip" ]
then
	echo "Stripping jsh2colour.exe"
	$STRIPPER ../bin/jsh2colour.exe
	exit 0
fi

if [ "$1" = "clean" ]
then
	make -s clean
	exit 0
fi

exec make $SENDARGS $*

