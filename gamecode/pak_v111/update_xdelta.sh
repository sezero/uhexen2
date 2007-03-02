#!/bin/sh
# Script for patching cdrom versions of Hexen II pak files to v1.11
# For use with Hammer of Thyrion v1.4.0 and probably laer versions.

echo -e "\nHammer of Thyrion (http://uhexen2.sourceforge.net)"
echo -e "Patch script for Raven's Hexen II v1.11 update\n"

# here are the correct md5sums:
# cdrom-1.03 version:
# b53c9391d16134cb3baddc1085f18683  data1/pak0.pak
# 9a2010aafb9c0fe71c37d01292030270  data1/pak1.pak
# updated 1.11 version:
# c9675191e75dd25a3b9ed81ee7e05eff  data1/pak0.pak
# c2ac5b0640773eed9ebe1cda2eca2ad0  data1/pak1.pak

# make sure the data1 directory exists
if [ ! -d "data1" ]; then
	echo -e "data1 directory not found. Quitting.\n"
	exit 1
fi
# make sure we have write permissions
chmod 755 data1

# make sure we have the md5sum utility program
md5sum $0 &> /dev/null
status=$?
if [ "$status" -ne 0 ]; then
	echo -e "md5sum utility not installed. quitting"
	exit 1
fi

# make sure our xdelta-1.1.4 exists: people may have
# newer versions of xdelta which shall not work with
# our delta files. We use our patched 1.1.4 version,
# configured with --disable-shared, and the binary
# renamed to xdelta114
if [ -f "xdelta114" ]; then
	have_xdelta=1
	xdelta_bin="xdelta114"
elif [ -f "xdelta113" ]; then
	have_xdelta=1
	xdelta_bin="xdelta113"
else
	have_xdelta=0
fi

# we will patch data1/pak0.pak and data1/pak1.pak
# i is our counter. starting with 0.
i=0

while [ $i -lt 2 ]
do
	echo "data1/pak${i}.pak :"
	# make sure the pak file exists
	if [ ! -f "data1/pak${i}.pak" ]; then
		echo -e "\tFile not found. Quitting.\n"
		exit 1
	fi
	echo -e "\tChecking md5sum..."

	# make sure the pak file is not patched already
	case $i in
	0)
		sum="c9675191e75dd25a3b9ed81ee7e05eff"
		;;
	1)
		sum="c2ac5b0640773eed9ebe1cda2eca2ad0"
		;;
	*)
		echo -e "Error in script!\n"
		exit 1
		;;
	esac
	echo "${sum}  data1/pak${i}.pak" > newpak${i}.md5
	md5sum --status --check newpak${i}.md5
	status=$?
	if [ "$status" -eq 0 ]; then
		echo -e "\tAlready patched. Skipping"
		rm -f newpak${i}.md5
		let i=1+i
		continue
	fi

	# make sure the pak file is cdrom version
	case $i in
	0)
		sum="b53c9391d16134cb3baddc1085f18683"
		;;
	1)
		sum="9a2010aafb9c0fe71c37d01292030270"
		;;
	*)
		echo -e "Error in script!\n"
		exit 1
		;;
	esac
	echo "${sum}  data1/pak${i}.pak" > oldpak${i}.md5
	md5sum --status --check oldpak${i}.md5
	status=$?
	rm -f oldpak${i}.md5
	if [ "$status" -ne 0 ]; then
		echo -e "\tDoes not match cdrom version. Quitting\n"
		exit 1
	fi

	# make sure the patch file exists
	if [ ! -f "patchdata/data1/data1pak${i}.xd" ]; then
		echo -e "\tpatchdata/data1/data1pak${i}.xd not found. Quitting.\n"
		rm -f newpak${i}.md5
		exit 1
	fi

	# make sure our xdelta exists
	if [ ${have_xdelta} -eq 0 ]; then
		rm -f newpak${i}.md5
		echo "Patch binary xdelta113 or xdelta114 not found."
		echo "You can compile it from its up-to-date source"
		echo "tarball downloadable from the Hammer of Thyrion"
		echo -e "website.\n"
		exit 1
	fi

	# rename the old pak file, patch process will produce the correct name
	chmod 644 data1/pak${i}.pak
	mv --force data1/pak${i}.pak data1/pak${i}.pak.103
	# apply the patch
	chmod 755 ${xdelta_bin}
	echo -n -e "\tPatching : "
	./${xdelta_bin} patch patchdata/data1/data1pak${i}.xd data1/pak${i}.pak.103 data1/pak${i}.pak
	status=$?
	if [ "$status" -ne 0 ]; then
		echo -e "Failed. Quitting.\n"
		rm -f newpak${i}.md5
		mv --force data1/pak${i}.pak.103 data1/pak${i}.pak
		exit 1
	fi
	# make sure the patched pak file is ok
	md5sum --status --check newpak${i}.md5
	status=$?
	rm -f newpak${i}.md5
	if [ "$status" -ne 0 ]; then
		echo -e "Failed. Quitting.\n"
		mv --force data1/pak${i}.pak.103 data1/pak${i}.pak
		exit 1
	fi
	rm -f data1/pak${i}.pak.103
	echo "Success."
	let i=1+i
done

echo ""

exit 0

