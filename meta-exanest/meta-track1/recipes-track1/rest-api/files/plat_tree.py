#!/usr/bin/env python
#
# Copyright 2014-present Facebook. All Rights Reserved.
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
from tree import tree
from pal import *

slot_names = ["none" ,"tpdb-b", "tpdb-a", "kdb-b", "kdb-a", "qfdb-d", "qfdb-c", "qfdb-b", "qfdb-a" ]
def get_slot_name(num)
    return slot_names[num]

def populate_server_node(num):

    prsnt = pal_is_fru_prsnt(num)
    if prsnt == None or prsnt == 0:
        return None

    r_server = tree(get_slot_name(num), data = get_node_server(num))

    r_fruid = tree("fruid", data = get_node_fruid(slot))

    r_sensors = tree("sensors", data = get_node_sensors(slot))

    r_logs = tree("logs", data = get_node_logs(slot))

    r_config = tree("config", data = get_node_config(slot))

    r_server.addChildren([r_fruid, r_sensors, r_logs, r_config])

    return r_server

# Initialize Platform specific Resource Tree
def init_plat_tree():

    # Create /api end point as root node
    r_api = tree("api", data = get_node_api())

    # Add /api/mezz to represent Network Mezzaine card
    r_mezz = tree("mezz", data = get_node_mezz())
    r_api.addChild(r_mezz)

    # Add servers /api/server[1-max]
    num = pal_get_num_slots()
    print("Slots : " + repr(num))
    for i in range(1, num-1):
        r_server = populate_server_node(i)
        if r_server:
            r_api.addChild(r_server)

     # /api/mezz/bmc end point
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

    return r_api
