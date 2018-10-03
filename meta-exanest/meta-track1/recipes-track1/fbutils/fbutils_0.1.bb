# Copyright 2014-present Facebook. All Rights Reserved.
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
SUMMARY = "Utilities"
DESCRIPTION = "Various utilities"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=eb723b61539feef013de476e68b5c50a"

SRC_URI = "file://ast-functions \
           file://sol-util \
           file://power_led.sh \
           file://power_util.py \
           file://setup-gpio.sh \
           file://gpio_names.sh \
           file://power-on.sh \
           file://fw_env_config.sh \
           file://set_site_type.sh \
           file://set_mac.sh \
           file://set_site_boot.sh \
           file://set_tpdb.sh \
           file://jtag_setup.sh \
           file://COPYING \
          "

pkgdir = "utils"

S = "${WORKDIR}"

binfiles = "sol-util power_led.sh power_util.py set_site_type.sh set_tpdb.sh jtag_setup.sh set_site_boot.sh set_mac.sh "

DEPENDS_append = "update-rc.d-native"

do_install() {
  dst="${D}/usr/local/fbpackages/${pkgdir}"
  install -d $dst
  install -m 644 ast-functions ${dst}/ast-functions
  install -m 644 gpio_names.sh ${dst}/gpio_names.sh
  install -m 644 set_site_type.sh ${dst}/set_site_type.sh
  install -m 644 set_site_boot.sh ${dst}/set_site_boot.sh
  install -m 644 set_tpdb.sh ${dst}/set_tpdb.sh
  install -m 644 set_mac.sh ${dst}/set_mac.sh
  install -m 644 jtag_setup.sh  ${dst}/jtag_setup.sh
  localbindir="${D}/usr/local/bin"
  install -d ${localbindir}
  for f in ${binfiles}; do
      install -m 755 $f ${dst}/${f}
      ln -s ../fbpackages/${pkgdir}/${f} ${localbindir}/${f}
  done

  # init
  install -d ${D}${sysconfdir}/init.d
  install -d ${D}${sysconfdir}/rcS.d
  install -m 0755 ${WORKDIR}/fw_env_config.sh ${D}${sysconfdir}/init.d/fw_env_config.sh
  update-rc.d -r ${D} fw_env_config.sh start 05 S .
  install -m 755 setup-gpio.sh ${D}${sysconfdir}/init.d/setup-gpio.sh
  update-rc.d -r ${D} setup-gpio.sh start 59 5 .
  #install -m 755 power-on.sh ${D}${sysconfdir}/init.d/power-on.sh
  #update-rc.d -r ${D} power-on.sh start 70 5 .
}

RDEPENDS_${PN} += "bash"
FILES_${PN} += "/usr/local ${sysconfdir}"
