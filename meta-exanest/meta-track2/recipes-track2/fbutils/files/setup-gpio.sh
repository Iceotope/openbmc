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

## Create the structure of the board with symlinks
rm -rf /tmp/mezzanine
mkdir -p /tmp/mezzanine
mkdir -p /tmp/mezzanine/gpio
for i in `seq 0 15`
do
    mkdir -p /tmp/mezzanine/site_${i}
    mkdir -p /tmp/mezzanine/site_${i}/gpio
    mkdir -p /tmp/mezzanine/db_${i}
done

## Do all the exporting

# I2C
gpio_export_out ${SITE_I2C_RST}
gpio_export_out ${DB_I2C_RST}

ln -s /sys/class/gpio/gpio${SITE_I2C_RST} /tmp/mezzanine/gpio/SITE_I2C_RST
ln -s /sys/class/gpio/gpio${DB_I2C_RST} /tmp/mezzanine/gpio/DB_I2C_RST

# Active low reset, so put them in normal mode
gpio_set ${SITE_I2C_RST} 1
gpio_set ${DB_I2C_RST} 1

# EEProm WP
gpio_export_out ${EEPROM_WP}
ln -s /sys/class/gpio/gpio${EEPROM_WP} /tmp/mezzanine/gpio/EEPROM_WP
gpio_set ${EEPROM_WP} 0

# Power on (48V)
gpio_export_out ${PS_ON}
ln -s /sys/class/gpio/gpio${PS_ON} /tmp/mezzanine/gpio/PS_ON
gpio_set ${PS_ON} 0

# 2v5 enable, turns on the retimers
gpio_export_out ${REG_2V5_ENABLE}
ln -s /sys/class/gpio/gpio${REG_2V5_ENABLE} /tmp/mezzanine/gpio/REG_2V5_ENABLE
gpio_set ${REG_2V5_ENABLE} 1

# Retimer resets
gpio_export_out ${RETIMER_RESET}
ln -s /sys/class/gpio/gpio${RETIMER_RESET} /tmp/mezzanine/gpio/RETIMER_RESET
gpio_set ${RETIMER_RESET} 0

# Internal Serial loopback
gpio_export_out ${SERIAL_LOOPBACK}
ln -s /sys/class/gpio/gpio${SERIAL_LOOPBACK} /tmp/mezzanine/gpio/SERIAL_LOOPBACK
gpio_set ${SERIAL_LOOPBACK} 0


# Slot ID
index=0
mkdir -p /tmp/mezzanine/gpio/SLOT_ID
for i in "${SLOT_ID[@]}"
do
   gpio_export ${i}
   ln -s /sys/class/gpio/gpio${i} /tmp/mezzanine/gpio/SLOT_ID/${index}
   index=$((index+1))
done

# IRQ's from the IO Expanders
index=0
mkdir -p /tmp/mezzanine/gpio/CTL_REG_INT
for i in "${CTL_REG_INT[@]}"
do
   gpio_export ${i}
   ln -s /sys/class/gpio/gpio${i} /tmp/mezzanine/gpio/CTL_REG_INT/${index}
   index=$((index+1))
done

# I2C Expander on base board
# IRQ's from the IO Expanders
index=0
mkdir -p /tmp/mezzanine/gpio/RST_CTLREG
for i in "${RST_CTLREG[@]}"
do
   gpio_export_out ${i}
   gpio_set ${i} 1
   ln -s /sys/class/gpio/gpio${i} /tmp/mezzanine/gpio/RST_CTLREG/${index}
   index=$((index+1))
done

## One wire bus here, we set links for the sensors at the sites to the directories

for i in `seq 0 15`
do
    master="/sys/devices/w1_bus_master$((i+1))"
    slave=`cat ${master}/w1_master_slaves`
    ln -s ${master}/${slave}/w1_slave /tmp/mezzanine/site_${i}/temperature
done

## Now we can search the i2c sub busses an bind all the ioexpanders.

# Yosemite OOM remediation
#   enable kernel panic (force reboot)
echo 1 >> /proc/sys/vm/panic_on_oom
