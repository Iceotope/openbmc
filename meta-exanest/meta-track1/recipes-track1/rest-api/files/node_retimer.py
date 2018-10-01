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
from smbus2 import SMBus
from time import sleep

retimer_address = [0x18,0x1A,0x1B,0x20,0x22,0x23]
clock_speeds = ["25Mhz", "125Mhz", "312.5Mhz", "BAD"]
#retimer_address = [0x1B,0x20,0x22,0x23]

valid_channels=[0,1,2,3,4,5,10,11,12,13,14,15]

GLOBAL_CDR_REG=0xB
GLOBAL_CDR_LOCKED=0x40
GLOBAL_CLK_REG=0x2

CHANNEL_INFO_REG=0x78
CHANNEL_CDR_LOCK=0x10
CHANNEL_SIGNAL_DET=0x20

CHANNEL_REG_0A=0x0A

GLOBAL_REG=0xFF
GLOBAL_REG_CHANNEL=0x1
GLOBAL_REG_SHARED=0x0

CHANNEL_SEL_LOW=0xFC
CHANNEL_SEL_HIGH=0xFD

CHANNEL_SEL_LOW_MASK=0x3F
CHANNEL_SEL_HIGH_MASK=0xFC

RETIMER_BUS=0x1

# Write a config file into a retimer
# Script has the i2c address encoded in it, so we dont have to know
# it up front. Not elegent but for now it works.
def writeRetimerConfig(scriptfile):
  res='failure'
  failed=False
  if os.path.isfile(scriptfile):
    bus=SMBus(RETIMER_BUS)
    retimerconfigfile = open(scriptfile,"r")
    if (bus):
      line = retimerconfigfile.readline()
      ## Parse the line, skip bad lines and write good ones to the i2c
      ## format is
      #I2C bus, I2C Chip Address, Register address, Register value
      while line:
        if not line.startswith("#"):
          line = line.replace("\n", "")
#          print("Config Line: "+line)
          config_line=line.split(',')
          if len(config_line) > 3:
            chip=int(config_line[1],16)
            reg=int(config_line[2],16)
            reg_value=int(config_line[3],16)
            #print("Device: "+repr(chip)+" Reg: "+repr(reg)+" Value: "+repr(reg_value))
            try:
              bus.write_byte_data(chip, reg, reg_value)
            except:
              failed=True
#              print("Failed to write configline: "+line)

        line = retimerconfigfile.readline()

      retimerconfigfile.close()
      bus.close()
      if not failed:
        res='success'

  return res



# Get clock info
def getRetimerClkInfo(num):
  clock=0
  locked=0
  retimer = retimer_address[num]
  #print("Retimer @0x{:02x}".format(retimer))
  bus = SMBus(RETIMER_BUS)
  #print("Bus Open")

  bus.write_byte_data(retimer, GLOBAL_REG, GLOBAL_REG_SHARED)
  #print("Write done")
  locked= bool(bus.read_byte_data(retimer, GLOBAL_CDR_REG) & GLOBAL_CDR_LOCKED)
  clock = bus.read_byte_data(retimer, GLOBAL_CLK_REG)
  #print("Retimer @0x{:02x} CLK_CONF: 0x{:02x} CLK_DETECT: {}".format(retimer,clock,locked))
  bus.close()
  result = {"clock": clock, "locked":locked}
  return result

def getRetimerLinkInfo(num):
  chan_dict={}
  retimer = retimer_address[num]
  #print("Retimer @0x{:02x}".format(retimer))
  bus = SMBus(RETIMER_BUS)
  #print("Bus Open")

  # Shared regs, select channel
  for channel in valid_channels:
    try:
      bus.write_byte_data(retimer, GLOBAL_REG, GLOBAL_REG_SHARED)
      #print("Global_Reg Selected")
      if channel<8:
        bus.write_byte_data(retimer, CHANNEL_SEL_LOW, (1<<channel))
        bus.write_byte_data(retimer, CHANNEL_SEL_HIGH, 0)
      else:
        bus.write_byte_data(retimer, CHANNEL_SEL_LOW,  0)
        bus.write_byte_data(retimer, CHANNEL_SEL_HIGH,(1<<channel-8))

      bus.write_byte_data(retimer, GLOBAL_REG, GLOBAL_REG_CHANNEL)
      #print("Channel_Selected")
      chan_detect=bus.read_byte_data(retimer, CHANNEL_INFO_REG)
      #print("register read @0x{:02x}".format(chan_detect))
      signal_det=bool(chan_detect & CHANNEL_SIGNAL_DET)
      cdr_lock=bool(chan_detect & CHANNEL_CDR_LOCK)
      #print("build_dict")
      chan_dict["channel_"+repr(channel)] = {}
      chan_dict["channel_"+repr(channel)]["Locked"] = repr(cdr_lock)
      chan_dict["channel_"+repr(channel)]["Signal_Detect"] = repr(signal_det)
    except:
      chan_dict["channel_"+repr(channel)] = {}
      chan_dict["channel_"+repr(channel)]["Locked"] = "Error"
      chan_dict["channel_"+repr(channel)]["Signal_Detect"] = "Error"

  bus.close()

  # Loop over all the retimers, pull all info out.
  return chan_dict

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

    def getInformation(self, param={}):
        clkinfo=getRetimerClkInfo(self.num)
        clkspeed=(clkinfo["clock"]>>5) & 0x3
        channels=getRetimerLinkInfo(self.num)
        info = {  "Description": "Retimer "+repr(self.num)+" (@0x{:02x})".format(retimer_address[self.num]),
          "Clock":clock_speeds[clkspeed],
          "CDR_Lock":repr(clkinfo["locked"]),
          "Channels":channels,
        }

        return info
#
# data["action"] will be the command
# data["value"] will be the argument

    def doAction(self, data):
      res = 'failure'

      if data["action"].lower() == "cdr_reset":
        retimer = retimer_address[self.num]
        #print("Retimer @0x{:02x}".format(retimer))
        bus = SMBus(RETIMER_BUS)
        #print("Bus Open")
        bus.write_byte_data(retimer, GLOBAL_REG, GLOBAL_REG_SHARED)
        bus.write_byte_data(retimer, CHANNEL_SEL_LOW, CHANNEL_SEL_LOW_MASK)
        bus.write_byte_data(retimer, CHANNEL_SEL_HIGH, CHANNEL_SEL_HIGH_MASK)
        bus.write_byte_data(retimer, CHANNEL_REG_0A, 0x54)
        sleep(0.25) # Time in seconds.
        bus.write_byte_data(retimer, CHANNEL_REG_0A, 0x50)

        #print("Write done")
        res = 'success'

      result = { "result": res }
      return result

def get_node_retimer(num):
    actions =  ["cdr_reset",
                ]
    return retimerNode(num = num, actions = actions)
