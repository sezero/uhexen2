#!/bin/sh

PREFIX=/usr/local/cross-tools
TARGET=i386-mingw32msvc
PATH="$PREFIX/bin:$PREFIX/$TARGET/bin:$PATH"
export PATH
MAKEFILE=Makefile.mingw

if [ "$1" = "all" ]
then
make -f $MAKEFILE clean
make -f $MAKEFILE WIN32CC=$TARGET-gcc WIN32AS=nasm WIN32RES=$TARGET-windres W32STRIP=$TARGET-strip MINGWDIR=$PREFIX/$TARGET $2 $3 $4 glh2_exe
make -f $MAKEFILE clean
make -f $MAKEFILE WIN32CC=$TARGET-gcc WIN32AS=nasm WIN32RES=$TARGET-windres W32STRIP=$TARGET-strip MINGWDIR=$PREFIX/$TARGET $2 $3 $4 glmp_exe
make -f $MAKEFILE clean
make -f $MAKEFILE WIN32CC=$TARGET-gcc WIN32AS=nasm WIN32RES=$TARGET-windres W32STRIP=$TARGET-strip MINGWDIR=$PREFIX/$TARGET $2 $3 $4 h2_exe
make -f $MAKEFILE clean
make -f $MAKEFILE WIN32CC=$TARGET-gcc WIN32AS=nasm WIN32RES=$TARGET-windres W32STRIP=$TARGET-strip MINGWDIR=$PREFIX/$TARGET $2 $3 $4 mp_exe
make -f $MAKEFILE clean
else
exec make -f $MAKEFILE WIN32CC=$TARGET-gcc WIN32AS=nasm WIN32RES=$TARGET-windres W32STRIP=$TARGET-strip MINGWDIR=$PREFIX/$TARGET $*
fi
