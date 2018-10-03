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
from subprocess import check_output,Popen

import os
import signal


TRANS=0
MASK=1
TLR_TRST_6=2
TLR_TRST=3
SB_Bn=4
MPSEL=5
RSTn=6
OEn=7
LP_SEL_0=8
LP_SEL_1=9
LP_SEL_2=10
LP_SEL_3=11
LP_SEL_4=12
LED=13

EXT=8
QFDB_B=9
QFDB_A=10
QFDB_C=11
QFDB_D=12

def get_pid(name):
    return int(check_output(["pidof","-s",name]))



def getGPIO():
  gpioValues = [0] * 16
## We need to get all the GPIOs
  for i in range(0, 16):
    gpioPath="/tmp/mezzanine/jtag/gpio/"+repr(i)+"/value"
    if os.path.isfile(gpioPath):
      try:
        gpiofile = open(gpioPath,"r")
        # Read the file, store the data value in an array
        line=gpiofile.readline()
        line = line.replace("\n", "")
        gpioValues[i]=int(line)
        #print("I/O "+repr(i)+" "+repr(gpioValues[i]))
        gpiofile.close()
      except:
        gpioValues[i]=-1

  return gpioValues

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
        jtagConfig = {}
        # Get the IO values,
        gpioValues=getGPIO()

        # Parse the IO values
        if gpioValues[TRANS]==1:
          jtagConfig["Transparent"] = "Yes"
        else:
          jtagConfig["Transparent"] = "No"

        if gpioValues[MASK]==1:
          jtagConfig["AddressMask"] = "Yes"
        else:
          jtagConfig["AddressMask"] = "No"

        if gpioValues[SB_Bn]==1:
          jtagConfig["Mode"] = "ScanBridge"
        else:
          jtagConfig["Mode"] = "Stitcher"

        if gpioValues[MPSEL]==1:
          jtagConfig["Master"] = "Connector"
        else:
          jtagConfig["Master"] = "Ethernet"

        if gpioValues[OEn]==1:
          jtagConfig["Output"] = "Disabled"
        else:
          jtagConfig["Output"] = "Enabled"

        if gpioValues[RSTn]==1:
          jtagConfig["Reset"] = "De-Asserted"
        else:
          jtagConfig["Reset"] = "Asserted"

        # now check for each item in chain, and build it
        chainString=""
        if gpioValues[EXT]==1:
          chainString=chainString+"Ext "

        if gpioValues[QFDB_B]==1:
          chainString=chainString+"QFDB_B "
        if gpioValues[QFDB_A]==1:
          chainString=chainString+"QFDB_A "
        if gpioValues[QFDB_C]==1:
          chainString=chainString+"QFDB_C "
        if gpioValues[QFDB_D]==1:
          chainString=chainString+"QFDB_D "
        jtagConfig["Chain"] = chainString

        info["Config"] = jtagConfig

        # Check for the xvcServer task
        try:
          server_pid=get_pid("xvcServer")
          info["Server"]="Running"
        except:
          info["Server"]="Stopped"

        return info
