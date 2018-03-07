# Iceotope Kernel config extras for Zynq BMC system

FILESEXTRAPATHS_prepend := "${THISDIR}/config:"

SRC_URI_append = " \
    file://track2;type=kmeta;destsuffix=track2 \
    "

KERNEL_FEATURES_append = " track2/track2.scc"
LINUX_VERSION_EXTENSION = "-xilinx-iceotope"
