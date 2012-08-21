#!/bin/sh

rm -f	glhexen2.ppc hexen2.ppc \
	glhexen2.x86 hexen2.x86 \
	glhexen2.x86_64 hexen2.x86_64 \
	Hexen\ II Hexen\ II\ gl
make distclean

OLDPATH=$PATH
MAKE_CMD=make

# ppc
PATH=/opt/cross_osx-ppc/bin:$OLDPATH
CC=powerpc-apple-darwin9-gcc
AR=powerpc-apple-darwin9-ar
RANLIB=powerpc-apple-darwin9-ranlib
LIPO=powerpc-apple-darwin9-lipo
export PATH CC AR RANLIB LIPO
MAKE_FLAGS="MACH_TYPE=ppc TARGET_OS=unix HOST_OS=darwin"
$MAKE_CMD $MAKE_FLAGS glh2 $* || exit 1
powerpc-apple-darwin9-strip -S glhexen2 || exit 1
mv glhexen2 glhexen2.ppc || exit 1
$MAKE_CMD distclean
$MAKE_CMD $MAKE_FLAGS h2 $* || exit 1
powerpc-apple-darwin9-strip -S hexen2 || exit 1
mv hexen2 hexen2.ppc || exit 1
$MAKE_CMD distclean

# x86
PATH=/opt/cross_osx-x86/bin:$OLDPATH
CC=i686-apple-darwin9-gcc
AR=i686-apple-darwin9-ar
RANLIB=i686-apple-darwin9-ranlib
LIPO=i686-apple-darwin9-lipo
export PATH CC AR RANLIB LIPO
MAKE_FLAGS="MACH_TYPE=x86 TARGET_OS=unix HOST_OS=darwin"
$MAKE_CMD $MAKE_FLAGS glh2 $* || exit 1
i686-apple-darwin9-strip -S glhexen2 || exit 1
mv glhexen2 glhexen2.x86 || exit 1
$MAKE_CMD distclean
$MAKE_CMD $MAKE_FLAGS h2 $* || exit 1
i686-apple-darwin9-strip -S hexen2 || exit 1
mv hexen2 hexen2.x86 || exit 1
$MAKE_CMD distclean

# x86_64
PATH=/opt/cross_osx-x86_64/usr/bin:$OLDPATH
CC=x86_64-apple-darwin9-gcc
AR=x86_64-apple-darwin9-ar
RANLIB=x86_64-apple-darwin9-ranlib
LIPO=x86_64-apple-darwin9-lipo
export PATH CC AR RANLIB LIPO
MAKE_FLAGS="MACH_TYPE=x86_64 TARGET_OS=unix HOST_OS=darwin"
$MAKE_CMD $MAKE_FLAGS glh2 $* || exit 1
x86_64-apple-darwin9-strip -S glhexen2 || exit 1
mv glhexen2 glhexen2.x86_64 || exit 1
$MAKE_CMD distclean
$MAKE_CMD $MAKE_FLAGS h2 $* || exit 1
x86_64-apple-darwin9-strip -S hexen2 || exit 1
mv hexen2 hexen2.x86_64 || exit 1
$MAKE_CMD distclean

$LIPO -create -o Hexen\ II\ gl glhexen2.ppc glhexen2.x86 glhexen2.x86_64 || exit 1
$LIPO -create -o Hexen\ II hexen2.ppc hexen2.x86 hexen2.x86_64 || exit 1

