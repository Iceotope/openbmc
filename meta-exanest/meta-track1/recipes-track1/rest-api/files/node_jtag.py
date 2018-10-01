#!/usr/bin/env python
#
# Copyright 2018-present Iceotope. All Rights Reserved.
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
#

from time import sleep
from node import node
from pal import *
import os

class jtagNode(node):
    def __init__(self, info = None, actions = None):
        if info == None:
            self.info = {}
        else:
            self.info = info
        if actions == None:
            self.actions = []
        else:
            self.actions = actions

    def getInformation(self, param={}):
        name = pal_get_platform_name().decode()
        info = {
            "Description": name + " JTAG Controller",
           }

        ## We need to get all the GPIOs
        gpioPath="/tmp/mezzanine/jtag/gpio"
        return info

    def doAction(self, data):
        res = 'success'

        # Return result codes.
        result = { "result": res }
        return result

def get_node_jtag():
    actions =  ["reset",
                ]

    return jtagNode(actions = actions)
