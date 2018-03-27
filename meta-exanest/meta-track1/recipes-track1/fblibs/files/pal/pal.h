/*
 *
 * Copyright 2017-present Facebook. All Rights Reserved.
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

#ifndef __PAL_H__
#define __PAL_H__

#include <openbmc/obmc-pal.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <openbmc/kv.h>
#include <openbmc/ipmi.h>
#include <stdbool.h>

#include <facebook/track1_sensor.h>
#include <facebook/track1_common.h>

#define BIT(value, index) ((value >> index) & 1)

#define FRU_STATUS_GOOD   1
#define FRU_STATUS_BAD    0

#define KV_STORE "/mnt/data/kv_store/%s"
#define KV_STORE_PATH "/mnt/data/kv_store"

#define MAX_NODES     8

#define LAST_KEY "last_key"

#define TRACK1_PLATFORM_NAME "track1"
#define LAST_KEY "last_key"
#define TRACK1_MAX_NUM_SLOTS 12

extern const char pal_server_list[];
extern const char pal_fru_list[];

enum {
  SERVER_POWER_OFF,
  SERVER_POWER_ON,
  SERVER_POWER_CYCLE,
  SERVER_POWER_RESET,
  SERVER_GRACEFUL_SHUTDOWN,
  SERVER_12V_OFF,
  SERVER_12V_ON,
  SERVER_12V_CYCLE,
};


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __PAL_H__ */
