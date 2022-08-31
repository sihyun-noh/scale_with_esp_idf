#!/bin/sh
echo "build, mfg"
python3 ~/esp/esp-idf/tools/mass_mfg/mfg_gen.py generate sample_config.csv sample_master.csv sample 0x3000
