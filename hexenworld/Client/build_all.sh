#! /bin/sh

MAKEFILE=Makefile.packaging

make -f $MAKEFILE clean
make -f $MAKEFILE hw_dynamic 
make -f $MAKEFILE clean
make -f $MAKEFILE glhw_dynamic
make -f $MAKEFILE clean
