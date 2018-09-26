# Copyright 2015-present Facebook. All Rights Reserved.
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

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://0001-group_change_server.patch \
            file://mTerm1/run \
            file://mTerm2/run \
            file://mTerm3/run \
            file://mTerm4/run \
            file://mTerm5/run \
            file://mTerm6/run \
            file://mTerm7/run \
            file://mTerm8/run \
            file://mTerm9/run \
            file://mTerm10/run \
            file://mTerm11/run \
            file://mTerm12/run \
            file://mTerm-service-setup.sh \
            file://serialoverlan.profile \
           "

S = "${WORKDIR}"

# launch 12 mTerm services, 2 for each server
MTERM_SERVICES = "mTerm1 \
                  mTerm2 \
                  mTerm3 \
                  mTerm4 \
                  mTerm5 \
                  mTerm6 \
                  mTerm7 \
                  mTerm8 \
                  mTerm9 \
                  mTerm10 \
                  mTerm11 \
                  mTerm12 \
                 "

inherit useradd
USERADD_PACKAGES = "${PN}"
GROUPADD_PARAM_${PN} = "-f -r serialoverlan"
USERADD_PARAM_${PN} = "-g tty -G serialoverlan --system --home-dir /home/serialoverlan -p '\$1\$X0a8UnKJ\$HOgG/h/sQDRq38lguZgQp0' qfdb_a; \
       -M -g tty -G serialoverlan --system --home-dir /home/serialoverlan -p '\$1\$X0a8UnKJ\$HOgG/h/sQDRq38lguZgQp0' qfdb_b; \
       -M -g tty -G serialoverlan --system --home-dir /home/serialoverlan -p '\$1\$X0a8UnKJ\$HOgG/h/sQDRq38lguZgQp0' qfdb_c; \
       -M -g tty -G serialoverlan --system --home-dir /home/serialoverlan -p '\$1\$X0a8UnKJ\$HOgG/h/sQDRq38lguZgQp0' qfdb_d \
       "

do_install_append() {
  install -d ${D}/home/serialoverlan
  install -m 775 serialoverlan.profile ${D}/home/serialoverlan/.profile
}
DEPENDS_${PN} += "fbutils"
FILES_${PN} += "/home/serialoverlan"
