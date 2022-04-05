#!/bin/sh

qbsp -watervis -notjunc worldmix.map || exit 1
light -extra worldmix.bsp || exit 1
vis -level 4 worldmix.bsp || exit 1
