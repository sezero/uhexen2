#!/bin/sh

UHEXEN2_TOP=..
. $UHEXEN2_TOP/scripts/cross_defs

BIN_FILES="bin/hcc.exe bin/vis.exe bin/qbsp.exe bin/light.exe
bin/qfiles.exe bin/genmodel.exe bin/bspinfo.exe
bin/jsh2colour.exe
bin/dhcc.exe hcc_old/hcc.exe"

if [ "$1" = "strip" ]
then
echo "Stripping all hexen2-util binaries"
	for i in ${BIN_FILES}
	do
	    $STRIPPER ${i}${EXE_EXT}
	done
exit 0
fi

HOST_OS=`uname`

case "$HOST_OS" in
FreeBSD|OpenBSD|NetBSD)
	MAKE_CMD=gmake
	;;
Linux)
	MAKE_CMD=make
	;;
*)
	MAKE_CMD=make
	;;
esac

if [ "$1" = "clean" ]
then
$MAKE_CMD -s -C hcc clean
$MAKE_CMD -s -C maputils clean
$MAKE_CMD -s -C genmodel clean
$MAKE_CMD -s -C qfiles clean
$MAKE_CMD -s -C dcc clean
$MAKE_CMD -s -C jsh2color clean
$MAKE_CMD -s -C hcc_old clean
exit 0
fi

echo "Building hcc, the HexenC compiler.."
$MAKE_CMD -C hcc $SENDARGS

echo "" && echo "Now building hcc, old version"
$MAKE_CMD -C hcc_old $SENDARGS

echo "" && echo "Now building qfiles.."
$MAKE_CMD -C qfiles $SENDARGS

echo "" && echo "Now building genmodel.."
$MAKE_CMD -C genmodel $SENDARGS

echo "" && echo "Now building light, vis and qbsp.."
$MAKE_CMD -C maputils $SENDARGS

echo "" && echo "Now building dhcc, a progs.dat decompiler.."
$MAKE_CMD -C dcc $SENDARGS

echo "" && echo "Now building jsh2colour, a lit file generator.."
$MAKE_CMD -C jsh2color $SENDARGS

