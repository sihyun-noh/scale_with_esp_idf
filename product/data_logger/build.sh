#!/usr/bin/env bash

# 매개변수 확인.
# default 로 없을 경우엔 build 실행.
# build, clean, flash, monitor 가 아닌 다른 명령어 입력 시 그냥 종료.
checkArgVariable() {
  if [ "${*}" = "build" ]; then
    echo "start build project"
  elif [ "${*}" = "flash" ]; then
    echo "start build and flash"
  elif [ "${*}" = "clean" ]; then
    echo "start clean project"
  elif [ "${*}" = "fullclean" ]; then
    echo "start fullclean project"
  elif [ "${*}" = "monitor" ]; then
    echo "start monitor"
  elif [ "${*}" = "flash monitor" ]; then
    echo "start build, flash and monitor"
  elif [ "${*}" = "menuconfig" ]; then
    echo "start menuconfig"
  elif [[ "${*}" =~ "erase" ]]; then
    echo "start ${*}"
  else
    echo "Not support command. Please check your command"
    exit 0
  fi
}

if [ -z ${1} ] || [[ ${1} = "build" ]] || [[ ${1} = "flash" ]]; then
  echo "Please select product. 1: Temp & Humi, 2 : CO2 TH, 3 : Soil EC, 4 : Solar Radiation, 5 : Atlas pH, 6 : Atlas EC, 7 : Water PH, 8 : Wind Direction, 9 : Wind Speed, 10 : Water EC"
  read SELECT_NUM

  if [ ${SELECT_NUM} = "1" ]; then
    PRODUCT_NAME="SHT3X"
  elif [ ${SELECT_NUM} = "2" ]; then
    PRODUCT_NAME="SCD4X"
  elif [ ${SELECT_NUM} = "3" ]; then
    PRODUCT_NAME="RK520_02"
  elif [ ${SELECT_NUM} = "4" ]; then
    PRODUCT_NAME="SWSR7500"
  elif [ ${SELECT_NUM} = "5" ]; then
    PRODUCT_NAME="ATLAS_PH"
  elif [ ${SELECT_NUM} = "6" ]; then
    PRODUCT_NAME="ATLAS_EC"
  elif [ ${SELECT_NUM} = "7" ]; then
    PRODUCT_NAME="RK500_02"
  elif [ ${SELECT_NUM} = "8" ]; then
    PRODUCT_NAME="RK110_02"
  elif [ ${SELECT_NUM} = "9" ]; then
    PRODUCT_NAME="RK100_02"
  elif [ ${SELECT_NUM} = "10" ]; then
    PRODUCT_NAME="RK500_13"
  fi

  echo "select ${PRODUCT_NAME}"
  export CURRENT_PROJECT=${PRODUCT_NAME}

  DEFINED_PROD=$(grep -h '#define SENSOR_TYPE*' main/config.h)
  COMPILE_PROD="#define SENSOR_TYPE ${PRODUCT_NAME}"

  if [ -n "${DEFINED_PROD}" ]; then
    perl -pi -e "s/${DEFINED_PROD}/${COMPILE_PROD}/g" main/config.h
  fi
fi

if [ ${#} -eq 0 ]; then
  echo "start build project"
else
  checkArgVariable ${*}
fi

# 현재 빌드할 product 경로 확인
PROD_PATH=`pwd -P`
echo ${PROD_PATH}

# thirdparty/esp 경로
PARENT_PATH="${PROD_PATH%/*/*}"
TOOL_PATH="${PARENT_PATH}/"thirdparty/esp

#esp-idf version
ESP_IDF_VER="v4.4.1"

# get esp-idf
downloadSdk() {
  git -C ${TOOL_PATH} clone -b ${ESP_IDF_VER} --recursive https://github.com/espressif/esp-idf.git
}

# esp 폴더 유무 확인 후 없을 경우 download
if [ ! -d "${TOOL_PATH}" ]; then
  echo "SDK isn't exist. Download SDK"
  mkdir -p ${TOOL_PATH}
  downloadSdk
fi

# tool setup & env setting
cd ${TOOL_PATH}/esp-idf

./install.sh esp32

# exit 0
. export.sh

# build or menuconfig .etc...
cd ${PROD_PATH}

# unset 환경 변수
clearEnvVariable() {
  if [ ! -z ${CURRENT_PROJECT} ]; then
    unset CURRENT_PROJECT
  fi
}

checkDeviceConnect() {
  # 매개변수 없을 시에는 기본적으로 build 만 동작
  # Flash 일 경우 device 연결 확인 - 현재는 macOS, 추후 linux, window 확인 후 추가 예정
  PORT_PATH=$(ls /dev/cu.usb*)
  echo "port port : $PORT_PATH"

  if [ -z ${PORT_PATH} ]; then
    echo "Device is not connected. Not possible flash / monitoring."
    clearEnvVariable
    exit 0
  fi
}

prodBuild() {
    mkdir -p build
    cd build
    cmake .. -G Ninja
    ninja
}

if [ -z ${1} ]; then
  prodBuild
elif [ ${1} = "flash" ]; then
  prodBuild
  checkDeviceConnect
  ESPPORT=${PORT_PATH} ESPBAUD=460800 ninja flash
elif [[ "${*}" =~ "erase" ]]; then
  checkDeviceConnect
  idf.py -p ${PORT_PATH} erase-flash
elif [[ ! "${*}" =~ "monitor" ]]; then
  idf.py ${*}
fi

if  [[ "${*}" =~ "monitor" ]]; then
  checkDeviceConnect
  screen ${PORT_PATH} 115200
fi

clearEnvVariable