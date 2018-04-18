#!/usr/bin/env python
#
# Copyright 2015-present Facebook. All Rights Reserved.
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
from node import node
from pal import *

class mezzNode(node):
    def __init__(self, info = None, actions = None):
      if info == None:
          self.info = {}
      else:
          self.info = info

      if actions == None:
          self.actions = []
      else:
          self.actions = actions

    def getInformation(self):
      name = pal_get_platform_name().decode()
      tpfile = open("/tmp/mezzanine/SLOT_ID")
      slotID = int(tpfile.readline(4))
      tpfile.close()
      result = {
          "Description": name + " Mezzanine Card",
          "Slot": repr(slotID),
      }

      return result

def get_node_mezz():

    return mezzNode()
