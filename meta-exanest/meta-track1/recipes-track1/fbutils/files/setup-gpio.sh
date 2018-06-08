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

# Some general helper functions
. /usr/local/fbpackages/utils/ast-functions
# Some gpio defs for names
. /usr/local/fbpackages/utils/gpio_names.sh

# Code for the retimers.
. /usr/local/fbpackages/utils/retimers.sh


## Functions




############### MAIN BLOCK ########################
## Create the structure of the board with symlinks
rm -rf /tmp/mezzanine
mkdir -p /tmp/mezzanine
mkdir -p /tmp/mezzanine/gpio
mkdir -p /tmp/mezzanine/bmc

for i in `seq 1 ${MAX_SITE}`
do
    mkdir -p /tmp/mezzanine/site_${i}
    mkdir -p /tmp/mezzanine/site_${i}/gpio
    mkdir -p /tmp/mezzanine/db_${i}

done

## Do all the exporting

#BMC xADC block

ln -s /sys/devices/soc0/amba/f8007100.adc/iio:device0 /tmp/mezzanine/bmc/xadc

# I2C resets, export them as inputs

# Active low reset, so put them in normal mode when we turn them to outputs.
gpio_set ${SITE_I2C_RST} 1
gpio_set ${DB_I2C_RST} 1

ln -s /sys/class/gpio/gpio${SITE_I2C_RST} /tmp/mezzanine/gpio/SITE_I2C_RST
ln -s /sys/class/gpio/gpio${DB_I2C_RST} /tmp/mezzanine/gpio/DB_I2C_RST


# EEProm WP
gpio_set ${EEPROM_WP} 0
ln -s /sys/class/gpio/gpio${EEPROM_WP} /tmp/mezzanine/gpio/EEPROM_WP

# 2v5 enable, turns on the retimers
gpio_set ${REG_2V5_ENABLE} 0
ln -s /sys/class/gpio/gpio${REG_2V5_ENABLE} /tmp/mezzanine/gpio/REG_2V5_ENABLE

# Power on (48V)
gpio_set ${PS_ON} 0
ln -s /sys/class/gpio/gpio${PS_ON} /tmp/mezzanine/gpio/PS_ON

# Release reset on retimers, and turn power on
gpio_set ${RETIMER_RESET} 1
gpio_set ${REG_2V5_ENABLE} 1
ln -s /sys/class/gpio/gpio${RETIMER_RESET} /tmp/mezzanine/gpio/RETIMER_RESET

# Internal Serial loopback
gpio_set ${SERIAL_LOOPBACK} 0
ln -s /sys/class/gpio/gpio${SERIAL_LOOPBACK} /tmp/mezzanine/gpio/SERIAL_LOOPBACK

# Slot ID
index=0
SLOT_ID_VALUE=0
mkdir -p /tmp/mezzanine/gpio/SLOT_ID
for i in "${SLOT_ID[@]}"
do
  gpio_export ${i}
  ln -s /sys/class/gpio/gpio${i} /tmp/mezzanine/gpio/SLOT_ID/${index}
  # Read it
  bitvalue=`cat /tmp/mezzanine/gpio/SLOT_ID/${index}/value 2>/dev/null`
  bitvalue=$(($bitvalue<<$index))
  SLOT_ID_VALUE=$(($SLOT_ID_VALUE+$bitvalue))
  index=$((index+1))
done
echo "${SLOT_ID_VALUE}" > /tmp/mezzanine/SLOT_ID
printf 'Setting up Track1 Mezzanine in slot: %X\n' ${SLOT_ID_VALUE}


# Toggle reset
retimer_reset

# flash the retimers
retimer_program ${SLOT_ID_VALUE}


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

ln -s /sys/bus/i2c/drivers/at24/1-0050/eeprom /tmp/mezzanine/eeprom

# Get site config list from eeprom
# Offset takes into account we start counting slots at 1, so real offset
# is 32.
SITE_EEPROM_OFFSET=31

