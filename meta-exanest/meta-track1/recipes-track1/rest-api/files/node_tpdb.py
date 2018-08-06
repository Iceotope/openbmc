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


import os
from subprocess import *
from node import node
from pal import *
from tree import tree
## CLASSES for the sub nodes.
#
# LEDs
#
class tpdbNodeLed(node):
    def __init__(self, node = None, num = None, info = None, actions = None):
        self.num = num
        self.node = node

        if info == None:
            self.info = {}
        else:
            self.info = info
        if actions == None:
            self.actions = []
        else:
            self.actions = actions

    def getInformation(self, param={}):
        ledStatus="0ff"
        ledPath="/tmp/mezzanine/db_" + repr(self.node) + "/gpio/IO/" + repr(6-self.num) + "/value"
        if os.path.isfile(ledPath):
          tpfile = open(ledPath,"r")
          ledValue = int(tpfile.readline(1))
          tpfile.close()
          if ledValue == 0:
            ledStatus = "On"
          else:
            ledStatus = "Off"

        info = { 'led'+repr(self.num): { 'status': ledStatus}}
        return info

    def doAction(self, data):
        print("Doing action on module "+ repr(self.node))
        ledPath="/tmp/mezzanine/db_" + repr(self.node) + "/gpio/IO/" + repr(6-self.num) + "/value"
        res = 'failure'

        if os.path.isfile(ledPath):
          tpfile = open(ledPath,"w")
          if data["action"] == "on":
            tpfile.write ("0\n")
            res = 'success'
          elif data["action"] == "off":
            tpfile.write ("1\n")
            res = 'success'

          tpfile.close()

        result = { "result": res }

        return result


def get_node_tpdb_led(node, num):
    actions =  ["on", "off", ]
    return tpdbNodeLed(node = node, num = num, actions = actions)

#
# PWMs
#
class tpdbNodePwm(node):
    def __init__(self, node = None, num = None, info = None, actions = None):
      self.num = num
      self.node = node
      self.duty = 0
      self.freq = 0

      if info == None:
        self.info = {}
      else:
        self.info = info
      if actions == None:
        self.actions = []
      else:
        self.actions = actions

    def getPeriod(self):
      period = 0.0
      filePath="/tmp/mezzanine/db_" + repr(self.node) + "/pwmchip/pwm" + repr(self.num) + "/period"
      if os.path.isfile(filePath):
        tpfile=open(filePath,"r")
        period=float(tpfile.readline(20))
        tpfile.close()
      return period

    def getFreq(self):
      period=self.getPeriod()
      if ( period >0):
        period=period*1e-9
        self.freq=1/(period)
      else:
        self.freq=0

      return self.freq

    def getDutyPeriod(self):
      period = 0.0
      filePath="/tmp/mezzanine/db_" + repr(self.node) + "/pwmchip/pwm" + repr(self.num) + "/duty_cycle"
      if os.path.isfile(filePath):
        tpfile=open(filePath,"r")
        period=float(tpfile.readline(20))
        tpfile.close()

      return period

    def getDutyCycle(self):
      duty_period=self.getDutyPeriod()
      period=self.getPeriod()
      if ( period >0):
        self.duty=(duty_period/period)*100.0
      else:
        self.duty=0


      return self.duty

    def getInformation(self, param={}):
        freqHz = 0
        duty_cycle_normal = 0
        enabled = 0
