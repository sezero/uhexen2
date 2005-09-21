#!/bin/sh

# Valid arguments:
# clean : performs a "make clean" operation in all utils directories.
# strip : performs a "make strip" in all utils directories (strips all pre-built binaries)
# no args : builds all util binaries for win32

PREFIX=/usr/local/cross-tools
TARGET=i386-mingw32msvc
PATH="$PREFIX/bin:$PREFIX/$TARGET/bin:$PATH"
export PATH

MAKEFILE=Makefile.mingw

echo "Building hcc, the HexenC compiler.."
make -C h2mp_utils/hcc -f $MAKEFILE WIN32CC=$TARGET-gcc W32STRIP=$TARGET-strip MINGWDIR=$TARGET $*

echo "" && echo "Now building hcc, old version"
make -C h2_utils/hcc -f $MAKEFILE WIN32CC=$TARGET-gcc W32STRIP=$TARGET-strip MINGWDIR=$TARGET $*

echo "" && echo "Now building qfiles.."
make -C h2mp_utils/qfiles -f $MAKEFILE WIN32CC=$TARGET-gcc W32STRIP=$TARGET-strip MINGWDIR=$TARGET $*

echo "" && echo "Now building genmodel.."
make -C h2mp_utils/genmodel -f $MAKEFILE WIN32CC=$TARGET-gcc W32STRIP=$TARGET-strip MINGWDIR=$TARGET $*

echo "" && echo "Now building light, vis and qbsp.."
make -C h2mp_utils/utils -f $MAKEFILE WIN32CC=$TARGET-gcc W32STRIP=$TARGET-strip MINGWDIR=$TARGET $*

echo "" && echo "Now building light, vis and qbsp, old versions.."
make -C h2_utils/utils -f $MAKEFILE WIN32CC=$TARGET-gcc W32STRIP=$TARGET-strip MINGWDIR=$TARGET $*

echo "" && echo "Now building dhcc, a progs.dat decompiler.."
make -C dcc -f $MAKEFILE WIN32CC=$TARGET-gcc W32STRIP=$TARGET-strip MINGWDIR=$TARGET $*

