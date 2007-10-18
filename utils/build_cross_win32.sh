#!/bin/sh

UHEXEN2_TOP=..
. $UHEXEN2_TOP/scripts/cross_defs

BIN_DIR=bin

if test "$1" = "strip"; then
	echo "Stripping all hexen2-util binaries"
	$STRIPPER hcc_old/hcc.exe	\
		$BIN_DIR/hcc.exe	\
		$BIN_DIR/dhcc.exe	\
		$BIN_DIR/vis.exe $BIN_DIR/light.exe	\
		$BIN_DIR/qbsp.exe $BIN_DIR/bspinfo.exe	\
		$BIN_DIR/qfiles.exe	\
		$BIN_DIR/genmodel.exe	\
		$BIN_DIR/jsh2colour.exe	\
		$BIN_DIR/bsp2wal.exe $BIN_DIR/lmp2pcx.exe
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
	$MAKE_CMD -s -C hcc clean
	$MAKE_CMD -s -C maputils clean
	$MAKE_CMD -s -C genmodel clean
	$MAKE_CMD -s -C qfiles clean
	$MAKE_CMD -s -C dcc clean
	$MAKE_CMD -s -C jsh2color clean
	$MAKE_CMD -s -C hcc_old clean
	$MAKE_CMD -s -C texutils/bsp2wal clean
	$MAKE_CMD -s -C texutils/lmp2pcx clean
	exit 0
fi

echo "Building hcc, the HexenC compiler.."
$MAKE_CMD -C hcc || exit 1

echo "" && echo "Now building hcc, old version"
$MAKE_CMD -C hcc_old || exit 1

echo "" && echo "Now building qfiles.."
$MAKE_CMD -C qfiles || exit 1

echo "" && echo "Now building genmodel.."
$MAKE_CMD -C genmodel || exit 1

echo "" && echo "Now building light, vis and qbsp.."
$MAKE_CMD -C maputils || exit 1

echo "" && echo "Now building dhcc, a progs.dat decompiler.."
$MAKE_CMD -C dcc || exit 1

echo "" && echo "Now building jsh2colour, a lit file generator.."
$MAKE_CMD -C jsh2color || exit 1

echo "" && echo "Now building texutils.."
$MAKE_CMD -C texutils/bsp2wal || exit 1
$MAKE_CMD -C texutils/lmp2pcx || exit 1