for i in `seq 1 ${MAX_SITE}`
do
  site_type=$(get_eeprom /tmp/mezzanine/eeprom $((${SITE_EEPROM_OFFSET}+$i)) )
  # Remove leading zeros
  site_type=$((10#$site_type))
  if [ "$site_type" -gt 3 ]; then
  ## None
    if [ "$site_type" -eq 255 ]; then
      ## Auto mode
      echo -n "255" > /tmp/mezzanine/db_${i}/type
    else
      ## None, out of range.
      echo -n "0" > /tmp/mezzanine/db_${i}/type
    fi
  else
    echo -n "$site_type" > /tmp/mezzanine/db_${i}/type
  fi
done

# PowerManagment chip on baseboard (track 1 at least)
## Needa a driver, need to port something over from another LTC chip
## Currently all code does direct i2c operation in C ti the i2c layer
## with ioctls/read/writes.
## One possibilty is to mod a driver, add a "supported chip" and submit
## to the kernel drivers?


## One wire bus here, we set links for the sensors at the sites to the directories
## No onewire on track 1 currently
#for i in `seq 0 ${MAX_SITE}`
#do
#    master="/sys/devices/w1_bus_master$((i+1))"
#    slave=`cat ${master}/w1_master_slaves`
#    ln -s ${master}/${slave}/w1_slave /tmp/mezzanine/site_${i}/temperature
#done

## Now we can search the i2c sub busses an bind all the ioexpanders, site and
## other nice things.

for i in `seq 1 ${MAX_SITE}`
do
  #echo "Site ${i}"
  if [ "$i" -gt 8 ]; then
  #i2c goes 0-7, so sub 9
    site_bus=`ls -l /sys/bus/i2c/devices/1-0077/channel-$((i-9))|cut -d- -f5`
    db_bus=`ls -l /sys/bus/i2c/devices/0-0077/channel-$((i-9))|cut -d- -f5`
  else
    #i2c goes 0-7, so sub 1
    site_bus=`ls -l /sys/bus/i2c/devices/1-0070/channel-$((i-1))|cut -d- -f5`
    db_bus=`ls -l /sys/bus/i2c/devices/0-0070/channel-$((i-1))|cut -d- -f5`
  fi

  ln -s /dev/i2c-${site_bus} /tmp/mezzanine/site_${i}/i2c_bus
  ln -s /dev/i2c-${db_bus} /tmp/mezzanine/db_${i}/i2c_bus

  # echo "Site ${i} bus = ${site_bus}"

  #### DB setup
  ## Check the type for auto-detect
  site_detect ${i} ${db_bus}
  ## Check site for type now
  SITE_TYPE=`cat /tmp/mezzanine/db_${i}/type 2>/dev/null`
  case ${SITE_TYPE} in
    0)
    ## NONE,
      echo "EMPTY site ${i}"
      rm -f /tmp/mezzanine/db_${1}/type

      # Assign site's IO expander defaults, based on TPDB
      # we use low/high on outputs to spec default values
      SITE_LOWIO_DIR_DEFAULT=("${SITE_LOWIO_DIR_DEFAULT_TPDB[@]}")
      SITE_HIGHIO_DIR_DEFAULT=("${SITE_HIGHIO_DIR_DEFAULT_TPDB[@]}")
    ;;
    1)
    ## QFDB,
      echo "QFDB board in site ${i}"
    ;;
    2)
    ## KDB,
      echo "KDB board in site ${i}"
      # Assign site's IO expander defaults
      SITE_LOWIO_DIR_DEFAULT=("${SITE_LOWIO_DIR_DEFAULT_KDB[@]}")
      SITE_HIGHIO_DIR_DEFAULT=("${SITE_HIGHIO_DIR_DEFAULT_KDB[@]}")
    ;;
    3)
    ## TPDB,
      echo "TPDB board in site ${i}"
      init_tpdb ${i} ${db_bus} ${site_bus}
      # Assign site's IO expander defaults
      SITE_LOWIO_DIR_DEFAULT=("${SITE_LOWIO_DIR_DEFAULT_TPDB[@]}")
      SITE_HIGHIO_DIR_DEFAULT=("${SITE_HIGHIO_DIR_DEFAULT_TPDB[@]}")

    ;;
    *)
    ## OTHERS
      echo "Unknown board in site ${i}"
      # Assign site's IO expander defaults, based on TPDB
      SITE_LOWIO_DIR_DEFAULT=("${SITE_LOWIO_DIR_DEFAULT_TPDB[@]}")
      SITE_HIGHIO_DIR_DEFAULT=("${SITE_HIGHIO_DIR_DEFAULT_TPDB[@]}")

    ;;
  esac

  #### Back to the mezzanine setup.
  # push the device name into the bind directory for the IO Expanders

  echo -n "${site_bus}-0074" > /sys/bus/i2c/drivers/pca953x/bind 2>/dev/null
  echo -n "${site_bus}-0075" > /sys/bus/i2c/drivers/pca953x/bind 2>/dev/null

  ## Low 0-15 are on -0074
  mkdir -p /tmp/mezzanine/site_${i}/gpio/IO
  lowbase=`cat /sys/bus/i2c/devices/${site_bus}-0074/gpio/*/base`
  #echo "Site ${i} lowbase = ${lowbase}"
  if [ "$lowbase" -gt 0 ]; then
    for j in `seq 0 15`
    do
      gpio_export $((lowbase+$j))
      ln -s /sys/class/gpio/gpio$((lowbase+j)) /tmp/mezzanine/site_${i}/gpio/IO/${j}

      # Set direction and value defaults
      echo "${SITE_LOWIO_DIR_DEFAULT[j]}" > /tmp/mezzanine/site_${i}/gpio/IO/${j}/direction

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

      # Set direction and value defaults
      echo "${SITE_HIGHIO_DIR_DEFAULT[j]}" > /tmp/mezzanine/site_${i}/gpio/IO/$((j+16))/direction

    done
  fi

## Values are encoded in the direction as low/high
### Set default values.. if not empty
#  for j in `seq 0 15`
#  do
#    if [ "${SITE_LOWIO_DIR_DEFAULT[j]}" == "out" ]; then
#      echo "${SITE_LOWIO_VAL_DEFAULT[j]}" > /tmp/mezzanine/site_${i}/gpio/IO/${j}/value
#    fi
#    if [ "${SITE_HIGHIO_DIR_DEFAULT[j]}" == "out" ]; then
#      echo "${SITE_HIGHIO_VAL_DEFAULT[j]}" > /tmp/mezzanine/site_${i}/gpio/IO/$((j+16))/value
#    fi
#
#  done

done


# Yosemite OOM remediation
#   enable kernel panic (force reboot)
echo 1 >> /proc/sys/vm/panic_on_oom
