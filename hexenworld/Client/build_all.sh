#!/bin/sh

if test "`uname`" = "FreeBSD" ; then
	MAKE=gmake
else
	MAKE=make
fi

$MAKE clean
$MAKE hw
$MAKE clean
$MAKE glhw
$MAKE clean

