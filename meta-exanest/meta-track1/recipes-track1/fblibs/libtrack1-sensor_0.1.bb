# Copyright 2018-present Iceotope. All Rights Reserved.
SUMMARY = "Track1 Sensor Library"
DESCRIPTION = "library for reading various sensors"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://track1_sensor.c;beginline=8;endline=20;md5=da35978751a9d71b73679307c4d296ec"


SRC_URI = "file://track1_sensor \
          "
DEPENDS =+ " libipmi libipmb libtrack1-common obmc-i2c obmc-pal "
RDEPENDS_${PN} += " libtrack1-common libkv "

S = "${WORKDIR}/track1_sensor"

do_install() {
  install -d ${D}${libdir}
    install -m 0644 libtrack1_sensor.so ${D}${libdir}/libtrack1_sensor.so

    install -d ${D}${includedir}/facebook
    install -m 0644 track1_sensor.h ${D}${includedir}/facebook/track1_sensor.h
}

FILES_${PN} = "${libdir}/libtrack1_sensor.so"
FILES_${PN}-dev = "${includedir}/facebook/track1_sensor.h"
