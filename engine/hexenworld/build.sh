#!/bin/sh

if test "$1" = "strip"; then
	exe_ext=
	if env | grep -i windir > /dev/null; then
		exe_ext=".exe"
	fi
	strip	server/hwsv$exe_ext	\
		client/hwcl$exe_ext	\
		client/glhwcl$exe_ext
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
	$MAKE_CMD -s -C client clean
	$MAKE_CMD -s -C server clean
	exit 0
fi

echo "Building hexenworld server..."
$MAKE_CMD -C server || exit 1

echo "" && echo "Building hexenworld client (software renderer)"
$MAKE_CMD -C client hw || exit 1

echo "" && echo "Building hexenworld client (opengl renderer)"
$MAKE_CMD -s -C client localclean
$MAKE_CMD -C client glhw || exit 1

