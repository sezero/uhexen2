#!/bin/sh

# the script I use for converting uppercase characters in
# file names to lowercase. can't remember its origin...

usage ()
{
    echo Usage: `basename $0` [-r ] file... >&2
    exit 2
}

if [ $# -lt 1 ]
then
    usage
fi

if [ "$1" = "-r" ]
then
    recursive=1
    shift
    if [ $# -lt 1 ]
    then
        usage
    fi
else
    recursive=0
fi

for i in "$@"
do
    new=`echo $i | tr "[:upper:]" "[:lower:]"`
    if [ "$new" != "$i" ]
    then
        echo $i "->" $new >&2
        mv "$i" "$new"
    fi
    if [ $recursive = 1 -a -d "$new" ]
    then
        $0 -r "$new"/*
    fi
done
