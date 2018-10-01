#!/usr/bin/env python
#
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
#


import os
from subprocess import *
from node import node
from pal import *

slot_names = ["none" ,"QFDB", "KDB", "TPDB", "AUTO"]

class serverNode(node):
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

    def getInformation(self, param={}):
        ret = pal_get_server_power(self.num)
        if ret == 0:
            status = 'power-off'
        elif ret == 1:
            status = 'power-on'
        elif ret == 2:
            status = 'power-cycle'
        elif ret == 3:
            status = 'reset'
        elif ret == 4:
            status = 'awaiting power off'
        elif ret == 5:
            status = 'main power off'
        else:
            status = 'error'

        typeFile = open("/tmp/mezzanine/db_" + repr(self.num) + "/type","r")
        nodeType = int(typeFile.readline(4))
        typeFile.close()
        if nodeType > 3:
          node = slot_names[4]
        else:
          node = slot_names[nodeType]

        info = { "status": status,
                 "nodeType" : node
               }

        return info

    def doAction(self, data):
        if pal_server_action(self.num, data["action"].lower()) == -1:
            res = 'failure'
        else:
            res = 'success'

        result = { "result": res }

        return result

def get_node_server(num):
    actions =  ["power-on",
                "power-off",
                "power-reset",
                "power-cycle",
                "graceful-shutdown",
                "12V-on",
                "12V-off",
                "12V-cycle",
                "identify-on",
                "identify-off",
                ]
    return serverNode(num = num, actions = actions)
