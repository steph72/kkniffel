#!/bin/sh

set -e

if [ ! -f "disc/kkniffel.d81" ]; then
  mkdir -p disc
  c1541 -format kkniffel,s3 d81 disc/kkniffel.d81
fi

/bin/sh tools/buildResources.sh
cat cbm/wrapper.prg bin/kk.c64 > bin/kk.m65

c1541 <<EOF
attach disc/kkniffel.d81
delete autoboot.c65
delete kk.m65
write cbm/autoboot.c65
write bin/kk.m65
EOF

for filename in gamedata/*; do
  c1541 disc/kkniffel.d81 -delete $(basename $filename)
  c1541 disc/kkniffel.d81 -write $filename
done

