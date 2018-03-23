# Iceotope Kernel config extras for Zynq BMC system

FILESEXTRAPATHS_prepend := "${THISDIR}/config:"

SRC_URI_append = " \
    file://track1;type=kmeta;destsuffix=track1 \
    "

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
SRC_URI_append = " file://uartlite.patch \
    "

KERNEL_FEATURES_append = " track1/track1.scc"
LINUX_VERSION_EXTENSION = "-xilinx-iceotope"
