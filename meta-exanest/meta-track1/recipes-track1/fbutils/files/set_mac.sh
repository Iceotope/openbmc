#!/bin/bash

##
## Works on the QSPI and EEPROM data to fix/set/read MAC address
##
## Uses the SLOT ID to work out sub address needed.
##
## AA:BB:CC:DD:EE:FF
##
##
## FF is based on slot ID,
## High nibble is the SLOT, low nibble is site. 0=BMC
##                                              1-E for other uses.

## No options, dump status currently
###Arguments:
# --set : Just sets the EEPROM MAC
# --check : Checks to see if eeprom and qspi match
# --fix : Checks to see if they match, fixes QSPI and reboots

# Some general helper functions
. /usr/local/fbpackages/utils/ast-functions
# Some gpio defs for names
. /usr/local/fbpackages/utils/gpio_names.sh


EEPROM_FILE=/tmp/mezzanine/eeprom

ARG_DEBUG=0

# set an initial value for the flag
ARG_SET=0
ARG_CHECK=0
ARG_FIX=0

ARG_ANY=0

ARG_LONGOPTS="help,set,check,fix,debug"

######
###### FUNCTIONS
######

# call this function to write to stderr
echo_stderr ()
{
    echo "$@" >&2
}


######
###### MAIN CODE
######
# read the options, if there are any!
ARG_CMD="getopt --longoptions ${ARG_LONGOPTS} -n 'jtag_setup.sh' -- $0 $@"

if [ ${ARG_DEBUG} -eq 1 ]; then
  echo_stderr "Arg procesing.. "
  echo_stderr "$ARG_CMD"
fi

TEMP=`${ARG_CMD}`
eval set -- "$TEMP"

# extract options and their arguments into variables.
while true ; do

  case "$1" in
      --fix)
        ARG_FIX=1 ; ARG_ANY=1; shift ;;

      --debug)
        ARG_DEBUG=1 ; shift ;;

      --check)
        ARG_CHECK=1 ; ARG_ANY=1; shift ;;

      --set)
        ARG_SET=1 ; ARG_ANY=1; shift ;;


      --help)
        echo_stderr "Track1 MAC Utility"
        echo_stderr
        echo_stderr " set_mac.sh <options> [MAC ADDRESS in AA:BB:CC:DD:EE:FF] "
        echo_stderr "  Options."
        echo_stderr "    --help      : Print this message"
        echo_stderr "    --set       : Set the MAC in EEPROM and SPI"
        echo_stderr "    --check     : See if the MAC is valid"
        echo_stderr "    --fix       : Fix the QSPI MAC to match EEPROM, or make fresh if both are invalid"

        echo_stderr

        exit 1 ;;

      --) shift ; break ;;

      *) echo_stderr "Bad Arguments!" ; exit 10 ;;
  esac
done

# do something with the variables -- in this case just dump them out
if [ ${ARG_DEBUG} -eq 1 ]; then
  echo_stderr "ARG_ANY       = $ARG_ANY"
  echo_stderr "ARG_FIX       = $ARG_FIX"
  echo_stderr "ARG_SET       = $ARG_SET"
  echo_stderr "ARG_CHECK     = $ARG_CHECK"

fi

## Extract the two MACs, and the SLOT ID
SITE_EEPROM_OFFSET=24

