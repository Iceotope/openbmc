#!/bin/bash

## No options, dump status currently

###Arguments:
# --trans/--notrans
# --addmask
# --ethernet/--connector
# --reset
# --oe/--nooe
# --bridge/--stitcher

ARG_DEBUG=0

# set an initial value for the flag
ARG_TRANS=0
ARG_NOTRANS=0

ARG_ADDMASK=0
ARG_NOADDMASK=0

ARG_ETHERNET=0
ARG_CONNECTOR=0

ARG_RESET=0

ARG_OE=0
ARG_NOOE=0

ARG_BRIDGE=0
ARG_STITCHER=0

ARG_ANY=0

ARG_LONGOPTS="help,ioreset,debug,trans,notrans,addmask,noaddmask,ethernet,connector,reset,oe,nooe,bridge,stitcher"
GPIO_BASE="/tmp/mezzanine/jtag/gpio"

## IO Pins as defined by the IOExpander at /tmp/mezzanine/jtag/gpio
declare -A IO_PINS=(  [TRANS]=0 \
    [MASK]=1 \
    [TLR_TRST_6]=2 \
    [TLR_TRST]=3 \
    [SB_Bn]=4 \
    [MPSEL]=5 \
    [RSTn]=6 \
    [OEn]=7 \
    [LP_SEL_0]=8 \
    [LP_SEL_1]=9 \
    [LP_SEL_2]=10 \
    [LP_SEL_3]=11 \
    [LP_SEL_4]=12 \
    [LED]=13 )

## Site name to number
declare -a SITE_NAMES=( "EXT" \
  "QFDB_B" \
  "QFDB_A" \
  "QFDB_C" \
  "QFDB_D" )

declare -A SITE_NAMES_REVERSE=( [EXT]=0 \
  [QFDB_B]=1 \
  [QFDB_A]=2 \
  [QFDB_C]=3 \
  [QFDB_D]=4 \
  [QFDB-B]=1 \
  [QFDB-A]=2 \
  [QFDB-C]=3 \
  [QFDB-D]=4 )


######
###### FUNCTIONS
######

# call this function to write to stderr
echo_stderr ()
{
    echo "$@" >&2
}

# Arg1 = pin number
get_io_pin() {
  if [ ${ARG_DEBUG} -eq 1 ]; then
    echo_stderr "Getting pin $1"
  fi
  if [ -d ${GPIO_BASE} ]; then
    cat "${GPIO_BASE}/${1}/value"
  else
    case "$1" in
      ${IO_PINS[LP_SEL_0]})
        echo 0;;
      ${IO_PINS[LP_SEL_1]})
        echo 1;;
      ${IO_PINS[LP_SEL_2]})
        echo 0;;
      ${IO_PINS[LP_SEL_3]})
        echo 1;;
      ${IO_PINS[LP_SEL_4]})
        echo 0;;
      *) echo 0 ;;
  esac
  fi

}

# Arg1 = pin number, Arg2 = value
set_io_pin() {

  if [ $2 -eq -1 ]; then
    VALUE=in
  elif [ $2 -eq 0 ]; then
    VALUE=low
  else
    VALUE=high
  fi

  if [ ${ARG_DEBUG} -eq 1 ]; then
    echo_stderr "Setting pin $1 to $VALUE"
  fi

  ## Actual setting needs the GPIO to exist!
  if [ -d ${GPIO_BASE} ]; then
    echo ${VALUE} > "${GPIO_BASE}/${1}/direction"
  fi
}

