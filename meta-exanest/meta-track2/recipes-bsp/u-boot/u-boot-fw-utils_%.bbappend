# This is a uboot extra customisation for our verison,
# we need the custom platform init

FILESEXTRAPATHS_prepend := "${THISDIR}/u-boot-xlnx:"
## Adds our board config into the mix
SRC_URI_append = " file://ps7_init_gpl.c \
    file://ps7_init_gpl.h \
    file://zynq_iceotope_bmc.h\
    file://zynq_iceotope_bmc_defconfig\
    file://fw_env.config \
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

do_install_append () {
if [ -e ${WORKDIR}/fw_env.config ] ; then
        install -d ${D}${sysconfdir}
        install -m 644 ${WORKDIR}/fw_env.config ${D}${sysconfdir}/fw_env.config
        install -m 644 ${WORKDIR}/fw_env.config ${D}${sysconfdir}/fw_env.config.normal
    fi
    if [ -e ${WORKDIR}/fw_env.config.full ] ; then
        install -d ${D}${sysconfdir}
        install -m 644 ${WORKDIR}/fw_env.config.full ${D}${sysconfdir}/fw_env.config.full
    fi
}
