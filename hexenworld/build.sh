#!/bin/sh

if test "`uname`" = "FreeBSD" ; then
	MAKE=gmake
else
	MAKE=make
fi

echo "Building hexenworld server..."
$MAKE -C Server

echo "" && echo "Building hexenworld master server.."
$MAKE -C Master

echo "" && echo "Building hexenworld client (software renderer)"
$MAKE -C Client -f Makefile.unix hw_dynamic

echo "" && echo "Building hexenworld client (opengl renderer)"
$MAKE -C Client -f Makefile.unix clean
$MAKE -C Client -f Makefile.unix glhw_dynamic

