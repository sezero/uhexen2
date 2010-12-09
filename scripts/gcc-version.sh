#!/bin/sh
#
# gcc-version gcc-command
#
# Prints the gcc version of `gcc-command' in a canonical 4-digit form
# such as `0295' for gcc-2.95, `0303' for gcc-3.3, etc.
#
# adapted from the linux kernel tree

if test $# -lt 1; then
	echo "No parameters given!"
	exit 1
fi

$1 --version > /dev/null 2>&1
if test $? -ne 0; then
	echo "command $1 not found"
	exit 1
fi

compiler="$*"

MAJOR=`echo __GNUC__ | $compiler -E -xc - | tail -n 1`
MINOR=`echo __GNUC_MINOR__ | $compiler -E -xc - | tail -n 1`

printf "%02d%02d\\n" $MAJOR $MINOR

