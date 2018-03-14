# # Copyright 2017-present Iceotope Ltd. All Rights Reserved.
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

## Add local drevice tree path
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

## Add in all the xilinx device tree generated files.

MACHINE_DEVICETREE_prepend_zynq = " \
      track2/zynq-7000.dtsi \
      track2/pl.dtsi \
      track2/pcw.dtsi \
      track2/ioexpander.dtsi \
      track2/track2-zynq7.dts \
    "
