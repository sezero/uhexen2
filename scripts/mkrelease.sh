#!/bin/sh

# the script I use to prepare a release tree from the H.o.T. CVS snapshot

if echo `pwd` | grep "\/scripts" &>/dev/null ; then
cd ..
fi
# this relies on the name of this script file being "mkrelease.sh"
if [[ ! -f scripts/mkrelease.sh ]] ; then
echo "change into the cvs snapshot directory before running this script"
exit 1
fi

# kill the unwanted CVS* directories
rm -rf CVSROOT
find -name CVS | xargs rm -rf 

# move the spec file to the root
mv packaging/hexen2.spec ./
rm -rf packaging

# change all shell scripts' permissions
mv utils/build utils/build.sh
find -name *.sh | xargs chmod 755 

# move the unused/reference asm files directory into 00_unused
mv asm 00_unused

# rename the weird xdelta files
mv gamecode/pak_v111/patchdata/data1/pak0.pak.103_111 gamecode/pak_v111/patchdata/data1/data1pak0.xd
mv gamecode/pak_v111/patchdata/data1/pak1.pak.103_111 gamecode/pak_v111/patchdata/data1/data1pak1.xd
patch gamecode/pak_v111/h2_103_111.dat scripts/patchdat.pat

