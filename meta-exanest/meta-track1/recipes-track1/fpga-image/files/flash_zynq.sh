#! /bin/sh
#
# This calls the Xilinx tools to flash things, assumes path is okay
# already
#
program_flash -f ./boot.bin -fsbl ./zynq_fsbl.elf \
  -flash_type qspi_single -blank_check -verify -cable type xilinx_tcf \
  url tcp:localhost:3121

