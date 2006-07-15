#!/bin/sh

EXE_EXT=
BIN_FILES="bin/hcc bin/vis bin/qbsp bin/light 
bin/qfiles bin/genmodel bin/bspinfo
bin/jsh2colour
bin/dhcc hcc_old/hcc"

if env | grep -i windir > __tmp.tmp; then
EXE_EXT=".exe";
fi
rm -f __tmp.tmp

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
$MAKE_CMD -C hcc
echo "" && echo "Now building hcc, old version"
$MAKE_CMD -C hcc_old
echo "" && echo "Now building qfiles.."
$MAKE_CMD -C qfiles
echo "" && echo "Now building genmodel.."
$MAKE_CMD -C genmodel
echo "" && echo "Now building light, vis and qbsp.."
$MAKE_CMD -C maputils
echo "" && echo "Now building dhcc, a progs.dat decompiler.."
$MAKE_CMD -C dcc
echo "" && echo "Now building jsh2colour, a lit file generator.."
$MAKE_CMD -C jsh2color