for i in `seq 0 5`
do
  tmp=$(get_eeprom ${EEPROM_FILE} $((${SITE_EEPROM_OFFSET}+$i)) )
  MAC_BYTE[${i}]=$((10#$tmp))
done

EEPROM_MAC=$(printf "%02x:%02x:%02x:%02x:%02x:%02x" ${MAC_BYTE[0]} ${MAC_BYTE[1]} ${MAC_BYTE[2]} ${MAC_BYTE[3]} ${MAC_BYTE[4]} ${MAC_BYTE[5]})

echo "EEPROM MAC: ${EEPROM_MAC}"

# QSPI mac
QSPI_MAC=`fw_printenv ethaddr| cut -f 2 -d =`
if [ -z ${QSPI_MAC} ]; then
  QSPI_MAC="ff:ff:ff:ff:ff:ff"
fi

# SLOT
SLOT_ID=`cat /tmp/mezzanine/SLOT_ID`

echo "QSPI MAC: ${QSPI_MAC}"
if [ $# -gt 0 ]; then
  NEW_MAC=${1}
  echo "NEW MAC (raw): ${NEW_MAC}"
  IFS=':' read -r -a MAC_ARRAY <<< "$NEW_MAC"
  if [ ! ${#MAC_ARRAY[@]} -eq 6 ]; then
    echo "MAC Address Length Error!"
    exit 10
  fi

  for i in `seq 0 5`
  do
    MAC_ARRAY[$i]="0x${MAC_ARRAY[$i]}"
    MAC_ARRAY[$i]=$(printf "0x%02x" ${MAC_ARRAY[$i]})
  done

  MAC_ARRAY[5]=$(printf "0x%01x0" $SLOT_ID)
  MAC_ARRAY[0]=$(( (${MAC_ARRAY[0]} | 0x02) &0xFE ))
#  MAC_ARRAY[0]=$(printf "0x%02x" ${MAC_ARRAY[$0]})

  for i in `seq 0 5`
  do
#    echo "${MAC_ARRAY[$i]}"
    MAC_ARRAY[$i]=$((${MAC_ARRAY[$i]}))
  done

else
  ## New MAC is eeprom with the masking for the low byte
  MAC_ARRAY[5]=$(printf "0x%01x0" $SLOT_ID)

  for i in `seq 0 4`
  do
    MAC_ARRAY[$i]=$((${MAC_BYTE[$i]}))
  done
fi

NEW_MAC=$(printf "%02x:%02x:%02x:%02x:%02x:%02x" ${MAC_ARRAYMAC_ARRAY[0]} ${MAC_ARRAY[1]} ${MAC_ARRAY[2]} ${MAC_ARRAY[3]} ${MAC_ARRAY[4]} ${MAC_ARRAY[5]})
echo "NEW MAC (with slotID): ${NEW_MAC}"


## Now do the requested operation

if [ ${ARG_SET} -eq 1 ]; then
  if [ -z "$NEW_MAC" ]; then
    echo "Bad New MAC"
    exit 10
  fi

  ## Set the EEPROM and QSPI to the new MAC
  # QSPI version
  fw_setenv ethaddr ${NEW_MAC}

  for i in `seq 0 5`
  do
    MAC_HEX[$i]=$(printf "%02x" ${MAC_ARRAY[$i]})
  done
  # send the new one as raw data
  printf "\\x${MAC_HEX[0]}\\x${MAC_HEX[1]}\\x${MAC_HEX[2]}\\x${MAC_HEX[3]}\\x${MAC_HEX[4]}\\x${MAC_HEX[5]}" | dd of=${EEPROM_FILE} bs=1 seek=$((${SITE_EEPROM_OFFSET})) count=6 conv=notrunc
#  printf "\\x${MAC_HEX[0]}\\x${MAC_HEX[1]}\\x${MAC_HEX[2]}\\x${MAC_HEX[3]}\\x${MAC_HEX[4]}\\x${MAC_HEX[5]}" | hexdump -C



elif [ ${ARG_CHECK} -eq 1 ]; then
  # CHECK if the QSPI matches the EEPROM
  # 0 return means good, all MATCH
  # 1 means no EEPROM
  # 2 means no QSPI
  # 3 means neither!
  # 4 means no match
  RETCODE=0

  if [ ${QSPI_MAC,,} == "ff:ff:ff:ff:ff:ff" ]; then
    echo_stderr "Bad QSPI MAC"
    echo_stderr "Please use exa-fw-util.py or set_mac.h to set a MAC address"
    RETCODE=$((${RETCODE} | 0x2))
  fi
  if [ ${EEPROM_MAC,,} == "ff:ff:ff:ff:ff:ff" ]; then
    echo_stderr "Bad EEPROM MAC"
    echo_stderr "Please use exa-fw-util.py or set_mac.h to set a MAC address"
    RETCODE=$((${RETCODE} | 0x1))
  fi

  if [ ${NEW_MAC} == ${QSPI_MAC} ]; then
    exit ${RETCODE}
  else
    RETCODE=$((${RETCODE} | 0x4))
  fi

  exit ${RETCODE}

elif [ $ARG_FIX -eq 1 ]; then
  # If EEPROM is good, then set the QSPI if different and reboot!
  ## fixing, if needed.
    RETCODE=0

  if [ ${QSPI_MAC,,} == "ff:ff:ff:ff:ff:ff" ]; then
    echo_stderr "Bad QSPI MAC"
    echo_stderr "Please use exa-fw-util.py or set_mac.h to set a MAC address"
    RETCODE=$((${RETCODE} | 0x2))
    exit ${RETCODE}
  fi
  if [ ${EEPROM_MAC,,} == "ff:ff:ff:ff:ff:ff" ]; then
    echo_stderr "Bad EEPROM MAC"
    echo_stderr "Please use exa-fw-util.py or set_mac.h to set a MAC address"
    RETCODE=$((${RETCODE} | 0x1))
    exit ${RETCODE}
  fi

  if [ $((${RETCODE} & 0x1)) -eq 1 ];then
    # BAD EEPROM, don't do anything, but exit with an error.
    exit ${RETCODE}
  fi

  if [ ${NEW_MAC} == ${QSPI_MAC} ]; then
    ## Both match, and are not 0xFF's
    exit 0
  else
    # Set QSPI to the same as the EEPROM
    fw_setenv ethaddr ${NEW_MAC}
    # Update EEPROM with any slot changes
    for i in `seq 0 5`
    do
      MAC_HEX[$i]=$(printf "%02x" ${MAC_ARRAY[$i]})
    done
    # send the new one as raw data
    printf "\\x${MAC_HEX[0]}\\x${MAC_HEX[1]}\\x${MAC_HEX[2]}\\x${MAC_HEX[3]}\\x${MAC_HEX[4]}\\x${MAC_HEX[5]}" | dd of=${EEPROM_FILE} bs=1 seek=$((${SITE_EEPROM_OFFSET})) count=6 conv=notrunc
sleep 1
    reboot
  fi

fi
