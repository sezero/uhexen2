#!/bin/sh

MAKE=make
MAKEFILE=Makefile.mingw

$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE h2_exe
$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE glh2_exe
$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE mp_exe
$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE glmp_exe
$MAKE -f $MAKEFILE clean

