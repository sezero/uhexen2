#!/bin/sh
# Simple script that rips Hexen II Portal of Praevus cdrom audio tracks to a
# format useable by Hammer of Thyrion (uHexen2). Needs cdparanoia and oggenc.

# This particular script is to be used with the Hexen II: Portal of Praevus
# i.e. the mission pack cdrom.

# create directory
mkdir -p portals/music
cd portals/music

# rip all tracks beginning with second one (the first track is data)
cdparanoia -B "2-"

# encode the trackXX.cdda.wav files with proper renaming
oggenc -q 6 -o tulku7.ogg track02.cdda.wav
oggenc -q 6 -o tulku1.ogg track03.cdda.wav
oggenc -q 6 -o tulku4.ogg track04.cdda.wav
oggenc -q 6 -o tulku2.ogg track05.cdda.wav
oggenc -q 6 -o tulku9.ogg track06.cdda.wav
oggenc -q 6 -o tulku10.ogg track07.cdda.wav
oggenc -q 6 -o tulku6.ogg track08.cdda.wav
oggenc -q 6 -o tulku5.ogg track09.cdda.wav
oggenc -q 6 -o tulku8.ogg track10.cdda.wav
oggenc -q 6 -o tulku3.ogg track11.cdda.wav
# track12 isn't associated with any midi file
oggenc -q 6 -o track12.ogg track12.cdda.wav

# remove .wav files
rm *.wav

echo "Ripping done. Move all portals/music/*.ogg files"
echo "to /your/path/to/hexen2/portals/music/ directory"
