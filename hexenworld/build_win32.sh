#!/bin/sh

MAKE=make

echo "Building hexenworld server..."
$MAKE -C Server -f Makefile.mingw

echo "" && echo "Building hexenworld master server.."
$MAKE -C Master -f Makefile.mingw

echo "" && echo "Building hexenworld client (software renderer)"
$MAKE -C Client -f Makefile.mingw hw_exe

echo "" && echo "Building hexenworld client (opengl renderer)"
$MAKE -C Client -f Makefile.mingw clean
$MAKE -C Client -f Makefile.mingw glhw_exe
