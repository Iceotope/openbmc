#   EuroAXA Mezzanine Firmware tool
#   Copyright (C) 2019-present  Iceotope Ltd.
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
SUMMARY = "Utilities"
DESCRIPTION = "EXA Firmware Utility"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv3"
LIC_FILES_CHKSUM = "file://COPYING;md5=5b4473596678d62d9d83096273422c8c"

SRC_URI = "file://exa-fw-util.py \
           file://COPYING \
          "

pkgdir = "euroexa"

S = "${WORKDIR}"

binfiles = "exa-fw-util.py"

do_install() {
  dst="${D}/usr/local/packages/${pkgdir}"
  install -d $dst
  install -m 644 exa-fw-util.py ${dst}/exa-fw-util.py

  localbindir="${D}/usr/local/bin"
  install -d ${localbindir}
  for f in ${binfiles}; do
      install -m 755 $f ${dst}/${f}
      ln -s ../packages/${pkgdir}/${f} ${localbindir}/${f}
  done
}

RDEPENDS_${PN} += "python3"
FILES_${PN} += "/usr/local "
