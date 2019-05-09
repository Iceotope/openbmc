#! /bin/sh
#
# This calls the Xilinx tools to flash things, assumes path is okay
# already
#

echo
echo "Flashing \"$1\" into zynq via JTAG"
echo

program_flash -f $1 -fsbl ./zynq_fsbl.elf \
  -flash_type qspi_single -verify -cable type xilinx_tcf \
  url tcp:localhost:3121

