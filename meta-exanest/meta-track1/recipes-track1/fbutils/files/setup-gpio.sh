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

MAX_SITE=7

## Create the structure of the board with symlinks
rm -rf /tmp/mezzanine
mkdir -p /tmp/mezzanine
mkdir -p /tmp/mezzanine/gpio
for i in `seq 0 ${MAX_SITE}`
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
# Set all outputs to 1, as they are active low resets.
index=0
mkdir -p /tmp/mezzanine/gpio/RST_CTLREG
for i in "${RST_CTLREG[@]}"
do
   gpio_export_out ${i}
   gpio_set ${i} 1
   ln -s /sys/class/gpio/gpio${i} /tmp/mezzanine/gpio/RST_CTLREG/${index}
   index=$((index+1))
done


# EEPROM on base board (track 1 at least currently)

# PowerManagment chip on baseboard (track 1 at least)



## One wire bus here, we set links for the sensors at the sites to the directories
## No onewire on track 1 currently
#for i in `seq 0 ${MAX_SITE}`
#do
#    master="/sys/devices/w1_bus_master$((i+1))"
#    slave=`cat ${master}/w1_master_slaves`
#    ln -s ${master}/${slave}/w1_slave /tmp/mezzanine/site_${i}/temperature
#done

## Now we can search the i2c sub busses an bind all the ioexpanders.

for i in `seq 0 ${MAX_SITE}`
do
  echo "Site ${i}"
  if [ "$i" -gt 7 ]; then
    site_bus=`ls -l /sys/bus/i2c/devices/1-0077/channel-$((i-8))|cut -d- -f5`
    db_bus=`ls -l /sys/bus/i2c/devices/0-0077/channel-$((i-8))|cut -d- -f5`
  else
    site_bus=`ls -l /sys/bus/i2c/devices/1-0070/channel-${i}|cut -d- -f5`
    db_bus=`ls -l /sys/bus/i2c/devices/0-0070/channel-${i}|cut -d- -f5`
  fi

  ln -s /dev/i2c-${site_bus} /tmp/mezzanine/site_${i}/i2c_bus
  ln -s /dev/i2c-${db_bus} /tmp/mezzanine/db_${i}/i2c_bus

  #echo "Site ${i} bus = ${site_bus}"
  # push the device name into the bind directory

  echo -n "${site_bus}-0074" > /sys/bus/i2c/drivers/pca953x/bind
  echo -n "${site_bus}-0075" > /sys/bus/i2c/drivers/pca953x/bind

  ## Low 0-15 are on -0074
  mkdir -p /tmp/mezzanine/site_${i}/gpio/IO
  lowbase=`cat /sys/bus/i2c/devices/${site_bus}-0074/gpio/*/base`
  #echo "Site ${i} lowbase = ${lowbase}"
  if [ "$lowbase" -gt 0 ]; then
    for j in `seq 0 15`
    do
      gpio_export $((lowbase+$j))
      ln -s /sys/class/gpio/gpio$((lowbase+j)) /tmp/mezzanine/site_${i}/gpio/IO/${j}
    done
  fi
  ## High 16-31 are on -0075
  highbase=`cat /sys/bus/i2c/devices/${site_bus}-0075/gpio/*/base`
  #echo "Site ${i} highbase = ${highbase}"
  if [ "$highbase" -gt 0 ]; then
    for j in `seq 0 15`
    do
      gpio_export $((highbase+$j))
      ln -s /sys/class/gpio/gpio$((highbase+j)) /tmp/mezzanine/site_${i}/gpio/IO/$((j+16))
    done
  fi

done

# Yosemite OOM remediation
#   enable kernel panic (force reboot)
echo 1 >> /proc/sys/vm/panic_on_oom
