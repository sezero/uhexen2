#!/bin/sh

if test "$1" = "strip"; then
	exe_ext=
	if env | grep -i windir > /dev/null; then
		exe_ext=".exe"
	fi
	strip Master/hwmaster$exe_ext	\
		Server/hwsv$exe_ext	\
		Client/hwcl$exe_ext	\
		Client/glhwcl$exe_ext
	exit 0
fi

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

if test "$1" = "clean"; then
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

