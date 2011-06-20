#!/bin/sh
# Simple script that rips Hexen II cdrom audio tracks to a
# format useable by Hammer of Thyrion (uHexen2). Needs cdparanoia and oggenc.

# create directory
mkdir -p data1/music
cd data1/music

# rip all tracks beginning with second one (the first track is data)
cdparanoia -B "2-"

# encode the trackXX.cdda.wav files with proper renaming
oggenc -q 6 -o casa1.ogg track02.cdda.wav
oggenc -q 6 -o casa2.ogg track03.cdda.wav
oggenc -q 6 -o casa3.ogg track04.cdda.wav
oggenc -q 6 -o casa4.ogg track05.cdda.wav
oggenc -q 6 -o egyp1.ogg track06.cdda.wav
oggenc -q 6 -o egyp2.ogg track07.cdda.wav
oggenc -q 6 -o egyp3.ogg track08.cdda.wav
oggenc -q 6 -o meso1.ogg track09.cdda.wav
oggenc -q 6 -o meso2.ogg track10.cdda.wav
oggenc -q 6 -o meso3.ogg track11.cdda.wav
oggenc -q 6 -o roma1.ogg track12.cdda.wav
oggenc -q 6 -o roma2.ogg track13.cdda.wav
oggenc -q 6 -o roma3.ogg track14.cdda.wav
oggenc -q 6 -o casb1.ogg track15.cdda.wav
oggenc -q 6 -o casb2.ogg track16.cdda.wav
oggenc -q 6 -o casb3.ogg track17.cdda.wav

# remove .wav files
rm *.wav

echo -e "\n Ripping done, move data1/music/ directory to /your/path/to/hexen2/data1/music/"
