# Copyright 2018-present Iceotope. All Rights Reserved.
SUMMARY = "Track1 Common Library"
DESCRIPTION = "library for common track1 information"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://track1_common.c;beginline=8;endline=20;md5=da35978751a9d71b73679307c4d296ec"


SRC_URI = "file://track1_common \
          "

DEPENDS += "libkv "

S = "${WORKDIR}/track1_common"

do_install() {
    install -d ${D}${libdir}
    install -m 0644 libtrack1_common.so ${D}${libdir}/libtrack1_common.so

    install -d ${D}${includedir}

    install -d ${D}${includedir}/facebook
    install -m 0644 track1_common.h ${D}${includedir}/facebook/track1_common.h
}

FILES_${PN} = "${libdir}/libtrack1_common.so"
FILES_${PN}-dev = "${includedir}/facebook/track1_common.h"

RDEPENDS_${PN} += " libkv "
