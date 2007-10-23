#!/bin/sh

# used for building h2dos.exe with djgpp cross toolchain

PREFIX=/usr/local/cross-djgpp
TARGET=i586-pc-msdosdjgpp

PATH="$PREFIX/bin:$PREFIX/$TARGET/bin:$PATH"
export PATH

CC="$TARGET-gcc"
AS="$TARGET-as"
AR="$TARGET-ar"
NASM="nasm"
DJGPPDIR="$PREFIX/$TARGET"
DOSBUILD=1
export CC NASM DJGPPDIR AS AR MINGWDIR DOSBUILD

HOST_OS=`uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'`

case "$HOST_OS" in
freebsd|openbsd|netbsd)
	MAKE_CMD=gmake
	;;
linux)
	MAKE_CMD=make
	;;
*)
	MAKE_CMD=make
	;;
esac

$MAKE_CMD $2 $3 $4 $5 $6 h2  || exit 1

