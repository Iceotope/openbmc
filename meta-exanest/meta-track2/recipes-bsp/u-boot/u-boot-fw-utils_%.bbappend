# This is a uboot extra customisation for our verison,
# we need the custom platform init

FILESEXTRAPATHS_prepend := "${THISDIR}/u-boot-xlnx:"
## Adds our board config into the mix
SRC_URI_append = " file://0001-Adding-in-the-iceotop-bmc-config.patch \
           file://fw_env.config \
    "

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
