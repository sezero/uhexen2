#!/bin/sh

rm -f	h2patch.ppc \
	h2patch.x86 \
	h2patch.x86_64 \
	h2patch.bin
make -f Makefile.darwin distclean

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
$MAKE_CMD -f Makefile.darwin MACH_TYPE=ppc $* || exit 1
powerpc-apple-darwin9-strip -S h2patch || exit 1
mv h2patch h2patch.ppc || exit 1
$MAKE_CMD -f Makefile.darwin distclean

# x86
PATH=/opt/cross_osx-x86/bin:$OLDPATH
CC=i686-apple-darwin9-gcc
AS=i686-apple-darwin9-as
AR=i686-apple-darwin9-ar
RANLIB=i686-apple-darwin9-ranlib
LIPO=i686-apple-darwin9-lipo
export PATH CC AS AR RANLIB LIPO
$MAKE_CMD -f Makefile.darwin MACH_TYPE=x86 $* || exit 1
i686-apple-darwin9-strip -S h2patch || exit 1
mv h2patch h2patch.x86 || exit 1
$MAKE_CMD -f Makefile.darwin distclean

# x86_64
PATH=/opt/cross_osx-x86_64/usr/bin:$OLDPATH
CC=x86_64-apple-darwin9-gcc
AS=x86_64-apple-darwin9-as
AR=x86_64-apple-darwin9-ar
RANLIB=x86_64-apple-darwin9-ranlib
LIPO=x86_64-apple-darwin9-lipo
export PATH CC AS AR RANLIB LIPO
$MAKE_CMD -f Makefile.darwin MACH_TYPE=x86_64 $* || exit 1
x86_64-apple-darwin9-strip -S h2patch || exit 1
mv h2patch h2patch.x86_64 || exit 1
$MAKE_CMD -f Makefile.darwin distclean

$LIPO -create -o h2patch.bin h2patch.ppc h2patch.x86 h2patch.x86_64 || exit 1
