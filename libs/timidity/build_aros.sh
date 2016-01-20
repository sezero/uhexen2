#!/bin/sh

# for x86-AROS cross-builds

UHEXEN2_TOP=../..
. $UHEXEN2_TOP/scripts/cross_defs.aros

HOST_OS=`uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'`
case "$HOST_OS" in
freebsd|openbsd|netbsd)
	MAKE_CMD=gmake ;;
linux)	MAKE_CMD=make ;;
*)	MAKE_CMD=make ;;
esac

exec $MAKE_CMD $*

