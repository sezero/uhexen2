#!/bin/sh

MAKEFILE=Makefile.packaging

if test "`uname`" = "FreeBSD" ; then
	MAKE=gmake
else
	MAKE=make
fi

$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE hw_dynamic 
$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE glhw_dynamic
$MAKE -f $MAKEFILE clean

