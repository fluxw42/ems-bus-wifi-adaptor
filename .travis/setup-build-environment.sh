#!/bin/bash

set -e

BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ ! -d "/home/travis/Espressif/esptool" ]; then

	# Download and build lx106 cross-tool
	mkdir -p ~/Espressif
	git clone -b lx106 git://github.com/jcmvbkbc/crosstool-NG.git ~/Espressif/crosstool-NG
	cd ~/Espressif/crosstool-NG

	# Some downloads moved, apply patch to update URL
	patch -p1 < "${BASE_DIR}/patches/0001-Fix-download-url-for-mpc-1.0.2.patch"

	./bootstrap && ./configure --prefix=`pwd` && make && make install
	./ct-ng xtensa-lx106-elf
	./ct-ng build
	PATH=$PWD/builds/xtensa-lx106-elf/bin:$PATH

	# Install ESP8266 SDK and apply patches
	cd ~/Espressif
	wget -O esp_iot_sdk_v0.9.3_14_11_21.zip https://github.com/esp8266/esp8266-wiki/raw/master/sdk/esp_iot_sdk_v0.9.3_14_11_21.zip
	wget -O esp_iot_sdk_v0.9.3_14_11_21_patch1.zip https://github.com/esp8266/esp8266-wiki/raw/master/sdk/esp_iot_sdk_v0.9.3_14_11_21_patch1.zip
	unzip -o esp_iot_sdk_v0.9.3_14_11_21.zip
	unzip -o esp_iot_sdk_v0.9.3_14_11_21_patch1.zip
	mv esp_iot_sdk_v0.9.3 ESP8266_SDK
	mv License ESP8266_SDK/

	# Get additional libraries and include files
	cd ~/Espressif/ESP8266_SDK
	sed -i -e 's/xt-ar/xtensa-lx106-elf-ar/' -e 's/xt-xcc/xtensa-lx106-elf-gcc/' -e 's/xt-objcopy/xtensa-lx106-elf-objcopy/' Makefile
	mv examples/IoT_Demo .
	wget -O lib/libc.a https://github.com/esp8266/esp8266-wiki/raw/master/libs/libc.a
	wget -O lib/libhal.a https://github.com/esp8266/esp8266-wiki/raw/master/libs/libhal.a
	wget -O include.tgz https://github.com/esp8266/esp8266-wiki/raw/master/include.tgz
	tar -xvzf include.tgz

	# Install ESP tool and dependencies
	git clone -b v1.3 https://github.com/espressif/esptool.git ~/Espressif/esptool

fi

pip install pyserial
