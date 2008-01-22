#!/bin/sh

# the script I use to prepare a release tree from the H.o.T. CVS snapshot
# $Id: mkrelease.sh,v 1.17 2008-01-22 13:05:48 sezero Exp $

if echo `pwd` | grep "\/scripts" > /dev/null 2>&1 ; then
cd ..
fi
# this relies on the name of this script file being "mkrelease.sh"
test -f scripts/mkrelease.sh || { echo "change into the cvs snapshot directory before running this script"; exit 1; }

# the current gamecode version
GAMECODE_VER=1.19

# kill the unwanted CVS* directories
rm -rf CVSROOT
find . -name CVS | xargs rm -rf 

# kill the obsoleted empty directories
rm -rf utils/h2_utils utils/h2mp_utils utils/dcc/bin
rm -rf hexen2/obsolete hexenworld/Server/win_stuff hexenworld/Master/win_stuff launcher/obsolete
rm -rf gamecode/hc/h2_single_prog gamecode/hc/hw_bossmaps
rm -rf docs/activision
rm -rf asm/obsolete xdelta11/doc/dcc99

# move the spec file to the root
cp -p packaging/hexen2.spec ./hexen2.spec
rm -rf packaging

# move the license file to the root
mv docs/LICENSE ./LICENSE

# change all shell scripts' permissions
find . -name *.sh | xargs chmod 755 

# move the unused/reference asm files directory into 00_unused
mv asm 00_unused/asm_ref

# rename the gamecode directory to include its version number
mv gamecode gamecode-$GAMECODE_VER

