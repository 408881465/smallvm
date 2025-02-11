#!/bin/bash
# Rebuild and update the precompiled binaries.
rm -f *.hex *.bin *.uf2
cd ..

pio run -e itsybitsy
python precompiled/uf2conv.py -c .pio/build/itsybitsy/firmware.bin -o extraVMs/vm_itsybitsy.uf2
pio run -e metroM0
python precompiled/uf2conv.py -c .pio/build/metroM0/firmware.bin -o extraVMs/vm_metroM0.uf2
pio run -e mbits
cp .pio/build/mbits/firmware.bin extraVMs/vm_mbits.bin
pio run -e pico-xrp
cp .pio/build/pico-xrp/firmware.uf2 extraVMs/vm_pico_xrp.uf2
