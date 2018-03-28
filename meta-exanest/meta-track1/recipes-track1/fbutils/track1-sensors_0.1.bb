# Copyright 2015-present Facebook. All Rights Reserved.
SUMMARY = "Track1 Sensor Utility"
DESCRIPTION = "Util for reading various sensors on Track1 Hardware"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://track1-sensors.c;beginline=4;endline=16;md5=b395943ba8a0717a83e62ca123a8d238"

SRC_URI = "file://track1-sensors \
          "

S = "${WORKDIR}/track1-sensors"

do_install() {
	install -d ${D}${bindir}
    install -m 0755 track1-sensors ${D}${bindir}/track1-sensors
}

DEPENDS += "libtrack1-sensor"

RDEPENDS_${PN} += "libtrack1-sensor"

FILES_${PN} = "${bindir}"