do_status_dump() {
  echo_stderr
  echo_stderr "Current JTAG Extender status.."
  ## Get the mask first
  MASK=$(get_chain_mask)
  if [ $((${MASK} & 1)) -eq 1 ]; then
    echo_stderr "  ${SITE_NAMES[0]} in chain"
  fi
  if [ $((${MASK} & 4)) -eq 4 ]; then
    echo_stderr "  ${SITE_NAMES[2]} in chain"
  fi
  if [ $((${MASK} & 2)) -eq 2 ]; then
    echo_stderr "  ${SITE_NAMES[1]} in chain"
  fi
  if [ $((${MASK} & 8)) -eq 8 ]; then
    echo_stderr "  ${SITE_NAMES[3]} in chain"
  fi
  if [ $((${MASK} & 16)) -eq 16 ]; then
    echo_stderr "  ${SITE_NAMES[4]} in chain"
  fi
  ## Get the staus of the IO pins

  if [ $(get_io_pin ${IO_PINS[TRANS]}) -eq 1 ]; then
    echo_stderr "  Transparent Mode"
  else
    echo_stderr "  Non-Transparent Mode"
  fi

  if [ $(get_io_pin ${IO_PINS[MASK]}) -eq 1 ]; then
    echo_stderr "  Masked Mode"
  else
    echo_stderr "  Non-Masked Mode"
  fi

  if [ $(get_io_pin ${IO_PINS[SB_Bn]}) -eq 1 ]; then
    echo_stderr "  ScanBridge Mode"
  else
    echo_stderr "  Stitcher Mode"
  fi

  if [ $(get_io_pin ${IO_PINS[MPSEL]}) -eq 0 ]; then
    echo_stderr "  Ethernet is Master"
  else
    echo_stderr "  Connector is Master"
  fi

  if [ $(get_io_pin ${IO_PINS[OEn]}) -eq 1 ]; then
    echo_stderr "  OE is Disabled"
  else
    echo_stderr "  OE Enabled"
  fi

  if [ $(get_io_pin ${IO_PINS[RSTn]}) -eq 1 ]; then
    echo_stderr "  Reset is Disabled"
  else
    echo_stderr "  Reset is Enabled"
  fi


}

get_chain_mask() {
  VALUE=0
  PIN=$(get_io_pin ${IO_PINS[LP_SEL_0]})
  VALUE=$(($VALUE + ($PIN<<0) ))

  PIN=$(get_io_pin ${IO_PINS[LP_SEL_1]})
  VALUE=$(($VALUE + ($PIN<<1) ))

  PIN=$(get_io_pin ${IO_PINS[LP_SEL_2]})
  VALUE=$(($VALUE + ($PIN<<2) ))

  PIN=$(get_io_pin ${IO_PINS[LP_SEL_3]})
  VALUE=$(($VALUE + ($PIN<<3) ))

  PIN=$(get_io_pin ${IO_PINS[LP_SEL_4]})
  VALUE=$(($VALUE + ($PIN<<4) ))

  echo ${VALUE}
}

