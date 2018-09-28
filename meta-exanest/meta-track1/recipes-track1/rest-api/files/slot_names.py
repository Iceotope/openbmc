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
from tree import tree
from pal import *

slot_names = ("all" ,"tpdb-b", "tpdb-a", "kdb-a", "kdb-b", "qfdb-c", "qfdb-d",  "qfdb-b", "qfdb-a", "bmc")
site_names = ("all" ,"site1", "site2", "site3", "site4", "site5", "site6",  "qfdb-b", "qfdb-a", "bmc")

slot_numbers = {'all': 0,
  'tpdb-b': 1, 'tpdb_b': 1,
  'tpdb-a': 2, 'tpdb_a': 2,
  'kdb-a': 3, 'kdb_a': 3,
  'kdb-b': 4, 'kdb_b': 4,
  'qfdb-c': 5, 'qfdb_c': 5,
  'qfdb-d': 6, 'qfdb_d': 6,
  'qfdb-b': 7, 'qfdb_b': 7,
  'qfdb-a': 8, 'qfdb_a': 8,
  'bmc': 9 }

def get_slot_name_from_number(num):
    return slot_names[num]


def get_site_name_from_number(num):
    return site_names[num]

def get_site_number_from_name(num):
    return slot_numbers[num]

