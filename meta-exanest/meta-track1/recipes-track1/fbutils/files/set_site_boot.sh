#! /bin/bash

## Sets the site boot mode

# If no site specifed it should just read and print.
# If a site is specified it should just do that site.

# Some general helper functions
. /usr/local/fbpackages/utils/ast-functions
# Some gpio defs for names
. /usr/local/fbpackages/utils/gpio_names.sh


SITE_START=1

## Check for first argument, which is a site,
if [ ! -z "$1" ]; then
  ## check if its a number or text, and convert to slot number
  slot_id=$1
  if [ $slot_id -eq $slot_id 2> /dev/null ]; then
    echo Number >/dev/null
  else
    slot_id=$(slot_number ${slot_id,,})
  fi

  SITE_START=$slot_id
  MAX_SITE=$slot_id
fi

SITE_EEPROM_OFFSET=31

for i in `seq ${SITE_START} ${MAX_SITE}`
do
  site_boot=`/usr/bin/kv get slot${i}_boot_order persistent`
  # Remove leading zeros
  site_boot=$((10#$site_boot))

  echo -n "Site ${i} ($(slot_name $i)) :"

  case $site_boot in
   0)
   echo "JTAG"
   ;;
   6)
   echo "QSPI"
   ;;
   *)
   echo "Unknown!"
   ;;
  esac

done

## Now set any new site if needed
if [ ! -z "$2" ]; then
  NEW_TYPE=${2^^}
  case ${NEW_TYPE} in
   0|JTAG)
   echo "setting as JTAG"
   NEW_BOOT=0
   ;;
   6|QSPI)
   echo "setting as QSPI"
   NEW_BOOT=6
   ;;
   *)
   echo "Unknown type ${NEW_SITE}, ignoring"
   exit 10
   ;;
  esac

## Set the new value
  /usr/bin/kv set slot${1}_boot_order ${NEW_BOOT} persistent
fi
