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
bool pal_is_fru_x86(uint8_t fru);
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
int pal_get_fru_list(char *list);
int pal_get_fru_id(char *str, uint8_t *fru);
int pal_get_fru_name(uint8_t fru, char *name);
int pal_get_fru_sdr_path(uint8_t fru, char *path);
int pal_get_fru_sensor_list(uint8_t fru, uint8_t **sensor_list, int *cnt);
int pal_fruid_write(uint8_t fru, char *path);
int pal_sensor_sdr_init(uint8_t fru, sensor_info_t *sinfo);
int pal_sensor_read_raw(uint8_t fru, uint8_t sensor_num, void *value);
int pal_sensor_threshold_flag(uint8_t fru, uint8_t snr_num, uint16_t *flag);
int pal_get_sensor_threshold(uint8_t fru, uint8_t sensor_num,
  uint8_t thresh, void *value);
int pal_get_sensor_name(uint8_t fru, uint8_t sensor_num, char *name);
int pal_get_sensor_units(uint8_t fru, uint8_t sensor_num, char *units);
int pal_get_fruid_path(uint8_t fru, char *path);
int pal_get_fruid_eeprom_path(uint8_t fru, char *path) ;
int pal_get_fruid_name(uint8_t fru, char *name);
int pal_set_def_key_value(void);
int pal_get_fru_devtty(uint8_t fru, char *devtty);
int pal_get_fru_devtty2(uint8_t fru, char *devtty);
void pal_dump_key_value(void);
int pal_set_last_pwr_state(uint8_t fru, char *state);
int pal_get_last_pwr_state(uint8_t fru, char *state);
int pal_get_sys_guid(uint8_t slot, char *guid);
int pal_set_sysfw_ver(uint8_t slot, uint8_t *ver);
int pal_get_sysfw_ver(uint8_t slot, uint8_t *ver);
int pal_is_bmc_por(void);
int pal_get_fru_discrete_list(uint8_t fru, uint8_t **sensor_list, int *cnt);
int pal_sensor_discrete_check(uint8_t fru, uint8_t snr_num, char *snr_name,
    uint8_t o_val, uint8_t n_val)
int pal_set_sensor_health(uint8_t fru, uint8_t value);
int pal_get_fru_health(uint8_t fru, uint8_t *value);
void pal_update_ts_sled(void);
void pal_log_clear(char *fru);
int pal_is_crashdump_ongoing(uint8_t slot);
int pal_get_board_rev_id(uint8_t *id);
int pal_get_mb_slot_id(uint8_t *id);
int pal_get_slot_cfg_id(uint8_t *id);
void pal_get_chassis_status(uint8_t slot, uint8_t *req_data,
  uint8_t *res_data, uint8_t *res_len);
uint8_t pal_set_power_restore_policy(uint8_t slot, uint8_t *pwr_policy,
  uint8_t *res_data);
int pal_get_platform_id(uint8_t *id);
int pal_get_fw_info(unsigned char target, unsigned char* res,
  unsigned char* res_len);
int pal_get_plat_sku_id(void);
int pal_get_restart_cause(uint8_t slot, uint8_t *restart_cause);
int pal_set_restart_cause(uint8_t slot, uint8_t restart_cause);
void pal_get_me_name(uint8_t fru, char *target_name);
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __PAL_H__ */
