/*
 *
 * Copyright 2018-present Iceotope. All Rights Reserved.
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

#ifndef __TRACK1_COMMON_H__
#define __TRACK1_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NUM_FRUS 9
enum {
  FRU_ALL    = 0,
  FRU_TPDB_B = 1,
  FRU_TPDB_A = 2,
  FRU_KDB_B  = 3,
  FRU_KDB_A  = 4,
  FRU_QFDB_C = 5,
  FRU_QFDB_D = 6,
  FRU_QFDB_B = 7,
  FRU_QFDB_A = 8,
  FRU_BMC    = 9,
};

// MPK new type definition, easier to expand.
typedef enum _DB_SITE_TYPE {
  DB_TYPE_NONE = 0,
  DB_TYPE_QFDB,
  DB_TYPE_KDB,
  DB_TYPE_TPDB,
  DB_TYPE_COUNT,
  DB_TYPE_BMC = 0xFE,
  DB_TYPE_AUTO = 0xFF
} DB_SITE_TYPE_t;


#define CRASHDUMP_KEY "slot%d_crashdump"

int track1_common_fru_name(uint8_t fru, char *str);
int track1_common_fru_id(char *str, uint8_t *fru);
int track1_common_crashdump(uint8_t fru);
int track1_get_fru_type(uint8_t fru, uint8_t *type);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __track1_COMMON_H__ */
