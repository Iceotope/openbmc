# Copyright 2015-present Facebook. All Rights Reserved.
# Copyright 2018-present Iceotope. All Rights Reserved.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA

SUMMARY = "Track1 Fruid Library"
DESCRIPTION = "library for reading all track1 fruids"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://track1_fruid.c;beginline=7;endline=19;md5=da35978751a9d71b73679307c4d296ec"


SRC_URI = "file://track1_fruid \
          "

DEPENDS += " libtrack1-common "

S = "${WORKDIR}/track1_fruid"

do_install() {
    install -d ${D}${libdir}
    install -m 0644 libtrack1_fruid.so ${D}${libdir}/libtrack1_fruid.so

    install -d ${D}${includedir}
    install -d ${D}${includedir}/facebook
    install -m 0644 track1_fruid.h ${D}${includedir}/facebook/track1_fruid.h
}

FILES_${PN} = "${libdir}/libtrack1_fruid.so"
FILES_${PN}-dev = "${includedir}/facebook/track1_fruid.h"
