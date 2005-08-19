#!/bin/sh

MAKE=make
MAKEFILE=Makefile.mingw

$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE hw_exe
$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE glhw_exe
$MAKE -f $MAKEFILE clean

