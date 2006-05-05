#!/bin/sh

EXE_EXT=
BIN_FILES="Master/hwmaster Server/hwsv Client/hwcl Client/glhwcl"

if env | grep -i windir > __tmp.tmp; then
EXE_EXT=".exe";
fi
rm -f __tmp.tmp

HOST_OS=`uname`

case "$HOST_OS" in
FreeBSD)
	MAKE=gmake
	;;
OpenBSD)
	MAKE=gmake
	;;
NetBSD)
	MAKE=gmake
	;;
Linux)
	MAKE=make
	;;
*)
	MAKE=make
	;;
esac

if [ "$1" = "strip" ]
then
	for i in ${BIN_FILES}
	do
	    strip ${i}${EXE_EXT}
	done
exit 0
fi

if [ "$1" = "clean" ]
then
$MAKE -s -C Client clean
$MAKE -s -C Master clean
$MAKE -s -C Server clean
exit 0
fi

echo "Building hexenworld server..."
$MAKE -C Server

echo "" && echo "Building hexenworld master server.."
$MAKE -C Master

echo "" && echo "Building hexenworld client (software renderer)"
$MAKE -C Client hw

echo "" && echo "Building hexenworld client (opengl renderer)"
$MAKE -C Client clean
$MAKE -C Client glhw

