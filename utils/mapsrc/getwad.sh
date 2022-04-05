#!/bin/sh

#wget http://www.quaddicted.com/files/wads/hex2full.zip
curl -o hex2full.zip -L http://www.quaddicted.com/files/wads/hex2full.zip
unzip -q hex2full.zip
mv HEX2FULL.WAD hex2full.wad
