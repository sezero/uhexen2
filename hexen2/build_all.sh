#! /bin/sh

MAKEFILE=Makefile.packaging

if [ -f /usr/local/bin/make ]; then
	MAKE=/usr/local/bin/make
elif [ -f /usr/bin/make ]; then
	MAKE=/usr/bin/make
elif [ -f /usr/local/bin/gmake ]; then
	MAKE=/usr/local/bin/gmake
elif [ -f /usr/bin/gmake ]; then
	MAKE=/usr/bin/gmake
else
	echo "Error: make or gmake not found"
	exit 1
fi

$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE h2_dynamic
$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE glh2_dynamic 
$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE mp_dynamic
$MAKE -f $MAKEFILE clean
$MAKE -f $MAKEFILE glmp_dynamic
$MAKE -f $MAKEFILE clean
