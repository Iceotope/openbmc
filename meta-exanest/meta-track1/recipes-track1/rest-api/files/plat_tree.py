#!/usr/bin/env python
#
# Copyright 2014-present Facebook. All Rights Reserved.
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

from ctypes import *
import json
import ssl
import socket
import os
from node_api import get_node_api
from node_spb import get_node_spb
from node_mezz import get_node_mezz
from node_bmc import get_node_bmc
from node_server import get_node_server
from node_fruid import get_node_fruid
from node_sensors import get_node_sensors
from node_logs import get_node_logs
from node_config import get_node_config
from node_tpdb import get_node_tpdb
from node_tpdb import get_node_tpdb_led
from node_tpdb import get_node_tpdb_pwm
from node_retimers import get_node_retimers
from node_retimer import get_node_retimer
from node_jtag import get_node_jtag
from node_identify import get_node_identify
from node_bootmode import get_node_bootmode
from tree import tree
from pal import *

from slot_names import *

def populate_server_node(num):
    slot=get_slot_name_from_number(num)
    prsnt = pal_is_fru_prsnt(num)
    if prsnt == None or prsnt == 0:
        print("Not present")
        return None

    r_server = tree(get_slot_name_from_number(num), data = get_node_server(num))

    r_fruid = tree("fruid", data = get_node_fruid(slot))

    r_sensors = tree("sensors", data = get_node_sensors(slot))

    r_logs = tree("logs", data = get_node_logs(slot))

    r_config = tree("config", data = get_node_config(slot))

    r_server.addChildren([r_fruid, r_sensors, r_logs, r_config])

# Need to check the type here, and add the required tpdb, kdb or qfdb nodes
    typeFile = open("/tmp/mezzanine/db_" + repr(num) + "/type","r")
    nodeType = int(typeFile.readline(4))
    if nodeType == 1:
      # QFDB
      print("slot " + repr(num)  + " QFDB board")
    elif nodeType == 2:
      # KDB
      print("slot " + repr(num)  + " KDB board")
    elif nodeType == 3:
      # TPDB
      print("slot "+ repr(num)  + " TPDB board")
      r_tpdb = tree("tpdb", data = get_node_tpdb(num))
      if r_tpdb:
        # Leds on the board
        for i in range(0,4):
          r_tpdb.addChild(tree("led"+repr(i), data = get_node_tpdb_led(num, i)))

        # PWMs on the board
        for i in range(0,5):
          r_tpdb.addChild(tree("pwm"+repr(i), data = get_node_tpdb_pwm(num, i)))

        r_server.addChild(r_tpdb)
      else:
        print("Failed to add TPDB in slot " + repr(num))

    else:
      #Unknown!
      print("Unknown board in slot " + repr(num))

    return r_server

# Initialize Platform specific Resource Tree
def init_plat_tree():

    # Create /api end point as root node
    r_api = tree("api", data = get_node_api())

    # Add /api/mezz to represent main card
    r_mezz = tree("mezz", data = get_node_mezz())
    r_api.addChild(r_mezz)

    # Add servers /api/server[1-max]
    num = pal_get_num_slots()
    print("Slots : " + repr(num))
    for i in range(1, num+1):
      print("Doing slot "+ repr(i))
      r_server = populate_server_node(i)
      if r_server:
        r_mezz.addChild(r_server)
      else:
        print("Failed to add server " + repr(i))

    # /api/mezz/identify to flash the led
    r_identify = tree("identify", data = get_node_identify("mezzanine"))
    r_mezz.addChild(r_identify)

    # /api/mezz/bootmode for bootmode configs
    r_bootmode = tree("bootmode", data = get_node_bootmode())
    r_mezz.addChild(r_bootmode)

    # /api/mezz/bmc end point, and sub points
    r_bmc = tree("bmc", data = get_node_bmc())
    r_mezz.addChild(r_bmc)

    # Add /api/mezz/bmc/fruid end point
    r_temp = tree("fruid", data = get_node_fruid("bmc"))
    r_bmc.addChild(r_temp)

    # /api/mezz/bmc/sensors end point
    r_temp = tree("sensors", data = get_node_sensors("bmc"))
    r_bmc.addChild(r_temp)

    # /api/mezz/bmc/logs end point
    r_temp = tree("logs", data = get_node_logs("bmc"))
    r_bmc.addChild(r_temp)


    # Add Retimer locations, and each module
    r_retimers = tree("retimers", data = get_node_retimers())
    for i in range(0, 6):
      r_retimers.addChild(tree("retimer"+repr(i), data=get_node_retimer(i)))

    r_mezz.addChild(r_retimers)

    # Add JTAG controller
    r_jtag = tree("jtag", data = get_node_jtag())
    r_mezz.addChild(r_jtag)

    return r_api
