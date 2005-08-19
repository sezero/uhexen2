#!/bin/sh

MAKE=make

$MAKE -C Server -f Makefile.mingw clean
$MAKE -C Master -f Makefile.mingw clean
$MAKE -C Client -f Makefile.mingw clean
