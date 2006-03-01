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

if [ "$1" = "clean" ]
then
make -s -C hcc clean
make -s -C maputils clean
make -s -C genmodel clean
make -s -C qfiles clean
make -s -C dcc clean
make -s -C jsh2color clean
make -s -C hcc_old clean
exit 0
fi

echo "Building hcc, the HexenC compiler.."
make -C hcc $SENDARGS

echo "" && echo "Now building hcc, old version"
make -C hcc_old $SENDARGS

echo "" && echo "Now building qfiles.."
make -C qfiles $SENDARGS

echo "" && echo "Now building genmodel.."
make -C genmodel $SENDARGS

echo "" && echo "Now building light, vis and qbsp.."
make -C maputils $SENDARGS

echo "" && echo "Now building dhcc, a progs.dat decompiler.."
make -C dcc $SENDARGS

echo "" && echo "Now building jsh2colour, a lit file generator.."
make -C jsh2color $SENDARGS