#
# data["action"] will be the command
# data["value"] will be the argument

    def doAction(self, data):
        res = 'failure'

        if data["action"].lower() == "stop_server":
          try:
            server_pid=get_pid("xvcServer")
            # Now kill it
            os.kill(server_pid, signal.SIGTERM) #or signal.SIGKILL
            res = 'success'
          except:
            res = 'failure' # It wasnt running..

        elif data["action"].lower() == "start_server":
          try:
            server_pid=get_pid("xvcServer")
            # Now kill it
            os.kill(server_pid, signal.SIGTERM) #or signal.SIGKILL
          except:
            pass

          # Now try to start it!
          try:
            Popen(['/usr/local/bin/xvcServer', '', '0'], close_fds=True)
          except:
            pass

        elif data["action"].lower() == "reset":
          gpioPath="/tmp/mezzanine/jtag/gpio/"+repr(RSTn)+"/direction"
          try:
            gpiofile = open(gpioPath,"w")
            gpiofile.write ("low"+"\n")
            sleep(10000/1000000) #10ms delay
            gpiofile.close()
            gpiofile = open(gpioPath,"w")
            gpiofile.write ("high"+"\n")
            res = 'success'
            gpiofile.close()
          except:
            res = 'failure'

        elif data["action"].lower() == "master":
          gpioPath="/tmp/mezzanine/jtag/gpio/"+repr(MPSEL)+"/direction"
          if data["value"].lower() == "ethernet":
            masterMode='low'
          elif data["value"].lower() == "connector":
            masterMode='high'

          try:
            gpiofile = open(gpioPath,"w")
            gpiofile.write (masterMode+"\n")
            res = 'success'
            gpiofile.close()
          except:
            res = 'failure'

        elif data["action"].lower() == "output":
          gpioPath="/tmp/mezzanine/jtag/gpio/"+repr(OEn)+"/direction"
          if data["value"].lower() == "enabled":
            oe='low'
          else:
            oe='high'

          try:
            gpiofile = open(gpioPath,"w")
            gpiofile.write (oe+"\n")
            res = 'success'
            gpiofile.close()
          except:
            res = 'failure'

        elif data["action"].lower() == "mode":
          gpioPath="/tmp/mezzanine/jtag/gpio/"+repr(SB_Bn)+"/direction"
          if data["value"].lower() == "scanbridge":
            jtagmode='low'
          elif data["value"].lower() == "stitcher":
            jtagmode='high'

          try:
            gpiofile = open(gpioPath,"w")
            gpiofile.write (jtagmode+"\n")
            res = 'success'
            gpiofile.close()
          except:
            res = 'failure'

        elif data["action"].lower() == "transparent":
          gpioPath="/tmp/mezzanine/jtag/gpio/"+repr(TRANS)+"/direction"
          if data["value"].lower() == "yes":
            transmode='high'
          elif data["value"].lower() == "no":
            transmode='low'

          try:
            gpiofile = open(gpioPath,"w")
            gpiofile.write (transmode+"\n")
            res = 'success'
            gpiofile.close()
          except:
            res = 'failure'

        elif data["action"].lower() == "addressmask":
          gpioPath="/tmp/mezzanine/jtag/gpio/"+repr(MASK)+"/direction"
          if data["value"].lower() == "yes":
            maskmode='high'
          elif data["value"].lower() == "no":
            maskmode='low'

          try:
            gpiofile = open(gpioPath,"w")
            gpiofile.write (maskmode+"\n")
            res = 'success'
            gpiofile.close()
          except:
            res = 'failure'

        elif data["action"].lower() == "chain":
          gpioValues = [0] * 16
          IOerror=False
          line=data["value"].lower()
          sites=line.split(' ')

          # Loop over all sites found.
          for site in sites:
            if site == "ext":
              gpioValues[EXT]=1
            elif site == "qfdb_b":
              gpioValues[QFDB_B]=1
            elif site == "qfdb_a":
              gpioValues[QFDB_A]=1
            elif site == "qfdb_c":
              gpioValues[QFDB_C]=1
            elif site == "qfdb_d":
              gpioValues[QFDB_D]=1

          # Now write out the sites config.
          gpioPath="/tmp/mezzanine/jtag/gpio/"+repr(EXT)+"/value"
          try:
            gpiofile = open(gpioPath,"w")
            gpiofile.write (repr(gpioValues[EXT])+"\n")
            res = 'success'
            gpiofile.close()
          except:
            IOerror=True

          gpioPath="/tmp/mezzanine/jtag/gpio/"+repr(QFDB_B)+"/value"
          try:
            gpiofile = open(gpioPath,"w")
            gpiofile.write (repr(gpioValues[QFDB_B])+"\n")
            res = 'success'
            gpiofile.close()
          except:
            IOerror=True

          gpioPath="/tmp/mezzanine/jtag/gpio/"+repr(QFDB_A)+"/value"
          try:
            gpiofile = open(gpioPath,"w")
            gpiofile.write (repr(gpioValues[QFDB_A])+"\n")
            res = 'success'
            gpiofile.close()
          except:
            IOerror=True

          gpioPath="/tmp/mezzanine/jtag/gpio/"+repr(QFDB_C)+"/value"
          try:
            gpiofile = open(gpioPath,"w")
            gpiofile.write (repr(gpioValues[QFDB_C])+"\n")
            res = 'success'
            gpiofile.close()
          except:
            IOerror=True

          gpioPath="/tmp/mezzanine/jtag/gpio/"+repr(QFDB_D)+"/value"
          try:
            gpiofile = open(gpioPath,"w")
            gpiofile.write (repr(gpioValues[QFDB_D])+"\n")
            res = 'success'
            gpiofile.close()
          except:
            IOerror=True

          # LED mirrors site 4's value'
          gpioPath="/tmp/mezzanine/jtag/gpio/"+repr(LED)+"/value"
          try:
            gpiofile = open(gpioPath,"w")
            if gpioValues[QFDB_D] == 1:
              gpiofile.write ("0\n")
            else:
              gpiofile.write ("1\n")
            res = 'success'
            gpiofile.close()
          except:
            IOerror=True

          if not IOerror:
            res='success'

        # Return result codes.
        result = { "result": res }
        return result

def get_node_jtag():
    actions =  ["reset",
                "chain",
                "master",
                "output",
                "mode",
                "transparent",
                "addressmask",
                "start_server",
                "stop_server"
                ]

    return jtagNode(actions = actions)
