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
from node_retimer import writeRetimerConfig
import os

RETIMER_2v5_ON=1
RETIMER_2v5_OFF=0

RETIMER_2v5_RESET=0
RETIMER_2v5_NORMAL=1
configs = { "10Gbit Infiniband based" : "10GBIT_INFINIBAND",
                "10.3125Gbit Ethernet based" : "10GBIT_ETHERNET"}

# Paths to scripts.
config_scripts = {"10GBIT_INFINIBAND" : "/usr/local/packages/retimers/retimer_10gbps.csv",
                  "10GBIT_ETHERNET" : "/usr/local/packages/retimers/retimer_10_3125gbps.csv"}

class retimersNode(node):
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
            "Description": name + " SerialIO Retimers",
            "Configs" : configs,
           }

        return info

#
# data["action"] will be the command
# data["value"] will be the argument

    def doAction(self, data):
        res = 'failure'
        gpioPath="/tmp/mezzanine/gpio"

        # Turn on the 2V5
        if data["action"].lower() == "power-on":
          gpioPath=gpioPath+"/REG_2V5_ENABLE/value"
          if os.path.isfile(gpioPath):
            gpiofile = open(gpioPath,"w")
            gpiofile.write (repr(RETIMER_2v5_ON)+"\n")
            res = 'success'
            gpiofile.close()


        # Turn off the 2V5
        elif data["action"].lower() == "power-off":
          gpioPath=gpioPath+"/REG_2V5_ENABLE/value"
          if os.path.isfile(gpioPath):
            gpiofile = open(gpioPath,"w")
            gpiofile.write (repr(RETIMER_2v5_OFF)+"\n")
            res = 'success'
            gpiofile.close()


        # Load a config file/mode specified
        elif data["action"].lower() == "configure":
          config_name=data["value"]
          ## Check that the value tag exists!
          if config_name in config_scripts.keys():
            script_name=config_scripts[config_name]
            #print("Using config "+config_name+" File: "+script_name)
            # Call the code to write this out to the retimers
            res = writeRetimerConfig(script_name)


        # Toggle the reset line
        elif data["action"].lower() == "reset":
          gpioPath=gpioPath+"/RETIMER_RESET/value"
          if os.path.isfile(gpioPath):
            gpiofile = open(gpioPath,"w")
            gpiofile.write (repr(RETIMER_2v5_RESET)+"\n")
            sleep(10000/1000000) #10ms delay
            gpiofile.close()
            gpiofile = open(gpioPath,"w")
            gpiofile.write (repr(RETIMER_2v5_NORMAL)+"\n")
            res = 'success'
            gpiofile.close()

        # Return result codes.
        result = { "result": res }
        return result

def get_node_retimers():
    actions =  ["power-on",
                "power-off",
                "configure",
                "reset",
                ]

    return retimersNode(actions = actions)
