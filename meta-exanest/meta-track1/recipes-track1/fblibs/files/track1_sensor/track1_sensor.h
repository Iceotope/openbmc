/*
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
 *
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

#ifndef __TRACK1_SENSOR_H__
#define __TRACK1_SENSOR_H__

#include <stdbool.h>
#include <openbmc/ipmi.h>
#include <openbmc/ipmb.h>
#include <openbmc/obmc-pal.h>
#include <facebook/track1_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SDR_LEN           64
#define MAX_SENSOR_NUM        0xFF
#define MAX_SENSOR_THRESHOLD  8
#define MAX_RETRIES_SDR_INIT  30
#define THERMAL_CONSTANT      255
#define ERR_NOT_READY         -2



#define MIN_48V_NOMINAL         48000
#define MIN_48V_LIMIT_MIN       (MIN_48V_NOMINAL-((MIN_48V_NOMINAL/100)*5))
#define MIN_48V_LIMIT_MAX       (MIN_48V_NOMINAL+((MIN_48V_NOMINAL/100)*5))


typedef struct _sensor_info_t {
  bool valid;
  sdr_full_t sdr;
} sensor_info_t;

// Sensors on a TPDB
enum {
  TPDB_SENSOR_W1_1 = 0x01,
  TPDB_SENSOR_W1_2 = 0x02,
  TPDB_SENSOR_W1_3 = 0x03,
  TPDB_SENSOR_W1_4 = 0x04,
  TPDB_SENSOR_W1_5 = 0x05,
  TPDB_SENSOR_W1_6 = 0x06,
  TPDB_SENSOR_W1_7 = 0x07,
  TPDB_SENSOR_W1_8 = 0x08
};

// Sensors on a KDB
enum {
  KDB_SENSOR_DUMMY = 0x1,
};

// Sensors on a QFDB
enum {
  QFDB_SENSOR_DUMMY = 0x1,
};

// Sensors on BMC
enum {
  BMC_SENSOR_TEMP = 0x01,
  BMC_SENSOR_V0 = 0x02,
  BMC_SENSOR_V1 = 0x03,
  BMC_SENSOR_V2 = 0x04,
  BMC_SENSOR_V3 = 0x05,
  BMC_SENSOR_V4 = 0x06,
  BMC_SENSOR_V5 = 0x07,
  BMC_SENSOR_V48 = 0x08,
  BMC_SENSOR_I48 = 0x09,
  BMC_SENSOR_P48 = 0x0A,
};

////extern float spb_sensor_threshold[MAX_SENSOR_NUM][MAX_SENSOR_THRESHOLD + 1];
////extern float nic_sensor_threshold[MAX_SENSOR_NUM][MAX_SENSOR_THRESHOLD + 1];

/******************************
 * MPK Ports to Track 1
 ******************************/

int track1_sensor_read(uint8_t fru, uint8_t sensor_num, void *value);
int track1_sensor_name(uint8_t fru, uint8_t sensor_num, char *name);
int track1_sensor_units(uint8_t fru, uint8_t sensor_num, char *units);
int track1_sensor_sdr_path(uint8_t fru, char *path);
int track1_sensor_threshold(uint8_t fru, uint8_t sensor_num, uint8_t thresh, float *value);
int track1_sensor_sdr_init(uint8_t fru, sensor_info_t *sinfo);


//extern float spb_sensor_threshold[MAX_SENSOR_NUM][MAX_SENSOR_THRESHOLD + 1];
//extern float nic_sensor_threshold[MAX_SENSOR_NUM][MAX_SENSOR_THRESHOLD + 1];

extern const uint8_t qfdb_sensor_list[];
extern const uint8_t tpdb_sensor_list[];
extern const uint8_t kdb_sensor_list[];
extern const uint8_t bmc_sensor_list[];
extern size_t qfdb_sensor_cnt;
extern size_t tpdb_sensor_cnt;
extern size_t kdb_sensor_cnt;
extern size_t bmc_sensor_cnt;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __YOSEMITE_SENSOR_H__ */
