#!/bin/sh

NoLokiPatch()
{
	echo "Error:  The patch binary loki_patch not found."
	echo "You can compile it from its up-to-date source"
	echo "tarball loki_patch-src4.tgz, downloadable from"
	echo "the Hammer of Thyrion website."
	echo ""
	exit 1
}

echo "=================================================="
echo "Hammer of Thyrion (http://uhexen2.sourceforge.net)"
echo "Patch script for Raven's Hexen II v1.11 PAK update"
echo ""

test -f "loki_patch" || { NoLokiPatch; }

PATCH_DAT="update_loki.dat"

test -f $PATCH_DAT || { echo "Error: Patch data file $PATCH_DAT not found."; exit 1; }

./loki_patch $PATCH_DAT .

status=$?
echo "                "
exit $status

