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
from time import sleep

from slot_names import *

# Modes.
BOOTMODE_JTAG=0
BOOTMODE_QSPI=6

def getKeyValue(name):
    key_status=""
    data = Popen('cat /mnt/data/kv_store/' + name, \
                        shell=True, stdout=PIPE).stdout.read().decode()
    key_status = data.strip('\n')
    return(key_status)



def setGPIO(site, io, value):
  gpioValues = [0] * 16
## We need to get all the GPIOs
  for i in range(0, 16):
    gpioPath="/tmp/mezzanine/site_"+repr(site)+"/gpio/IO/"+repr(io)+"/value"
    if os.path.isfile(gpioPath):
      try:
        gpiofile = open(gpioPath,"w")
        # Read the file, store the data value in an array
        gpiofile.write(repr(value)+"\n")
        gpiofile.close()
      except:
        pass

## Set the hardware, need to just check the 3 bits.
def set_hardware_boot(site, value):
    setGPIO(site, 4, (value & 0x1))
    setGPIO(site, 5, (value & 0x2)>>1)
    setGPIO(site, 6, (value & 0x3)>>2)

def set_site_jtag(site):
  pal_set_key_value("slot"+repr(site)+"_boot_order", repr(BOOTMODE_JTAG))
  ## Now set the hardware to match!
  set_hardware_boot(site, BOOTMODE_JTAG)
  return 'success'


def set_site_qspi(site):
  pal_set_key_value("slot"+repr(site)+"_boot_order", repr(BOOTMODE_QSPI))
  ## Now set the hardware to match!
  set_hardware_boot(site, BOOTMODE_QSPI)
  return 'success'

class bootmodeNode(node):
    def __init__(self, info = None, actions = None):

        if info == None:
            self.info = {}
        else:
            self.info = info
        if actions == None:
            self.actions = []
        else:
            self.actions = actions

## Loop over all sites, getting the bootmode ffrom the KV store

    def getInformation(self, param={}):
      info = {}
      for i in range(1, 8):
        keyval = getKeyValue("slot"+repr(i)+"_boot_order")
        if (keyval == "0"):
          mode="JTAG"
        elif (keyval == "6"):
          mode="QSPI"
        else:
          mode="UNKNOWN"
        info[get_slot_name_from_number(i)] = mode

      return info
#
# data["action"] will be the node to access, qfdb-a etc.
# data["value"] will be the argument, QSPI, JTAG etc.

    def doAction(self, data):
      res = 'failure'
      action=data["action"].lower()
      mode=data["value"].lower()
      # Look up the slot number.
      if does_site_exist(action):
        site=get_site_number_from_name(action)
        if (mode == "qspi"):
          res=set_site_qspi(site)
        elif (mode == "jtag"):
          res=set_site_jtag(site)

      result = { "result": res }
      return result

def get_node_bootmode():
    actions =  ["qfdb_a", "qfdb_b",
                "qfdb_c", "qfdb_d",
                "kdb_a", "kdb_b",
                "tpdb_a", "tpdb_b"
                ]
    return bootmodeNode( actions = actions)
