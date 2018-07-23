#!/bin/sh
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
#

source /usr/local/bin/openbmc-utils.sh

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

RJ45_SEL="${SCMCPLD_SYSFS_DIR}/rj45_mode_sel"

usage(){
    program=`basename "$0"`
    echo "Usage:"
    echo "     $program led/combo"
}

if [ $# -ne 1 ]; then
    usage
    exit -1
fi

devmem_set_bit $(scu_addr 88) 30
devmem_set_bit $(scu_addr 88) 31

if [ "$1" == "led" ]; then
    echo "Wait a few seconds to setup management port LED..."
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0x012
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0xa03
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0x019
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0x2418
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0x01d
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0x3435
    echo 1 > $RJ45_SEL
    echo "Done!"
elif [ "$1" == "combo" ]; then
    echo "Wait about 2.5 minutes to setup management combo port..."
    ast-mdio.py --mac 1 --phy 0x9 write 0x17 0x0f7e
    ast-mdio.py --mac 1 --phy 0x9 write 0x15 0x00
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0x083c
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0x00
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0x2f
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0x7167
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0x21
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0x7c05
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0x00
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0x1140
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0x21
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0x7c04
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0x09
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0x0200
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0x04
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0x01e1
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0x00
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0x1340
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0x234
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0xd18f
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0xb00
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0x1140
    ast-mdio.py --mac 1 --phy 0x9 write 0x1e 0x23e
    ast-mdio.py --mac 1 --phy 0x9 write 0x1f 0x7ae2
    echo "Done!"
else
    usage
    exit -1
fi
