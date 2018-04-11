# Copyright 2015-present Facebook. All Rights Reserved.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://mTerm1/run \
            file://mTerm2/run \
            file://mTerm3/run \
            file://mTerm4/run \
            file://mTerm5/run \
            file://mTerm6/run \
            file://mTerm7/run \
            file://mTerm8/run \
            file://mTerm9/run \
            file://mTerm10/run \
            file://mTerm11/run \
            file://mTerm12/run \
           file://mTerm-service-setup.sh \
           "

S = "${WORKDIR}"

# launch 12 mTerm services, 2 for each server
MTERM_SERVICES = "mTerm1 \
                  mTerm2 \
                  mTerm3 \
                  mTerm4 \
                  mTerm5 \
                  mTerm6 \
                  mTerm7 \
                  mTerm8 \
                  mTerm9 \
                  mTerm10 \
                  mTerm11 \
                  mTerm12 \
                 "
