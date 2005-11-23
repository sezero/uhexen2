#!/bin/sh

EXE_EXT=
BIN_FILES="Master/hwmaster Server/hwsv Client/hwcl Client/glhwcl"

if env | grep -i windir > __tmp.tmp; then
EXE_EXT=".exe";
fi
rm -f __tmp.tmp

if test "`uname`" = "FreeBSD" ; then
	MAKE=gmake
else
	MAKE=make
fi

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
make -s -C Client clean
make -s -C Master clean
make -s -C Server clean
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

