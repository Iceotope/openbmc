#! /bin/bash

## Needs to read EEPROM data, print existing site and change to new site.

# If no site specifed it should just read and print.
# If a site is specified it should just do that site.
# If a new type is specified then it should set that site to that type.

# Some general helper functions
. /usr/local/fbpackages/utils/ast-functions
# Some gpio defs for names
. /usr/local/fbpackages/utils/gpio_names.sh

EEPROM_FILE=/tmp/mezzanine/eeprom

if [ ! -e ${EEPROM_FILE} ]; then
  echo "No EEPROM Found!"
  exit 10
fi

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
  site_type=$(get_eeprom ${EEPROM_FILE} $((${SITE_EEPROM_OFFSET}+$i)) )
  # Remove leading zeros
  site_type=$((10#$site_type))

  echo -n "Site ${i} ($(slot_name $i)) :"

  case $site_type in
   1)
   echo "QFDB"
   ;;
   2)
   echo "KDB"
   ;;
   3)
   echo "TPDB"
   ;;
   0)
   echo "Empty"
   ;;
   255)
   echo "AutoDetect"
   ;;
   *)
   echo "Unknown!"
   ;;
  esac

done

## Now set any new site if needed
if [ ! -z "$2" ]; then
  NEW_SITE=$2
  case ${NEW_SITE} in
   1|QFDB)
   echo "setting as QFDB"
   NEW_TYPE=01
   ;;
   2|KDB)
   echo "setting as KDB"
   NEW_TYPE=02
   ;;
   3|TPDB)
   echo "setting as TPDB"
   NEW_TYPE=03
   ;;
   0|EMPTY)
   echo "setting as Empty"
   NEW_TYPE=00
   ;;
   255|AUTO|AUTODETECT)
   echo "setting AutoDetect"
   NEW_TYPE=FF
   ;;
   *)
   echo "Unknown type ${NEW_SITE}, ignoring"
   exit 10
   ;;
  esac

## Now nee to poke the epromm
printf "\\x${NEW_TYPE}" | dd of=${EEPROM_FILE} bs=1 seek=$((${SITE_EEPROM_OFFSET}+$i)) count=1 conv=notrunc
fi
