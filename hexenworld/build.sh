#!/bin/sh

EXE_EXT=
BIN_FILES="Master/hwmaster Server/hwsv Client/hwcl Client/glhwcl"

if env | grep -i windir > __tmp.tmp; then
EXE_EXT=".exe";
fi
rm -f __tmp.tmp

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
$MAKE_CMD -s -C Client clean
$MAKE_CMD -s -C Master clean
$MAKE_CMD -s -C Server clean
exit 0
fi

echo "Building hexenworld server..."
$MAKE_CMD -C Server || exit 1

echo "" && echo "Building hexenworld master server.."
$MAKE_CMD -C Master || exit 1

echo "" && echo "Building hexenworld client (software renderer)"
$MAKE_CMD -C Client hw || exit 1

echo "" && echo "Building hexenworld client (opengl renderer)"
$MAKE_CMD -s -C Client clean
$MAKE_CMD -C Client glhw || exit 1

