#! /bin/bash

## Sets the PWM values of all TPDBs to a value, if not specified it will print out existing values.

# If no site specifed it should just read and print.
# If a site is specified it should just do that site.
# If a new calue is specified then it should set that site to that type.

# Some general helper functions
. /usr/local/fbpackages/utils/ast-functions
# Some gpio defs for names
. /usr/local/fbpackages/utils/gpio_names.sh

## Set all PWM settings for a site
set_new_pwm() {
DEFAULT_FREQ=120
PWM_START=0
PWM_END=4
SITE=${1}

# DUTY cannot be lower than 5, or higher than 95
REQ_DUTY=${2}
if [ ${REQ_DUTY} -gt 95 ]; then
  REQ_DUTY=95
fi

if [ ${REQ_DUTY} -lt 5 ]; then
  REQ_DUTY=5
fi


for pwm_num in `seq ${PWM_START} ${PWM_END}`
do
  ENABLED=unknown
  DUTY=0

  ## Check enable, record for later!
  enable_val=`cat /tmp/mezzanine/db_${SITE}/pwmchip/pwm${pwm_num}/enable`
  ## stop the site while we change it.
  echo 0 > /tmp/mezzanine/db_${SITE}/pwmchip/pwm${pwm_num}/enable

  ## Period is in ns
  PERIOD=`awk "BEGIN {printf \"%d\", (${pwm_num}*5)+(1/${DEFAULT_FREQ} * 1000000000)}"`
  ## now do Duty cycle of it
  DUTY=`awk "BEGIN {printf \"%d\", ${PERIOD}/100*${REQ_DUTY} }"`
  ## If we where set to 0, then we're turning off!
  if [ ${2} -eq 0 ]; then
    echo ${PERIOD} > /tmp/mezzanine/db_${SITE}/pwmchip/pwm${pwm_num}/period
    echo 0 > /tmp/mezzanine/db_${SITE}/pwmchip/pwm${pwm_num}/duty_cycle
    echo 0 > /tmp/mezzanine/db_${SITE}/pwmchip/pwm${pwm_num}/enable
  else
    echo ${PERIOD} > /tmp/mezzanine/db_${SITE}/pwmchip/pwm${pwm_num}/period
    echo ${DUTY} > /tmp/mezzanine/db_${SITE}/pwmchip/pwm${pwm_num}/duty_cycle

    # Re-enable if needed
    echo ${enable_val} > /tmp/mezzanine/db_${SITE}/pwmchip/pwm${pwm_num}/enable
  fi

done
}

## Dump out all PWM settings for a site
dump_pwm() {
PWM_START=0
PWM_END=4
SITE=${1}
for pwm_num in `seq ${PWM_START} ${PWM_END}`
do
  ENABLED=unknown
  DUTY=0

  ## Check enable
  enable_val=`cat /tmp/mezzanine/db_${SITE}/pwmchip/pwm${pwm_num}/enable`
  if [ "${enable_val}" -eq 1 ]; then
    ENABLED=running
  else
    ENABLED=stopped
  fi

  # Get duty cycle
  duty_val=`cat /tmp/mezzanine/db_${SITE}/pwmchip/pwm${pwm_num}/duty_cycle`
  period_val=`cat /tmp/mezzanine/db_${SITE}/pwmchip/pwm${pwm_num}/period`

  ## Calculate duty as a percent of the period now.
  if [ ${period_val} -gt 0 ]; then
    DUTY=`awk "BEGIN {printf \"%3.0f\", ${duty_val}/${period_val} * 100}"`
  fi
  echo "  PWM ${i}: ${DUTY}% ${ENABLED}"
done
}

SITE_START=1

## Check for first argument, which is a site,
if [ ! -z "$1" ]; then
  # check it's a number, > 0
  if [ "$1" -ge 0 ]; then
    SITE_START=$1
    MAX_SITE=$1
  fi
fi

SITE_EEPROM_OFFSET=31

for i in `seq ${SITE_START} ${MAX_SITE}`
do
  ## Check the site type in the dummy file system.
  site_type=`cat /tmp/mezzanine/db_${i}/type`
  # Remove leading zeros
  site_type=$((10#$site_type))

  echo -n "Site ${i} : "

  case $site_type in
   1)
   echo "QFDB"
   ;;
   2)
   echo "KDB"
   ;;
   3)
   echo -n "TPDB "
   ## master enable for PWM, is in I/O 0
   ## New value?
   if [ ! -z "$2" ]; then
     ## Set new value
     set_new_pwm ${i} ${2}
     if [ ! -z "$3" ]; then
       # 3rd Arg is on or off for master
       if [ "$3" == "on" ]; then
         for pwm_num in `seq ${PWM_START} ${PWM_END}`
         do
           echo 1 > /tmp/mezzanine/db_${SITE}/pwmchip/pwm${pwm_num}/enable
         done
         echo low > /tmp/mezzanine/db_${i}/gpio/IO/0/direction
       else
         for pwm_num in `seq ${PWM_START} ${PWM_END}`
         do
           echo 0 > /tmp/mezzanine/db_${SITE}/pwmchip/pwm${pwm_num}/enable
         done

         echo high > /tmp/mezzanine/db_${i}/gpio/IO/0/direction
       fi
     fi
   fi
   ## Print out PWM values
   MASTER_EN=`cat /tmp/mezzanine/db_${i}/gpio/IO/0/value`
   if [ ${MASTER_EN} -eq 0 ]; then
     echo "(Master Enabled)"
   else
     echo "(Master Disabled)"
   fi

   dump_pwm ${i}

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