# Arg1 = new mask
set_chain_mask() {
  MASK=${1}
  echo_stderr "New chain Mask ${MASK}"
  ## Loop over all 5 bits, checking with
  ## site 0 is the backplane or zed connector.

    if [ $((${MASK} & 1)) -ne 0 ]; then
      set_io_pin ${IO_PINS[LP_SEL_0]} 1
    else
      set_io_pin ${IO_PINS[LP_SEL_0]} 0
    fi

    ## Site 1 = QFDB_B
    if [ $((${MASK} & 2)) -ne 0 ]; then
      set_io_pin ${IO_PINS[LP_SEL_1]} 1
    else
      set_io_pin ${IO_PINS[LP_SEL_1]} 0
    fi

    ## Site 2 = QFDB_B
    if [ $((${MASK} & 4)) -ne 0 ]; then
      set_io_pin ${IO_PINS[LP_SEL_2]} 1
    else
      set_io_pin ${IO_PINS[LP_SEL_2]} 0
    fi

    ## Site 3 = QFDB_B
    if [ $((${MASK} & 8)) -ne 0 ]; then
      set_io_pin ${IO_PINS[LP_SEL_3]} 1
    else
      set_io_pin ${IO_PINS[LP_SEL_3]} 0
    fi

    ## Site 4 = QFDB_B
    if [ $((${MASK} & 16)) -ne 0 ]; then
      set_io_pin ${IO_PINS[LP_SEL_4]} 1
      set_io_pin ${IO_PINS[LED]} 0
    else
      set_io_pin ${IO_PINS[LP_SEL_4]} 0
      set_io_pin ${IO_PINS[LED]} 1
    fi
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

if [ -d ${GPIO_BASE} ]; then
  echo_stderr "Running on real hardware with real I/O Extender!"
else
  echo_stderr "Running on fake hardware without I/O Extender!"
fi

# extract options and their arguments into variables.
while true ; do

  case "$1" in
      --trans)
        ARG_TRANS=1 ; ARG_ANY=1; shift ;;

      --notrans)
        ARG_NOTRANS=1 ; ARG_ANY=1; shift ;;

      --addmask)
        ARG_ADDMASK=1 ; ARG_ANY=1; shift ;;

      --noaddmask)
        ARG_NOADDMASK=1 ; ARG_ANY=1; shift ;;

      --ethernet)
        ARG_ETHERNET=1 ; ARG_ANY=1; shift ;;

      --connector)
        ARG_CONNECTOR=1 ; ARG_ANY=1; shift ;;

      --reset)
        ARG_RESET=1 ; ARG_ANY=1; shift ;;

      --oe)
        ARG_OE=1 ; ARG_ANY=1; shift ;;

      --nooe)
        ARG_NOOE=1 ; ARG_ANY=1; shift ;;

      --bridge)
        ARG_BRIDGE=1 ; ARG_ANY=1; shift ;;

      --stitcher)
        ARG_STITCHER=1 ; ARG_ANY=1; shift ;;

      --debug)
        ARG_DEBUG=1 ; shift ;;

      --ioreset)
        ## Perform a reset on the I/O Expander, putting everything into tristate.
        echo_stderr "Resetting I/O Expander"
        set_io_pin ${IO_PINS[RSTn]} 0
        ## Put it into reset, then put all pins into tristate input mode.

        set_io_pin ${IO_PINS[TRANS]} -1
        set_io_pin ${IO_PINS[MASK]} -1
        set_io_pin ${IO_PINS[TLR_TRST_6]} -1
        set_io_pin ${IO_PINS[TLR_TRST]} -1
        set_io_pin ${IO_PINS[SB_Bn]} -1
        set_io_pin ${IO_PINS[MPSEL]} -1
        set_io_pin ${IO_PINS[OEn]} -1
        set_io_pin ${IO_PINS[LP_SEL_0]} -1
        set_io_pin ${IO_PINS[LP_SEL_4]} -1
        set_io_pin ${IO_PINS[LP_SEL_4]} -1
        set_io_pin ${IO_PINS[LP_SEL_4]} -1
        set_io_pin ${IO_PINS[LP_SEL_4]} -1
        set_io_pin ${IO_PINS[LED]} -1

        ## Release the reset!
        set_io_pin ${IO_PINS[RSTn]} -1
        echo_stderr "Resetting I/O Expander complete"
        exit 1 ;;

      --help)
        echo_stderr "Track1 JTAG Controller Setup Application"
        echo_stderr
        echo_stderr " jtag_setup.sh <options> [site select] ..."
        echo_stderr "  Options."
        echo_stderr "    --help      : Print this message"
        echo_stderr "    --debug     : Display more verbose messages"
        echo_stderr "    --trans     : Transparent mode"
        echo_stderr "    --notrans   : non-Transparent mode"
        echo_stderr "    --addmask   : mask lower addresses"
        echo_stderr "    --noaddmask : do not mask lower addresses"
        echo_stderr "    --ethernet  : Ethernet access is Master"
        echo_stderr "    --connector : Front Panel access is Master"
        echo_stderr "    --reset     : Reset SCANSTA chip"
        echo_stderr "    --ioreset   : Reset SCANSTA I/O to power up defaults."
        echo_stderr "    --oe        : Output Enable"
        echo_stderr "    --nooe      : Output Disable"
        echo_stderr "    --bridge    : Bridge Mode"
        echo_stderr "    --stitcher  : Stitcher Mode "
        echo_stderr
        echo_stderr "  [site select] is a list of sites to include in the chain."
        echo_stderr
        echo_stderr "  QFDB_A QFDB_B QFDB_C QFDB_D EXT, multiple sites are allowed."
        echo_stderr

        exit 1 ;;

      --) shift ; break ;;

      *) echo_stderr "Internal error!" ; exit 1 ;;
  esac
done
# do something with the variables -- in this case just dump them out
if [ ${ARG_DEBUG} -eq 1 ]; then
  echo_stderr "ARG_ANY       = $ARG_ANY"
  echo_stderr "ARG_TRANS     = $ARG_TRANS"
  echo_stderr "ARG_NOTRANS   = $ARG_NOTRANS"
  echo_stderr "ARG_ADDMASK   = $ARG_ADDMASK"
  echo_stderr "ARG_NOADDMASK = $ARG_NOADDMASK"
  echo_stderr "ARG_ETHERNET  = $ARG_ETHERNET"
  echo_stderr "ARG_CONNECTOR = $ARG_CONNECTOR"
  echo_stderr "ARG_RESET     = $ARG_RESET"
  echo_stderr "ARG_OE        = $ARG_OE"
  echo_stderr "ARG_NOOE      = $ARG_NOOE"
  echo_stderr "ARG_BRIDGE    = $ARG_BRIDGE"
  echo_stderr "ARG_STITCHER  = $ARG_STITCHER"

