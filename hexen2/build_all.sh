#!/bin/sh

MAKEFILE=Makefile.packaging

if test "`uname`" = "FreeBSD" ; then
	MAKE=gmake
else
	MAKE=make
fi

$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE h2_dynamic
$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE glh2_dynamic 
$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE mp_dynamic
$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE glmp_dynamic
$MAKE -f $MAKEFILE clean

