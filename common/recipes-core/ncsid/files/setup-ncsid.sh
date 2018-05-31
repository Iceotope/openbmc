#!/bin/sh
#
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
#

### BEGIN INIT INFO
# Provides:          setup-ncsid
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description: Setup sensor monitoring
### END INIT INFO

echo -n "Setup ncsid for BMC "

runsv /etc/sv/ncsid > /dev/null 2>&1 &

# enables OEM AENs if Broadcom NIC
q=$(fw-util nic --version | grep -i broadcom)
if [ -z "$q" ]
then
      # non Broadcom
      echo "enable standard AENs" >> /dev/kmsg
      /usr/local/bin/ncsi-util 8 0 0 0 0 0 0 0 1
else
      # Broadcom
      echo "Enable Broadcom OEM AENs" >> /dev/kmsg
      /usr/local/bin/ncsi-util 8 0 0 0 0 0 3 0 1
fi

echo "done."
