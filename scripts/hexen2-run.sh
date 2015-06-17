#!/bin/sh
# This script can be used for running different variants of
# hexen2 clients or servers from a system-wide installation.
# Put this script on your $PATH , like under /usr/local/bin
# and create symlinks to it.
# Change the value of hexen2dir to match your installation.

# where is hexen2 installed?
hexen2dir=/usr/games/hexen2

# how are we called?
prog=`basename "$0"|sed -e 's/\.sh//g'|sed -e 's/-run//g'`

case $prog in
  hexen2|glhexen2|h2ded|hwcl|glhwcl|hwsv) # good
    ;;
  *)  echo "Bad binary name '$prog'"
      echo "must be one of hexen2, glhexen2, h2ded, hwcl, glhwcl, or hwsv"
      exit 1
    ;;
esac

if ! test -d "$hexen2dir" ; then
  echo "$0: hexen2 installation directory '$hexen2dir' does not exist"
  exit 1
fi

echo "Running '$prog $*'"
cd "$hexen2dir"
exec ./$prog $*
