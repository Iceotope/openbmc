#!/bin/bash
#
# Copyright 2018-present Facebook. All Rights Reserved.
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

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

if [ $# -ne 1 ]; then
    exit -1
fi

img="$1"

source /usr/local/bin/openbmc-utils.sh

trap 'gpio_set BMC_SCM_CPLD_EN 1' INT TERM QUIT EXIT

# change BMC_SCM_CPLD_EN to 0 to connect BMC to SCM CPLD pins
gpio_set BMC_SCM_CPLD_EN 0

ispvm dll /usr/lib/libcpldupdate_dll_gpio.so "${img}" --tms 210 --tdo 213 --tdi 212 --tck 211