#       /tmp/mezzanine/db_<n>/pwmchip/pwm<i>
#       duty_cycle in ns
#       period in ns
#       enable bool
#
        freqHz=self.getFreq()
        duty_cycle_normal=self.getDutyCycle()

        basePath="/tmp/mezzanine/db_" + repr(self.node) + "/pwmchip/pwm" + repr(self.num)
        # Check it exists
        if os.path.isdir(basePath):

          filePath=basePath+"/enable"
          tpfile = open(filePath,"r")
          enabled = int(tpfile.readline(1))
          tpfile.close()

        info = {'freq': freqHz, 'duty': duty_cycle_normal, 'enable': enabled}
        return info

    def doAction(self, data):
      basePath="/tmp/mezzanine/db_" + repr(self.node) + "/pwmchip/pwm" + repr(self.num)
      res = 'failure'
      if os.path.isdir(basePath):
          if data["action"] == "freq":
            if "value" in data:
              self.freq=float(data["value"])
              period=int(1e9/self.freq)
              tpfile = open(basePath +"/period","w")
              tpfile.write("%u\n" % period)
              tpfile.close()
              res = 'success'

          elif data["action"] == "duty_cycle":
            period = self.getPeriod()
            if (period > 0):
              if "value" in data:
                new_duty=float(data["value"])
                print("Period(ns) %u\n" % period)
                duty_period=int(period*(new_duty/100.0))
                print("Duty %u\n" % duty_period)
                tpfile = open(basePath +"/duty_cycle","w")
                tpfile.write("%u\n" % duty_period)
                tpfile.close()
                self.duty=self.getDutyCycle()
                res = 'success'

          elif data["action"] == "enable":
            tpfile = open(basePath +"/enable","w")
            tpfile.write("1\n")
            tpfile.close()
            res = 'success'

          elif data["action"] == "disable":
            tpfile = open(basePath +"/enable","w")
            tpfile.write("0\n")
            tpfile.close()
            res = 'success'

      result = { "result": res }

      return result



def get_node_tpdb_pwm(node, num):
    actions =  ["enable", "disable", "freq", "duty_cycle"]
    return tpdbNodePwm(node = node, num = num, actions = actions)


#
# Main class
#
class tpdbNode(node):
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

#
#       Get master PWM enable, via the IO pin on the GPIO port
#
        gpioPath="/tmp/mezzanine/db_" + repr(self.num) + "/gpio/IO/0/value"
        tpfile = open(gpioPath,"r")
        master_enable = int(tpfile.readline(1))
        tpfile.close()

        if master_enable == 0:
          master_enable_status="Enabled"
        else:
          master_enable_status="Disabled"

#
#       EEPROM WP
#

        wpPath="/tmp/mezzanine/db_" + repr(self.num) + "/gpio/IO/7/value"
        if os.path.isfile(wpPath):
          tpfile = open(wpPath,"r")
          wpValue = int(tpfile.readline(1))
          tpfile.close()
          if wpValue == 0:
            wpStatus = "Disabled"
          else:
            wpStatus = "Enabled"


#
#       Build return array up
#
        info = { "EEPROM Write Protect": wpStatus,
                 "Master PWM Enable": master_enable_status, }
        return info

#
# data["action"] will be the command
# data["value"] will be the argument

    def doAction(self, data):
      wpPath="/tmp/mezzanine/db_" + repr(self.num) + "/gpio/IO/7/value"
      gpioPath="/tmp/mezzanine/db_" + repr(self.num) + "/gpio/IO/0/value"
      res = 'failure'

      if data["action"] == "master_pwm_on":
        if os.path.isfile(gpioPath):
          tpfile = open(gpioPath,"w")
          tpfile.write ("0\n")
          res = 'success'
          tpfile.close()

      elif data["action"] == "master_pwm_off":
        if os.path.isfile(gpioPath):
          tpfile = open(gpioPath,"w")
          tpfile.write ("1\n")
          res = 'success'
          tpfile.close()

      elif data["action"] == "enable_eeprom_wp":
        if os.path.isfile(wpPath):
          tpfile = open(wpPath,"w")
          tpfile.write ("1\n")
          res = 'success'
          tpfile.close()

      elif data["action"] == "disable_eeprom_wp":
        if os.path.isfile(wpPath):
          tpfile = open(wpPath,"w")
          tpfile.write ("0\n")
          res = 'success'
          tpfile.close()

      result = { "result": res }

      return result

def get_node_tpdb(num):
    actions =  ["master_pwm_on",
                "master_pwm_off",
                "enable_eeprom_wp",
                "disable_eeprom_wp",
                ]
    return tpdbNode(num = num, actions = actions)

