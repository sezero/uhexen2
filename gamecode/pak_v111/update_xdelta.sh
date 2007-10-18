#!/bin/sh
# Script for patching cdrom versions of Hexen II pak files to v1.11
# For use with Hammer of Thyrion v1.4.0 and probably laer versions.

NoXdelta()
{
	echo "Patch binary xdelta113 or xdelta114 not found."
	echo "You can compile it from its up-to-date source"
	echo "tarball downloadable from the Hammer of Thyrion"
	echo "website."
	exit 1
}

echo "=================================================="
echo "Hammer of Thyrion (http://uhexen2.sourceforge.net)"
echo "Patch script for Raven's Hexen II v1.11 PAK update"
echo ""

# here are the correct md5sums:
# cdrom-1.03 version:
pak0old="b53c9391d16134cb3baddc1085f18683"
pak1old="9a2010aafb9c0fe71c37d01292030270"
# updated 1.11 version:
pak0new="c9675191e75dd25a3b9ed81ee7e05eff"
pak1new="c2ac5b0640773eed9ebe1cda2eca2ad0"

# make sure the data1 directory exists
test -d "data1" || { echo "Error: data1 directory not found."; exit 1; }
# make sure we have write permissions
chmod 755 data1 > /dev/null 2>&1 || { echo "Error: No permissions to write to the data1 directory"; exit 1; }

# make sure we have one of the md5 tools
if md5sum $0 > /dev/null 2>&1; then
	md5cmd="md5sum"
elif  md5 $0 > /dev/null 2>&1; then
	md5cmd="md5"
else
	echo "Error: Neither md5 nor md5sum utility installed."
	exit 1
fi

# make sure our xdelta-1.1.4 exists: people may have
# newer versions of xdelta which shall not work with
# our delta files. We use our patched 1.1.4 version,
# configured with --disable-shared, and the binary
# renamed to xdelta114
if test -f "xdelta114"; then
	have_xdelta=1
	xdelta_bin="xdelta114"
elif test -f "xdelta113"; then
	have_xdelta=1
	xdelta_bin="xdelta113"
else
	have_xdelta=0
fi


# start
i=0

while true
do
	echo "data1/pak${i}.pak :"
	# make sure the pak file exists
	test -f "data1/pak${i}.pak" || { echo "    Error: File not found."; exit 1; }

	if test $i -eq 0; then
		oldsum=$pak0old
		newsum=$pak0new
	else
		oldsum=$pak1old
		newsum=$pak1new
	fi
	echo "    Checking md5sum..."

	testsum=`$md5cmd data1/pak${i}.pak`
	echo "$testsum" | grep $newsum > /dev/null 2>&1
	if test $? -eq 0; then
		echo "    Already patched. Skipping"
		if test $i -eq 1; then
			break
		fi
		i=1
		continue
	fi
	# make sure the pak file is cdrom version
	echo "$testsum" | grep $oldsum > /dev/null 2>&1
	if test $? -ne 0; then
		echo "    Does not match cdrom version."
		exit 1
	fi

	# make sure the patch file exists
	test -f "patchdata/data1/data1pak${i}.xd" || { echo "    Error: patchdata/data1/data1pak${i}.xd not found."; exit 1; }
	# make sure our xdelta exists
	if test $have_xdelta -eq 0; then
		NoXdelta
	fi

	# rename the old pak file, patch process will produce the correct name
	chmod 644 data1/pak${i}.pak
	rm -f data1/pak${i}.pak.103
	mv data1/pak${i}.pak data1/pak${i}.pak.103
	# apply the patch
	chmod 755 $xdelta_bin || { "Error: unable to execute $xdelta_bin"; exit 1; }
	echo "    Patching ...."
	./$xdelta_bin patch patchdata/data1/data1pak${i}.xd data1/pak${i}.pak.103 data1/pak${i}.pak
	if test $? -ne 0; then
		echo "    Failed."
		rm -f data1/pak${i}.pak
		mv data1/pak${i}.pak.103 data1/pak${i}.pak
		exit 1
	fi
	# make sure the patched pak file is ok
	testsum=`$md5cmd data1/pak${i}.pak`
	echo "$testsum" | grep $newsum > /dev/null 2>&1
	if test $? -ne 0; then
		echo "    Failed."
		rm -f data1/pak${i}.pak
		mv data1/pak${i}.pak.103 data1/pak${i}.pak
		exit 1
	fi
	rm -f data1/pak${i}.pak.103
	echo "    Success."

	if test $i -eq 1; then
		break
	fi
	i=1
	continue
done

echo ""

