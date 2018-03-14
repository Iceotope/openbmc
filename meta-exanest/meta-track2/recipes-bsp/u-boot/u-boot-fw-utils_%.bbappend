# This is a uboot extra customisation for our verison,
# we need the custom platform init

FILESEXTRAPATHS_prepend := "${THISDIR}/u-boot-xlnx:"
## Adds our board config into the mix
SRC_URI_append = " file://0001-Adding-in-the-iceotop-bmc-config.patch \
    "
