#! /bin/sh

MAKEFILE=Makefile.packaging

make -f $MAKEFILE clean
make -f $MAKEFILE glmp_static
#make -f $MAKEFILE clean
make -f $MAKEFILE glmp_dynamic
make -f $MAKEFILE clean
make -f $MAKEFILE glh2_static
#make -f $MAKEFILE clean
make -f $MAKEFILE glh2_dynamic 
make -f $MAKEFILE clean
make -f $MAKEFILE mp_static
#make -f $MAKEFILE clean 
make -f $MAKEFILE mp_dynamic
make -f $MAKEFILE clean
make -f $MAKEFILE h2_static
#make -f $MAKEFILE clean
make -f $MAKEFILE h2_dynamic
