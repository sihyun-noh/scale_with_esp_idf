#!/bin/sh

PS3='Please Select one: '

select fruit in "STH_B" "STH_P" "SCO_B" "SCO_P"
do
  echo "The one you have selected is: $fruit"
  if [ "${fruit}" = "STH_B" ]; then
    echo "STH_B type mfg bin file flash."
    esptool.py -p /dev/tty.usbserial-A50285BI write_flash 0x4F4000 ./bin/sample-1.bin
  elif [ "${fruit}" = "STH_P" ]; then
    echo "STH_P type mfg bin file flash."
    esptool.py -p /dev/tty.usbserial-A50285BI write_flash 0x4F4000 ./bin/sample-2.bin

  elif [ "${fruit}" = "SCO_B" ]; then
    echo "SCO_P type mfg bin file flash."
    esptool.py -p /dev/tty.usbserial-A50285BI write_flash 0x4F4000 ./bin/sample-3.bin

  elif [ "${fruit}" = "SCO_P" ]; then
    echo "SCO_P type mfg bin file flash."
    esptool.py -p /dev/tty.usbserial-A50285BI write_flash 0x4F4000 ./bin/sample-4.bin
  else
    echo "dosen't selected"
  fi
  break  
done

exit 0


