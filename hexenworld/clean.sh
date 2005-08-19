#!/bin/sh

if test "`uname`" = "FreeBSD" ; then
	MAKE=gmake
else
	MAKE=make
fi

$MAKE -C Server clean
$MAKE -C Master clean
$MAKE -C Client -f Makefile.unix clean

