#!/usr/bin/env bash

########### tool setup & env setting ##########
echo "Generate MFG data for irrigation"
PROD_PATH=`pwd -P`
echo ${PROD_PATH}

PARENT_PATH="${PROD_PATH%/*/*/*}"
TOOL_PATH="${PARENT_PATH}/"thirdparty/esp

#esp-idf version
ESP_IDF_VER="v5.0"

downloadSdk() {
  git -C ${TOOL_PATH} clone -b ${ESP_IDF_VER} --recursive https://github.com/espressif/esp-idf.git
}

if [ ! -d "${TOOL_PATH}" ]; then
  echo "SDK isn't exist. Download SDK"
  mkdir -p ${TOOL_PATH}
  downloadSdk
fi

cd ${TOOL_PATH}/esp-idf

./install.sh all

. export.sh
########### tool setup & env setting ##########

selectDevice() {
  echo "Please select target chipset. 1: Master, 2: Child, 3: HID"
  read DEVICE_ID
  if [ ${DEVICE_ID} = "1" ]; then
    export IDF_TARGET=esp32
  elif [ ${DEVICE_ID} = "2" ]; then
    export IDF_TARGET=esp32
  elif [ ${DEVICE_ID} = "3" ]; then
    export IDF_TARGET=esp32s3
  else
    exit 0
  fi
}

generateBin() {
  python3 ${TOOL_PATH}/esp-idf/tools/mass_mfg/mfg_gen.py generate irrigation_config.csv irrigation_master.csv irrigation 0x6000
}

checkDeviceConnect() {
  # 매개변수 없을 시에는 기본적으로 build 만 동작
  # Flash 일 경우 device 연결 확인 - 현재는 macOS, 추후 linux, window 확인 후 추가 예정
  which fzf &> /dev/null
  if [ $? == 0 ] ; then
    PORT_PATH=$(ls /dev/cu.usb* | fzf)
  else
    PORT_PATH=$(ls /dev/cu.usb*)
  fi
  echo "port port : $PORT_PATH"

  if [ -z ${PORT_PATH} ]; then
    echo "Device is not connected. Not possible flash."
    cleanTmp
    exit 0
  fi
}

flashMFG() {
  esptool.py --chip ${IDF_TARGET} --port ${PORT_PATH} write_flash 0x244000 ./bin/irrigation-1.bin
}

cleanData() {
  rm -rf bin
  rm -rf csv
  rm -rf *tmp*.csv
}

cleanTmp() {
  rm -rf *tmp*.csv
}

cd ${PROD_PATH}

cleanData
selectDevice
generateBin
checkDeviceConnect
flashMFG
cleanTmp
echo "Generate and Flash MFG data completed"
