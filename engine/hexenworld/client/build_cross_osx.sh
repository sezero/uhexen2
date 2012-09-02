#!/bin/sh

rm -f	glhwcl.ppc hwcl.ppc \
	glhwcl.x86 hwcl.x86 \
	glhwcl.x86_64 hwcl.x86_64 \
	HexenWorld HexenWorld\ gl
make distclean

OLDPATH=$PATH
MAKE_CMD=make

# ppc
PATH=/opt/cross_osx-ppc/bin:$OLDPATH
CC=powerpc-apple-darwin9-gcc
AS=powerpc-apple-darwin9-as
AR=powerpc-apple-darwin9-ar
RANLIB=powerpc-apple-darwin9-ranlib
LIPO=powerpc-apple-darwin9-lipo
export PATH CC AS AR RANLIB LIPO
MAKE_FLAGS="MACH_TYPE=ppc TARGET_OS=unix HOST_OS=darwin"
$MAKE_CMD $MAKE_FLAGS glhw $* || exit 1
powerpc-apple-darwin9-strip -S glhwcl || exit 1
mv glhwcl glhwcl.ppc || exit 1
$MAKE_CMD distclean
$MAKE_CMD $MAKE_FLAGS hw $* || exit 1
powerpc-apple-darwin9-strip -S hwcl || exit 1
mv hwcl hwcl.ppc || exit 1
$MAKE_CMD distclean

# x86
PATH=/opt/cross_osx-x86/bin:$OLDPATH
CC=i686-apple-darwin9-gcc
AS=i686-apple-darwin9-as
AR=i686-apple-darwin9-ar
RANLIB=i686-apple-darwin9-ranlib
LIPO=i686-apple-darwin9-lipo
export PATH CC AS AR RANLIB LIPO
MAKE_FLAGS="MACH_TYPE=x86 TARGET_OS=unix HOST_OS=darwin"
$MAKE_CMD $MAKE_FLAGS glhw $* || exit 1
i686-apple-darwin9-strip -S glhwcl || exit 1
mv glhwcl glhwcl.x86 || exit 1
$MAKE_CMD distclean
$MAKE_CMD $MAKE_FLAGS hw $* || exit 1
i686-apple-darwin9-strip -S hwcl || exit 1
mv hwcl hwcl.x86 || exit 1
$MAKE_CMD distclean

# x86_64
PATH=/opt/cross_osx-x86_64/usr/bin:$OLDPATH
CC=x86_64-apple-darwin9-gcc
AS=x86_64-apple-darwin9-as
AR=x86_64-apple-darwin9-ar
RANLIB=x86_64-apple-darwin9-ranlib
LIPO=x86_64-apple-darwin9-lipo
export PATH CC AS AR RANLIB LIPO
MAKE_FLAGS="MACH_TYPE=x86_64 TARGET_OS=unix HOST_OS=darwin"
$MAKE_CMD $MAKE_FLAGS glhw $* || exit 1
x86_64-apple-darwin9-strip -S glhwcl || exit 1
mv glhwcl glhwcl.x86_64 || exit 1
$MAKE_CMD distclean
$MAKE_CMD $MAKE_FLAGS hw $* || exit 1
x86_64-apple-darwin9-strip -S hwcl || exit 1
mv hwcl hwcl.x86_64 || exit 1
$MAKE_CMD distclean

$LIPO -create -o HexenWorld\ gl glhwcl.ppc glhwcl.x86 glhwcl.x86_64 || exit 1
$LIPO -create -o HexenWorld hwcl.ppc hwcl.x86 hwcl.x86_64 || exit 1

