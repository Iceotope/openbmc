#!/usr/bin/env python
#
# Copyright 2018-present Icerotope. All Rights Reserved.
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


import os
from subprocess import *
from node import node
from pal import *

class retimerNode(node):
    def __init__(self, num = None, info = None, actions = None):
        self.num = num

        if info == None:
            self.info = {}
        else:
            self.info = info
        if actions == None:
            self.actions = []
        else:
            self.actions = actions

    def getInformation(self):

        ## Pull a lot of stuff out of the retimer for status and such
        ## Might need external code to do the i2c operations?

        info = {  "Description": "Dummy Retimer Node "+repr(self.num),
               }

        return info

    def doAction(self, data):

        res = 'success'

        result = { "result": res }

        return result

def get_node_retimer(num):
    actions =  ["dummy",
                ]
    return retimerNode(num = num, actions = actions)
