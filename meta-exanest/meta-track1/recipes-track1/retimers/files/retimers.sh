#!/bin/bash

#
# Code to support the Retimers and their setup/config
#
. /usr/local/fbpackages/utils/ast-functions

declare -a RETIMER_I2C_ADDRESS=( ${CTL_REG_INT_0} \
     ${CTL_REG_INT_1} \
     ${CTL_REG_INT_2} \
     ${CTL_REG_INT_3} \
     ${CTL_REG_INT_4} \
     ${CTL_REG_INT_5} \
     ${CTL_REG_INT_6} \
     ${CTL_REG_INT_7} )

retimer_reset() {
  # Retimer reset, toggle bit.
  gpio_set ${RETIMER_RESET} 0
  usleep 100000
  gpio_set ${RETIMER_RESET} 1
}

do_retimer() {
  echo "Parsing register set file <$1>..."

  SAVEIFS=$IFS

  while read -r line
  do
    IFS=, read -r I2C_DEV I2C_ADDR REG_ADDR REG_VAL <<<"$line"
    if [[ $I2C_DEV =~ \#.* ]]; then
      #echo "Skip <$line>"
      continue
    fi
    ## Forth Typo!. It never writes the REG_V, as it should be REG_VAL
    ##echo "i2cset -y $I2C_DEV $I2C_ADDR $REG_ADDR $REG_V"
    i2cset -y $I2C_DEV $I2C_ADDR $REG_ADDR $REG_VAL

  done < "$1"

  IFS=$SAVEIFS
}

retimer_program() {
  echo "Flashing Retimers..."
  ## Use one of these configs.
  do_retimer /usr/local/packages/retimers/retimer_10_3125gbps.csv
  ##do_retimer /usr/local/packages/retimers/retimer_10gbps.csv
  echo "...Done"
}
