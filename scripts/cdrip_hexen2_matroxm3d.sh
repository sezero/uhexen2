#!/bin/sh
# Simple script that rips Hexen II cdrom audio tracks to a
# format useable by Hammer of Thyrion (uHexen2). Needs cdparanoia and oggenc.

# This particular script is to be used with the Matrox m3D bundled version
# of Hexen II also known as "Continent of Blackmarsh":
# It has the first three audio tracks in reversed order by comparison to the
# retail disc from Activision (sigh...)

# create directory
mkdir -p data1/music
cd data1/music

# rip all tracks beginning with second one (the first track is data)
cdparanoia -B "2-"

# encode the trackXX.cdda.wav files with proper renaming
oggenc -q 6 -o casa1.ogg track04.cdda.wav
oggenc -q 6 -o casa2.ogg track03.cdda.wav
oggenc -q 6 -o casa3.ogg track02.cdda.wav
oggenc -q 6 -o casa4.ogg track05.cdda.wav

# remove .wav files
rm *.wav

echo "Ripping done. Move all data1/music/*.ogg files"
echo "to /your/path/to/hexen2/data1/music/ directory"
