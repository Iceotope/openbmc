/*
 *
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
#include <stdbool.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <openbmc/edb.h>
#include "track1_common.h"

#define CRASHDUMP_BIN       "/usr/local/bin/dump.sh"
#define CRASHDUMP_FILE      "/mnt/data/crashdump_"
#define DB_TYPE_FILE        "/tmp/mezzanine/db_%d/type"

struct threadinfo {
  uint8_t is_running;
  uint8_t fru;
  pthread_t pt;
};

static int
read_device(const char *device, int *value) {
  FILE *fp;
  int rc;

  fp = fopen(device, "r");
  if (!fp) {
    int err = errno;
#ifdef DEBUG
    syslog(LOG_INFO, "failed to open device %s", device);
#endif
    return err;
  }

  rc = fscanf(fp, "%d", value);
  fclose(fp);
  if (rc != 1) {
#ifdef DEBUG
    syslog(LOG_INFO, "failed to read device %s", device);
#endif
    return ENOENT;
  } else {
    return 0;
  }
}


static struct threadinfo t_dump[MAX_NUM_FRUS] = {0, };

// Return installed type for DB, NONE if not installed.
int
track1_get_fru_type(uint8_t fru, uint8_t *type) {

  int val;
  char vpath[MAX_VALUE_LEN] = {0};
  // Default
  *type = DB_TYPE_NONE;

  switch (fru) {
    case FRU_TPDB_B:
    case FRU_TPDB_A:
    case FRU_KDB_B:
    case FRU_KDB_A:
    case FRU_QFDB_D:
    case FRU_QFDB_C:
    case FRU_QFDB_B:
    case FRU_QFDB_A:
      // Create a string to the file
      sprintf(vpath, DB_TYPE_FILE, fru-1);

      if (read_device(vpath, &val)) {
        return -1;
      }
      // Is it present, and less than the count?
      if ( val < DB_TYPE_COUNT) {
        *type = val;
      }
      break;
    case FRU_BMC:
      *type = DB_TYPE_BMC;
      break;
    default:
      return -1;
  }

  return 0;
}

int
track1_common_fru_name(uint8_t fru, char *str) {

  switch(fru) {
    case FRU_TPDB_B:
      sprintf(str, "tpdb-b");
      break;

    case FRU_TPDB_A:
      sprintf(str, "tpdb-a");
      break;

    case FRU_KDB_B:
      sprintf(str, "kdb-b");
      break;

    case FRU_KDB_A:
      sprintf(str, "kdb-a");
      break;

    case FRU_QFDB_D:
      sprintf(str, "qfdb-d");
      break;

  case FRU_QFDB_C:
      sprintf(str, "qfdb-c");
      break;

  case FRU_QFDB_B:
      sprintf(str, "qfdb-b");
      break;

  case FRU_QFDB_A:
      sprintf(str, "qfdb-a");
      break;

    case FRU_BMC:
      sprintf(str, "bmc");
      break;

    default:
#ifdef DEBUG
      syslog(LOG_WARNING, "track1_common_fru_name: Wrong fru id");
#endif
    return -1;
  }

  return 0;
}

int
track1_common_fru_id(char *str, uint8_t *fru) {

  if (!strcmp(str, "all")) {
    *fru = FRU_ALL;
  } else if (!strcmp(str, "tpdb-b")) {
    *fru = FRU_TPDB_B;
  } else if (!strcmp(str, "tpdb-a")) {
    *fru = FRU_TPDB_A;
  } else if (!strcmp(str, "kdb-b")) {
    *fru = FRU_KDB_B;
  } else if (!strcmp(str, "kdb-a")) {
    *fru = FRU_KDB_A;
  } else if (!strcmp(str, "qfdb-d")) {
    *fru = FRU_QFDB_D;
  } else if (!strcmp(str, "qfdb-c")) {
    *fru = FRU_QFDB_C;
  } else if (!strcmp(str, "qfdb-b")) {
    *fru = FRU_QFDB_B;
  } else if (!strcmp(str, "qfdb-a")) {
    *fru = FRU_QFDB_A;
  } else if (!strcmp(str, "bmc")) {
    *fru = FRU_BMC;
  } else {
#ifdef DEBUG
    syslog(LOG_WARNING, "track1_common_fru_id: Wrong fru id");
#endif
    return -1;
  }

  return 0;
}

void *
generate_dump(void *arg) {

  uint8_t fru = *(uint8_t *) arg;
  char cmd[128];
  char fruname[16];

  // Usually the pthread cancel state are enable by default but
  // here we explicitly would like to enable them
  (void) pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  (void) pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  track1_common_fru_name(fru, fruname);

  // HEADER LINE for the dump
  memset(cmd, 0, 128);
  sprintf(cmd, "%s time > %s%s", CRASHDUMP_BIN, CRASHDUMP_FILE, fruname);
  system(cmd);

  // COREID dump
  memset(cmd, 0, 128);
  sprintf(cmd, "%s %s coreid >> %s%s", CRASHDUMP_BIN, fruname, CRASHDUMP_FILE, fruname);
  system(cmd);

  // MSR dump
  memset(cmd, 0, 128);
  sprintf(cmd, "%s %s msr >> %s%s", CRASHDUMP_BIN, fruname, CRASHDUMP_FILE, fruname);
  system(cmd);

  syslog(LOG_CRIT, "Crashdump for FRU: %d is generated.", fru);

  t_dump[fru-1].is_running = 0;

  sprintf(cmd, CRASHDUMP_KEY, fru);
  edb_cache_set(cmd, "0");

  return NULL;
}


int
track1_common_crashdump(uint8_t fru) {

  int ret;
  char cmd[100];

  // Check if the crashdump script exist
  if (access(CRASHDUMP_BIN, F_OK) == -1) {
    syslog(LOG_CRIT, "Crashdump for FRU: %d failed : "
        "crashdump binary is not preset", fru);
    return 0;
  }

  // Check if a crashdump for that fru is already running.
  // If yes, kill that thread and start a new one.
  if (t_dump[fru-1].is_running) {
    ret = pthread_cancel(t_dump[fru-1].pt);
    if (ret == ESRCH) {
      syslog(LOG_INFO, "track1_common_crashdump: No Crashdump pthread exists");
    } else {
      pthread_join(t_dump[fru-1].pt, NULL);
      // MPK: Maybe slot%d needs changing here? to the name of the slot, not sure
      // if the crash dump will use that?
      sprintf(cmd, "ps | grep '{dump.sh}' | grep 'slot%d' | awk '{print $1}'| xargs kill", fru);
      system(cmd);
      sprintf(cmd, "ps | grep 'me-util' | grep 'slot%d' | awk '{print $1}'| xargs kill", fru);
      system(cmd);
#ifdef DEBUG
      syslog(LOG_INFO, "track1_common_crashdump: Previous crashdump thread is cancelled");
#endif
    }
  }

  // Start a thread to generate the crashdump
  t_dump[fru-1].fru = fru;
  if (pthread_create(&(t_dump[fru-1].pt), NULL, generate_dump, (void*) &t_dump[fru-1].fru) < 0) {
    syslog(LOG_WARNING, "pal_store_crashdump: pthread_create for"
        " FRU %d failed\n", fru);
    return -1;
  }

  t_dump[fru-1].is_running = 1;

  syslog(LOG_INFO, "Crashdump for FRU: %d is being generated.", fru);

  return 0;
}
