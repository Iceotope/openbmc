
## IO PIn names

## Some pinIO names
SITE_I2C_RST=905
DB_I2C_RST=904
SLOT_ID_0=903
SLOT_ID_1=902
SLOT_ID_2=901
SLOT_ID_3=900

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
TRST_TO_MEZZ=886
RST_BTN=885

#BASE Board gpio expander
## We should try to do these dynamically,
## By finding the base address and working back.
RST_CTLREG_0=869
RST_CTLREG_1=870
RST_CTLREG_2=871
RST_CTLREG_3=872
RST_CTLREG_4=873
RST_CTLREG_5=874
RST_CTLREG_6=875
RST_CTLREG_7=876
RST_CTLREG_8=877
RST_CTLREG_9=878
RST_CTLREG_10=879
RST_CTLREG_11=880
RST_CTLREG_12=881
RST_CTLREG_13=882
RST_CTLREG_14=883
RST_CTLREG_15=884

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

# Array for default directions on IO expander,
# This is for TPDB operation
declare -a SITE_LOWIO_DIR_DEFAULT_TPDB=( "in" \
  "in" \
  "in" \
  "in" \
  "low" \
  "low" \
  "low" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" )

declare -a SITE_HIGHIO_DIR_DEFAULT_TPDB=( "in" \
  "in" \
  "in" \
  "in" \
  "high" \
  "high" \
  "high" \
  "low" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "high" \
  "low" \
  "high" )

# This is for KDB operation
declare -a SITE_LOWIO_DIR_DEFAULT_KDB=( "in" \
  "in" \
  "in" \
  "in" \
  "low" \
  "low" \
  "high" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" )

declare -a SITE_HIGHIO_DIR_DEFAULT_KDB=( "in" \
  "in" \
  "in" \
  "in" \
  "low" \
  "low" \
  "low" \
  "high" \
  "low" \
  "in" \
  "in" \
  "in" \
  "in" \
  "high" \
  "low" \
  "high" )

# This is for QFDB operation
declare -a SITE_LOWIO_DIR_DEFAULT_QFDB=( "in" \
  "in" \
  "in" \
  "in" \
  "low" \
  "low" \
  "low" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" \
  "in" )

declare -a SITE_HIGHIO_DIR_DEFAULT_QFDB=( "in" \
  "in" \
  "in" \
  "in" \
  "low" \
  "low" \
  "low" \
  "high" \
  "low" \
  "in" \
  "in" \
  "in" \
  "in" \
  "high" \
  "low" \
  "high" )

declare -a SITE_TYPE_NAMES=( "NONE" \
  "QFDB" \
  "KDB" \
  "TPDB" )

SITE_TYPE_AUTO=255
SITE_TYPE_NONE=0
SITE_TYPE_QFDB=1
SITE_TYPE_KDB=2
SITE_TYPE_TPDB=3

## TPDB IO defs


declare -a DB_TPDB_IO_DIR_DEFAULT=( "high" \
  "low" \
  "low" \
  "high" \
  "high" \
  "high" \
  "high" \
  "high" )

declare -a JTAG_DIR_DEFAULT=( "high" \
  "low" \
  "low" \
  "low" \
  "low" \
  "high" \
  "high" \
  "high" \
  "low" \
  "high" \
  "high" \
  "high" \
  "high" \
  "low" \
  "in" \
  "in" )

# GigE phy reset bit
GIGE_RSTn=24
#
LED_PWRUP=31
LED_PWRDN=30
LED_PWRCHANGE=29

