# Copyright 2015-present Facebook. All Rights Reserved.
# Copyright 2018-present Iceotope. All Rights Reserved.

SUMMARY = "Front Panel Control Utility"
DESCRIPTION = "Util to override the front panel control remotely"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://fpc-util.c;beginline=5;endline=17;md5=60a5269aba663b7e2cc510cd01b212ee"

SRC_URI = "file://Makefile \
           file://fpc-util.c \
           "

S = "${WORKDIR}"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 fpc-util ${D}${bindir}/fpc-util
}

DEPENDS += "libpal"
RDEPENDS_${PN} += "libpal"

FILES_${PN} = "${bindir}"
