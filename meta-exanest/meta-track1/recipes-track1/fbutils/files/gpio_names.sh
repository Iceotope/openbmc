
## IO PIn names

## Some pinIO names
SITE_I2C_RST=905
DB_I2C_RST=904
SLOT_ID_0=900
SLOT_ID_1=901
SLOT_ID_2=902
SLOT_ID_3=903

declare -a SLOT_ID=( ${SLOT_ID_0} \
    ${SLOT_ID_1} \
    ${SLOT_ID_2} \
    ${SLOT_ID_3} )

EEPROM_WP=899
PS_ON=898

CTL_REG_INT_0=890
CTL_REG_INT_1=891
CTL_REG_INT_2=892
CTL_REG_INT_3=893
CTL_REG_INT_4=894
CTL_REG_INT_5=895
CTL_REG_INT_6=896
CTL_REG_INT_7=897

declare -a CTL_REG_INT=( ${CTL_REG_INT_0} \
     ${CTL_REG_INT_1} \
     ${CTL_REG_INT_2} \
     ${CTL_REG_INT_3} \
     ${CTL_REG_INT_4} \
     ${CTL_REG_INT_5} \
     ${CTL_REG_INT_6} \
     ${CTL_REG_INT_7} )

REG_2V5_ENABLE=889
RETIMER_RESET=888
SERIAL_LOOPBACK=887

#BASE Board gpio expander
RST_CTLREG_0=871
RST_CTLREG_1=872
RST_CTLREG_2=873
RST_CTLREG_3=874
RST_CTLREG_4=875
RST_CTLREG_5=876
RST_CTLREG_6=877
RST_CTLREG_7=878
RST_CTLREG_8=879
RST_CTLREG_9=880
RST_CTLREG_10=881
RST_CTLREG_11=882
RST_CTLREG_12=883
RST_CTLREG_13=884
RST_CTLREG_14=885
RST_CTLREG_15=886

declare -a RST_CTLREG=( ${RST_CTLREG_0} \
  ${RST_CTLREG_1} \
  ${RST_CTLREG_2} \
  ${RST_CTLREG_3} \
  ${RST_CTLREG_4} \
  ${RST_CTLREG_5} \
  ${RST_CTLREG_6} \
  ${RST_CTLREG_7} \
  ${RST_CTLREG_8} \
  ${RST_CTLREG_9} \
  ${RST_CTLREG_10} \
  ${RST_CTLREG_11} \
  ${RST_CTLREG_12} \
  ${RST_CTLREG_13} \
  ${RST_CTLREG_14} \
  ${RST_CTLREG_15} )

# Array for default directions on IO expander
declare -a SITE_LOWIO_DIR_DEFAULT=( "in" \
  "in" \
  "in" \
  "in" \
  "out" \
  "out" \
  "out" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" )

  declare -a SITE_HIGHIO_DIR_DEFAULT=( "in" \
  "in" \
  "in" \
  "in" \
  "out" \
  "out" \
  "out" \
  "out" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "out" \
  "out" \
  "out" )

declare -a SITE_LOWIO_VAL_DEFAULT=( "0" \
  "0" \
  "0" \
  "0" \
  "0" \
  "0" \
  "0" \
  "0" \
  "0" \
  "0" \
  "0" \
  "0" \
  "0" \
  "0" \
  "0" \
  "0" )

  declare -a SITE_HIGHIO_VAL_DEFAULT=( "0" \
  "0" \
  "0" \
  "0" \
  "1" \
  "1" \
  "1" \
  "0" \
  "0" \
  "0" \
  "0" \
  "0" \
  "0" \
  "1" \
  "0" \
  "1" )
