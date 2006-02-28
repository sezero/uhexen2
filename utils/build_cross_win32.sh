#!/bin/sh

UHEXEN2_TOP=..
. $UHEXEN2_TOP/scripts/cross_defs

BIN_FILES="dcc/bin/dhcc.exe h2_utils/bin/hcc.exe h2mp_utils/bin/hcc.exe
jsh2color/jsh2colour.exe
h2_utils/bin/vis.exe h2_utils/bin/qbsp.exe h2_utils/bin/light.exe
h2mp_utils/bin/vis.exe h2mp_utils/bin/qbsp.exe h2mp_utils/bin/light.exe
h2mp_utils/bin/bspinfo.exe
h2mp_utils/bin/qfiles.exe h2mp_utils/bin/genmodel.exe"

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
make -s -C h2mp_utils/hcc clean
make -s -C h2_utils/hcc clean
make -s -C h2mp_utils/qfiles clean
make -s -C h2mp_utils/genmodel clean
make -s -C h2mp_utils/utils clean
make -s -C h2_utils/utils clean
make -s -C dcc clean
make -s -C jsh2color clean
exit 0
fi

echo "Building hcc, the HexenC compiler.."
make -C h2mp_utils/hcc $SENDARGS

echo "" && echo "Now building hcc, old version"
make -C h2_utils/hcc $SENDARGS

echo "" && echo "Now building qfiles.."
make -C h2mp_utils/qfiles $SENDARGS

echo "" && echo "Now building genmodel.."
make -C h2mp_utils/genmodel $SENDARGS

echo "" && echo "Now building light, vis and qbsp.."
make -C h2mp_utils/utils $SENDARGS

echo "" && echo "Now building light, vis and qbsp, old versions.."
make -C h2_utils/utils $SENDARGS

echo "" && echo "Now building dhcc, a progs.dat decompiler.."
make -C dcc $SENDARGS

echo "" && echo "Now building jsh2colour, a lit file generator.."
make -C jsh2color $SENDARGS

