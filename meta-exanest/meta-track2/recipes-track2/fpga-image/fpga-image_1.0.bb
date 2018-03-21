# This installs the FPGA image into the deploy directory
# Also copies over a elf for the fsbl, as the xilinx tools need it to
# flash the images.


DESCRIPTION = "Copies over the FPGA and other hardware related files"
SECTION = "bsp"
DEPENDS = ""
LICENSE = "Proprietary"

FILESEXTRAPATHS_append := "${THISDIR}/files:"

LIC_FILES_CHKSUM = "file://LICENSE;md5=cb1a05000bf35f0c896795edd999614d"

BITFILE="Z_system_wrapper.bit"
BITFILE_DEST="Z_system_wrapper.bit"

inherit deploy

SRC_URI = " file://zynq_fsbl.elf \
     file://Z_system_wrapper.bit \
     file://flash_zynq.sh \
     file://LICENSE \
    "

S = "${WORKDIR}"


do_deploy () {
  install -d ${DEPLOYDIR}
  install -m 644 ${S}/${BITFILE} ${DEPLOYDIR}/${BITFILE_DEST}
  install -m 644 ${S}/zynq_fsbl.elf ${DEPLOYDIR}/zynq_fsbl.elf
  install -m 744 ${S}/flash_zynq.sh ${DEPLOYDIR}/flash_zynq.sh
}
addtask deploy before do_build after do_compile
