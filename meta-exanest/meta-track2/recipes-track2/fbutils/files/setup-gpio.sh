#!/bin/bash
#
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
#

### BEGIN INIT INFO
# Provides:          gpio-setup
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description:  Set up GPIO pins as appropriate
### END INIT INFO

# This file contains definitions for the GPIO pins that were not otherwise
# defined in other files.  We should probably move some more of the
# definitions to this file at some point.

# The commented-out sections are generally already defined elsewhere,
# and defining them twice generates errors.

. /usr/local/fbpackages/utils/ast-functions
. /usr/local/fbpackages/utils/gpio_names.sh

## Do all the exporting

# I2C
gpio_export_out ${SITE_I2C_RST}
gpio_export_out ${DB_I2C_RST}

gpio_set ${SITE_I2C_RST} 0
gpio_set ${DB_I2C_RST} 0

# Slot ID
gpio_export ${SLOT_ID_0}
gpio_export ${SLOT_ID_1}
gpio_export ${SLOT_ID_2}
gpio_export ${SLOT_ID_3}

# EEProm WP
gpio_export_out ${EEPROM_WP}
gpio_set ${EEPROM_WP} 0

# Power on (48V)
gpio_export_out ${PS_ON}
gpio_set ${PS_ON} 0


# IRQ's from the IO Expanders
gpio_export ${CTL_REG_INT_0}
gpio_export ${CTL_REG_INT_1}
gpio_export ${CTL_REG_INT_2}
gpio_export ${CTL_REG_INT_3}
gpio_export ${CTL_REG_INT_4}
gpio_export ${CTL_REG_INT_5}
gpio_export ${CTL_REG_INT_6}
gpio_export ${CTL_REG_INT_7}

# 2v5 enable, turns on the retimers
gpio_export_out ${REG_2V5_ENABLE}
gpio_set ${REG_2V5_ENABLE} 1

# Retimer resets
gpio_export_out ${RETIMER_RESET}
gpio_set ${RETIMER_RESET} 0

# Internal Serial loopback
gpio_export_out ${SERIAL_LOOPBACK}
gpio_set ${SERIAL_LOOPBACK} 0

# Yosemite OOM remediation
#   enable kernel panic (force reboot)
echo 1 >> /proc/sys/vm/panic_on_oom
