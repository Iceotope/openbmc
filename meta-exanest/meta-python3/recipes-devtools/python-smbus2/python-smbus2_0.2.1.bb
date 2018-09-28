SUMMARY = "A drop-in replacement for smbus-cffi/smbus-python in pure Python"
HOMEPAGE = "https://github.com/kplindegaard/smbus2"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://PKG-INFO;md5=751b587b4c6d296d3a0a0361f66e1bf2"

SRC_URI="https://files.pythonhosted.org/packages/c6/be/6eab4b27693ec2c87f7ff864dfca86c58fbfd1627acbe191dd2f18e0ac3e/smbus2-0.2.1.tar.gz"
SRC_URI[md5sum] = "de1f6a4eb5aaef90ae30fb9d3cab5145"

#SRC_URI += "file://add_missing_CHANGES_md.patch"

PYPI_SRC_URI = "https://files.pythonhosted.org/packages/c6/be/6eab4b27693ec2c87f7ff864dfca86c58fbfd1627acbe191dd2f18e0ac3e/smbus2-0.2.1.tar.gz"
PYPI_PACKAGE = "smbus2"

PYPI_PACKAGE_EXT = "tar.gz"

inherit pypi
inherit setuptools3

BBCLASSEXTEND = "native nativesdk"
# Python3 only. Override py2 packages
RDEPENDS_${PN}_remove = "python-core"
RDEPENDS_${PN} += "python3-core"
#RDEPENDS_${PN} = "python3"
