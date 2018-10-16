#!/usr/bin/env python
#
#   EuroAXA Mezzanine Firmware tool
#   Copyright (C) 2019-present  Iceotope Ltd.
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

## EuroExa FW/eeprom setup util for track1
#
# Sets the following info
#
# MAC
# Serial
# Board Rev
# Board type
# Site information
# Format in EPROM, taken from C code.
# /*
#  * EEPROM data format
#  */

# typedef struct _eepromNetworkConfig
# {
#         u8      networkIP[4];
#         u8      networkMask[4];
#         u8      networkGW[4];
#         u8      networkMAC[6];
#         u8      networkDHCP;
# } __attribute__((packed)) boardNetwork_t ;

# typedef struct _boardData
# {
#         u32     boardType;              //4
#         u16     boardRev;               //2
#         u16     boardMagic;             //2
#         u32     boardSerial;            //4
#         boardNetwork_t boardNetConfig;  //19
#         u8      boardPAD[1];            // 1
#         DB_SITE_TYPE_t siteDefaults[SITE_MAX_COUNT]; // 16 sites
#         u32     CRC
# } __attribute__((packed)) boardData_t; //Size = 48 Bytes, so
#                                       //6 frames of flash long
# #define BOARD_REV_DEBUGGING (1<<15)
# #define BOARD_CONFIG_EEPROM_OFFSET 0x0
# #define BOARD_CONFIG_MAGIC_VALUE 0xFEED
# // MPK new type definition, easier to expand.
# typedef enum _BOARD_TYPE {
#        MEZZ_TRACK_1 = 0x54524b31, // TRK1 in ASCII
#        MEZZ_TRACK_2 = 0x54524b32  // TRK2 in ASCII
# } BOARD_TYPE_t;


import argparse
import re
import struct

eeprom_fmt = '=L H H L 4B 4B 4B 6B B B 8B L' # int[5], float, byte[255]
eeprom_len = struct.calcsize(eeprom_fmt)
eeprom_unpack = struct.Struct(eeprom_fmt).unpack_from
eeprom_pack = struct.Struct(eeprom_fmt).pack

## Functions

# Print only if we're verbose
def verbose_print(string_to_print):
  if args.verbose:
    print(string_to_print)

# Get slot ID
## Open the file and read it out, 0-15 is the value, return 255 on error
def get_slotid():
  try:
    slot_fd = open("/tmp/mezzanine/SLOT_ID", "r")
    slot_string = slot_fd.readline()
    return(int(slot_string))
  except:
    return(255)

## Main
allowed_mac = re.compile(r"""
                         (
                            ^([0-9A-F]{2}[:]){5}([0-9A-F]{2})$
                         )
                         """,
                         re.VERBOSE|re.IGNORECASE)

parser = argparse.ArgumentParser(description="EuroExa FW/eeprom setup util for track1")
parser.add_argument("-v", "--verbose", help="Be Verbose",
                    action="store_true")
parser.add_argument("-w", "--write", help="Write to EEPROM",
                    action="store_true")

parser.add_argument("-d,", "--defaults", help="Set site defaults",
                    action="store_true")
parser.add_argument("-m,", "--mac", help="MAC Address")
parser.add_argument("-s,", "--serial", type=int, help="Serial Number")
parser.add_argument("-r,", "--revision", type=int, help="Revision")
#parser.add_argument("-t,", "--type", type=int, help="Board Type") # Fixed as track1 here.

args = parser.parse_args()
verbose_print("verbosity turned on")
if args.mac:
  if re.match(allowed_mac, args.mac):
    verbose_print("MAC : " + args.mac)
    mac_array = args.mac.split(":")
    #convert it to numeric
    for i in range(0,len(mac_array)):
      mac_array[i] = int(mac_array[i],16)
      verbose_print("MAC[%d] : 0x%x" % (i,mac_array[i]))
  else:
    print("Badly formatted MAC address : " + args.mac)
    exit(10)

if args.serial:
  verbose_print("Serial : " + repr(args.serial))

if args.revision:
  verbose_print("Revision : " + repr(args.revision))

if args.defaults:
  verbose_print("Site Defaults set on")

## Read in existing eeprom.
# /tmp/mezzanine/eeprom is the file
EEPROM_FILE="/tmp/mezzanine/eeprom"

results = []

f = open(EEPROM_FILE, "rb")
if f:
    eeprom_data = f.read(eeprom_len)
    if eeprom_data:
      results = eeprom_unpack(eeprom_data)
    f.close()
else:
    print("Unable to open EEPROM file : " + EEPROM_FILE)
    exit(10)

# Print out existing EEPROM?
verbose_print("\nExisting EEPROM")
verbose_print("---------------")
if results[2] == 0xFEED:
  verbose_print("EEPROM is GOOD")
else:
  verbose_print("EEPROM is BAD")
verbose_print("Type     0x%08X (%c%c%c%c)\nSerial   %d\nRevision %d" % (results[0], (results[0]>>24 & 0xff), (results[0]>>16 & 0xff), \
          (results[0]>>8 & 0xff), (results[0] & 0xff), results[3], results[1]))
verbose_print("MAC      %02X:%02X:%02X:%02X:%02X:%02X" % (results[16] ,results[17],results[18],results[19],results[20],results[21] ))
for i in range(24,32):
  verbose_print("Site %d : 0x%02X" % (i-24+1, results[i]))
# convert from tuple, so we can modify things
tempdata = list(results)

# Revision
if args.revision:
  tempdata[1] = args.revision

# Serial
if args.serial:
  tempdata[3] = args.serial

# Type
tempdata[0] = 0x54524b31
# Magic
tempdata[2] = 0xFEED

# MAC
# Fix top byte to have local admin set, and unicast
if args.mac:
  tempdata[16] = (mac_array[0] | 0x02) & 0xFE

  tempdata[17] = mac_array[1]
  tempdata[18] = mac_array[2]
  tempdata[19] = mac_array[3]
  tempdata[20] = mac_array[4]
  # Fix this, we need the Slot ID
  slot_address = get_slotid()
  if slot_address < 16:
    tempdata[21] = slot_address <<4
  else:
    print("Bad SLOT %d" % (slot_address))
    tempdata[21] = 0xFF

# Default settings for sites.
if args.defaults:
  tempdata[24] = 255
  tempdata[25] = 255
  tempdata[26] = 0
  tempdata[27] = 0
  tempdata[28] = 1
  tempdata[29] = 1
  tempdata[30] = 1
  tempdata[31] = 1

# Calc the CRC of the whole thing?

# Convert back to tuple
results = tuple(tempdata)

verbose_print("\nNew EEPROM")
verbose_print("---------------")
verbose_print("Type     0x%08X (%c%c%c%c)\nSerial   %d\nRevision %d" % (results[0], (results[0]>>24 & 0xff), (results[0]>>16 & 0xff), \
          (results[0]>>8 & 0xff), (results[0] & 0xff), results[3], results[1]))
verbose_print("MAC      %02X:%02X:%02X:%02X:%02X:%02X" % (results[16] ,results[17],results[18],results[19],results[20],results[21] ))
for i in range(24,32):
  verbose_print("Site %d : 0x%02X" % (i-24+1, results[i]))

eeprom_data=eeprom_pack(*results)

# Write it out to file?
if args.write:
  try:
    verbose_print("Writing EEPROM")
    f = open(EEPROM_FILE, "wb")
    f.write(eeprom_data)
    f.close()
  except (IOError, OSError):
    print("Unable to open EEPROM file (For write) : " + EEPROM_FILE)
    exit(10)

verbose_print("Done..")
