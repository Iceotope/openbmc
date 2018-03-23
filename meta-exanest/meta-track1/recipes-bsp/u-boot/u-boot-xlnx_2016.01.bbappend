# This is a uboot extra customisation for our verison,
# we need the custom platform init

# this matches u-boot-xlnx 'xilinx-v2016.3' release tag
SRCREV = "95e11f6eb4bc07bbee12a4217c58146bddac61b2"
PV = "v2016.03${XILINX_EXTENSION}+git${SRCPV}"

## Removed, we copy the files in directly after unpack/patch stage
#DEPENDS += "platform-init"

FILESEXTRAPATHS_prepend := "${THISDIR}/u-boot-xlnx:"

XILINX_EXTENSION = "-xilinx-iceotope"

UBOOT_ENV_track1 = "uEnv"

SRC_URI_append_track1 = " file://uEnv.txt"

## Adds our board config into the mix
SRC_URI_append = " file://0001-Adding-in-the-iceotop-bmc-config.patch \
    file://ps7_init_gpl.c \
    file://ps7_init_gpl.h \
    file://zynq_iceotope_bmc.h\
    file://zynq_iceotope_bmc_defconfig\
    "
do_configure_prepend () {
  # Copy over our inits
    install -d ${S}/board/xilinx/zynq/zynq_iceotope_bmc/
    cp -f ${WORKDIR}/ps7_init_gpl.h ${S}/board/xilinx/zynq/zynq_iceotope_bmc/
    cp -f ${WORKDIR}/ps7_init_gpl.c ${S}/board/xilinx/zynq/zynq_iceotope_bmc/
    cp -f ${WORKDIR}/zynq_iceotope_bmc.h  ${S}/include/configs/zynq_iceotope_bmc.h
    cp -f ${WORKDIR}/zynq_iceotope_bmc_defconfig  ${S}/configs/zynq_iceotope_bmc_defconfig
    cp -f ${S}/arch/arm/dts/zynq-microzed.dts ${S}/arch/arm/dts/zynq-iceotope_bmc.dts
}
