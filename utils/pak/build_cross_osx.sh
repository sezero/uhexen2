#!/bin/sh

rm -f	pakx.ppc \
	pakx.x86 \
	pakx.x86_64 \
	pakx.bin \
	paklist.ppc \
	paklist.x86 \
	paklist.x86_64 \
	paklist.bin
make distclean

OLDPATH=$PATH
MAKE_CMD=make

OSXBUILD=1
export OSXBUILD

# ppc
PATH=/opt/cross_osx-ppc/bin:$OLDPATH
CC=powerpc-apple-darwin9-gcc
AS=powerpc-apple-darwin9-as
AR=powerpc-apple-darwin9-ar
RANLIB=powerpc-apple-darwin9-ranlib
LIPO=powerpc-apple-darwin9-lipo
export PATH CC AS AR RANLIB LIPO
$MAKE_CMD MACH_TYPE=ppc $* || exit 1
powerpc-apple-darwin9-strip -S pakx || exit 1
powerpc-apple-darwin9-strip -S paklist || exit 1
mv pakx pakx.ppc || exit 1
mv paklist paklist.ppc || exit 1
$MAKE_CMD distclean

# x86
PATH=/opt/cross_osx-x86/bin:$OLDPATH
CC=i686-apple-darwin9-gcc
AS=i686-apple-darwin9-as
AR=i686-apple-darwin9-ar
RANLIB=i686-apple-darwin9-ranlib
LIPO=i686-apple-darwin9-lipo
export PATH CC AS AR RANLIB LIPO
$MAKE_CMD MACH_TYPE=x86 $* || exit 1
i686-apple-darwin9-strip -S pakx || exit 1
i686-apple-darwin9-strip -S paklist || exit 1
mv pakx pakx.x86 || exit 1
mv paklist paklist.x86 || exit 1
$MAKE_CMD distclean

# x86_64
PATH=/opt/cross_osx-x86_64/usr/bin:$OLDPATH
CC=x86_64-apple-darwin9-gcc
AS=x86_64-apple-darwin9-as
AR=x86_64-apple-darwin9-ar
RANLIB=x86_64-apple-darwin9-ranlib
LIPO=x86_64-apple-darwin9-lipo
export PATH CC AS AR RANLIB LIPO
$MAKE_CMD MACH_TYPE=x86_64 $* || exit 1
x86_64-apple-darwin9-strip -S pakx || exit 1
x86_64-apple-darwin9-strip -S paklist || exit 1
mv pakx pakx.x86_64 || exit 1
mv paklist paklist.x86_64 || exit 1
$MAKE_CMD distclean

$LIPO -create -o pakx.bin pakx.ppc pakx.x86 pakx.x86_64 || exit 1
$LIPO -create -o paklist.bin paklist.ppc paklist.x86 paklist.x86_64 || exit 1
