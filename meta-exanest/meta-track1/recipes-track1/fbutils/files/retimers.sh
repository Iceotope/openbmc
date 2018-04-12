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
  gpio_set ${RETIMER_RESET} 1
}

retimer_program() {
  echo "Flashing Retimers..."
  echo "...Done"
}
