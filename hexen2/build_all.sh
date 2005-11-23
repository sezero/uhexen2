#!/bin/sh

if test "`uname`" = "FreeBSD" ; then
	MAKE=gmake
else
	MAKE=make
fi

$MAKE clean
$MAKE h2
$MAKE clean
$MAKE glh2
$MAKE clean
$MAKE mp
$MAKE clean
$MAKE glmp
$MAKE clean

