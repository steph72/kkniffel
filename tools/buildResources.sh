#!/bin/sh

set -e
rm -rf gamedata/*

# convert ui images with palette
for filename in images-src/ui/*.png; do
  python3 tools/png2dbm.py -v $filename gamedata/$(basename $filename .png).dbm
done

# convert aux images and shift palette
#for filename in images-src/artwork/*.png; do
#  python3 tools/png2dbm.py -vr $filename gamedata/$(basename $filename .png).dbm
#done