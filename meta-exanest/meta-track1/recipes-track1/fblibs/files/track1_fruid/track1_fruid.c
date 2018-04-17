/* Copyright 2015-present Facebook. All Rights Reserved.
 * Copyright 2018-present Iceotope. All Rights Reserved.
 *
 * This file contains code to support IPMI2.0 Specificaton available @
 * http://www.intel.com/content/www/us/en/servers/ipmi/ipmi-specifications.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <syslog.h>
#include "track1_fruid.h"

/* Populate char path[] with the path to the fru's fruid binary dump */
int
track1_get_fruid_path(uint8_t fru, char *path) {
  char fname[16] = {0};

  switch(fru) {
    case FRU_TPDB_B:
      sprintf(fname, "slot1");
      break;
    case FRU_TPDB_A:
      sprintf(fname, "slot2");
      break;
    case FRU_KDB_B:
      sprintf(fname, "slot3");
      break;
    case FRU_KDB_A:
      sprintf(fname, "slot4");
      break;
    case FRU_QFDB_C:
      sprintf(fname, "slot5");
      break;
    case FRU_QFDB_D:
      sprintf(fname, "slot6");
      break;
    case FRU_QFDB_B:
      sprintf(fname, "slot7");
      break;
    case FRU_QFDB_A:
      sprintf(fname, "slot8");
      break;
    case FRU_BMC:
      sprintf(fname, "bmc");
      break;
    default:
#ifdef DEBUG
      syslog(LOG_WARNING, "track1_get_fruid_path: wrong fruid");
#endif
      return -1;
  }

  sprintf(path, TRACK1_FRU_PATH, fname);
  return 0;
}

int
track1_get_fruid_eeprom_path(uint8_t fru, char *path) {
  char fname[16] = {0};

  switch(fru) {
    case FRU_TPDB_B:
    case FRU_TPDB_A:
    case FRU_KDB_A:
    case FRU_KDB_B:
    case FRU_QFDB_A:
    case FRU_QFDB_B:
    case FRU_QFDB_C:
    case FRU_QFDB_D:
      sprintf(path, "/tmp/mezzanine/db_%d/eeprom",fru);
      break;

    case FRU_BMC:
      sprintf(path, "/tmp/mezzanine/eeprom");
      break;
    default:
#ifdef DEBUG
      syslog(LOG_WARNING, "track1_get_fruid_eeprom_path: wrong fruid");
#endif
      return -1;
  }

  return 0;
}

/* Populate char name[] with the path to the fru's name */
int
track1_get_fruid_name(uint8_t fru, char *name) {

  switch(fru) {
    case FRU_TPDB_B:
      sprintf(name, "Thermal Proxy B (slot1)");
      break;

    case FRU_TPDB_A:
      sprintf(name, "Thermal Proxy A (slot2)");
      break;

    case FRU_KDB_A:
      sprintf(name, "KDB A (slot4)");
      break;

    case FRU_KDB_B:
      sprintf(name, "KDB B (slot3)");
      break;

    case FRU_QFDB_C:
      sprintf(name, "Quad FPGA C (slot5)");
      break;

    case FRU_QFDB_D:
      sprintf(name, "Quad FPGA D (slot6)");
      break;

    case FRU_QFDB_B:
      sprintf(name, "Quad FPGA B (slot7)");
      break;

    case FRU_QFDB_A:
      sprintf(name, "Quad FPGA A (slot8)");
      break;


    case FRU_BMC:
      sprintf(name, "MicroZynq BMC");
      break;
    default:
#ifdef DEBUG
      syslog(LOG_WARNING, "track1_get_fruid_name: wrong fruid");
#endif
      return -1;
  }
  return 0;
}