fi
## "$#" will be the number of arguments left, which are the chip/port numbers
## Parse these, to not what port ot use.
##
JTAG_CHAIN_MASK=$(get_chain_mask)
echo_stderr "Existing chain ${JTAG_CHAIN_MASK}"
if [ $# -gt 0 ]; then
  JTAG_CHAIN_MASK=0
  echo_stderr "Altering chain mask"
  while [ $# -gt 0 ] ; do
    if [ ${SITE_NAMES_REVERSE[${1^^}]+_} ]; then
      echo_stderr "Site added ${1^^}"
      JTAG_CHAIN_MASK=$(($JTAG_CHAIN_MASK+(1<<$((${SITE_NAMES_REVERSE[${1^^}]})))))
    else
      echo_stderr "Unknown site ${1}!!"
    fi

    shift
  done

  echo_stderr "CHAIN_MASK = $JTAG_CHAIN_MASK"
  ## Now set the bits needed
  set_chain_mask ${JTAG_CHAIN_MASK}
fi




## Any args set? process them..
if [ ${ARG_ANY} -eq 1 ]; then
  ## Check for clashes
  if [ ${ARG_TRANS} -eq 1 ] && [ ${ARG_NOTRANS} -eq 1 ]; then
    echo_stderr "Cannot Specify --trans and --notrans!"; exit 10
  fi

  if [ ${ARG_OE} -eq 1 ] && [ ${ARG_NOOE} -eq 1 ]; then
    echo_stderr "Cannot Specify --oe and --nooe!"; exit 10
  fi

  if [ ${ARG_CONNECTOR} -eq 1 ] && [ ${ARG_ETHERNET} -eq 1 ]; then
    echo_stderr "Cannot Specify --connector and --ethernet!"; exit 10
  fi

  if [ ${ARG_BRIDGE} -eq 1 ] && [ ${ARG_STITCHER} -eq 1 ]; then
    echo_stderr "Cannot Specify --bridge and --stitcher!"; exit 10
  fi

  if [ ${ARG_ADDMASK} -eq 1 ] && [ ${ARG_NOADDMASK} -eq 1 ]; then
    echo_stderr "Cannot Specify --addmask and --noaddmask!"; exit 10
  fi

  ## Now we can process and set the GPIOs as needed.

  if [ ${ARG_TRANS} -eq 1 ]; then
    set_io_pin ${IO_PINS[TRANS]} 1
  fi

  if [ ${ARG_NOTRANS} -eq 1 ]; then
    set_io_pin ${IO_PINS[TRANS]} 0
  fi

  if [ ${ARG_ADDMASK} -eq 1 ]; then
      set_io_pin ${IO_PINS[ADDMASK]} 1
  fi

  if [ ${ARG_NOADDMASK} -eq 1 ]; then
    set_io_pin ${IO_PINS[ADDMASK]} 0
  fi

  if [ ${ARG_ETHERNET} -eq 1 ]; then
    set_io_pin ${IO_PINS[MPSEL]} 0
  fi

  if [ ${ARG_CONNECTOR} -eq 1 ]; then
    set_io_pin ${IO_PINS[MPSEL]} 1
  fi

  if [ ${ARG_BRIDGE} -eq 1 ]; then
    set_io_pin ${IO_PINS[SB_Bn]} 0
  fi

  if [ ${ARG_STITCHER} -eq 1 ]; then
      set_io_pin ${IO_PINS[SB_Bn]} 1
  fi

  if [ ${ARG_OE} -eq 1 ]; then
      set_io_pin ${IO_PINS[OEn]} 0
  fi

  if [ ${ARG_NOOE} -eq 1 ]; then
    set_io_pin ${IO_PINS[OEn]} 1
  fi

  ## This does a system reset.
  if [ ${ARG_RESET} -eq 1 ]; then
      set_io_pin ${IO_PINS[RSTn]} 0
      sleep 2
      set_io_pin ${IO_PINS[RSTn]} 1
  fi

  ## Dump these out.
  do_status_dump

else
  do_status_dump
fi
