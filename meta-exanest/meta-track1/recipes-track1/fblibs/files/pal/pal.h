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
  SERVER_GLOBAL_RESET,
};


int pal_get_platform_name(char *name);
int pal_get_num_slots(uint8_t *num);
int pal_is_slot_server(uint8_t fru);
int pal_is_server_12v_on(uint8_t slot_id, uint8_t *status);
int pal_get_server_power(uint8_t slot_id, uint8_t *status);
int pal_set_server_power(uint8_t slot_id, uint8_t cmd);
int pal_is_debug_card_prsnt(uint8_t *status);
int pal_is_fru_ready(uint8_t fru, uint8_t *status);
int pal_is_fru_prsnt(uint8_t fru, uint8_t *status);
int pal_set_hb_led(uint8_t status);
int pal_set_rst_btn(uint8_t slot, uint8_t status);
int pal_get_rst_btn(uint8_t *status);
int pal_get_pwr_btn(uint8_t *status);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __PAL_H__ */
