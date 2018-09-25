# Copyright 2019-present Iceotope. All Rights Reserved.

SUMMARY = "Retimers"
DESCRIPTION = "Scripts and other bits to control the Retimers"
SECTION = "base"
PR = "r1"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8632344561f6474310f177b308473b46"

SRC_URI = "file://retimers.sh \
           file://check_retimers.py \
           file://retimer_10_3125gbps.csv \
           file://retimer_10gbps.csv \
           file://LICENSE \
          "

pkgdir = "retimers"

S = "${WORKDIR}"

binfiles = "check_retimers.py"

do_install() {
  dst="${D}/usr/local/packages/${pkgdir}"
  install -d $dst
  install -m 644 retimers.sh ${dst}/retimers.sh
  install -m 644 check_retimers.py ${dst}/check_retimers.py
  install -m 644 retimer_10_3125gbps.csv ${dst}/retimer_10_3125gbps.csv
  install -m 644 retimer_10gbps.csv ${dst}/retimer_10gbps.csv

  localbindir="${D}/usr/local/bin"
  install -d ${localbindir}
  for f in ${binfiles}; do
      install -m 755 $f ${dst}/${f}
      ln -s ../packages/${pkgdir}/${f} ${localbindir}/${f}
  done
}

RDEPENDS_${PN} += "bash"
FILES_${PN} += "/usr/local "
