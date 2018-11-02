/*
 * Copyright 2015-present Facebook. All Rights Reserved.
 * Copyright 2017-present Iceotope. All Rights Reserved.
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

 /*
  * This file contains functions and logics that depends on Track1 specific
  * hardware and kernel drivers. Here, some of the empty "default" functions
  * are overridden with simple functions that returns non-zero error code.
  * This is for preventing any potential escape of failures through the
  * default functions that will return PAL_EOK no matter what.
  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>

#include <openbmc/obmc-sensor.h>
#include <openbmc/kv.h>
#include "pal.h"

#define SITE_GPIO_VAL "/tmp/mezzanine/site_%d/gpio/IO/%d/value"
#define SITE_GPIO_DIR "/tmp/mezzanine/site_%d/gpio/IO/%d/direction"
#define GPIO_PS_ON_VAL "/tmp/mezzanine/gpio/PS_ON/value"
#define HB_LED "/sys/class/leds/led-ds23/brightness"

#define BMC_LED_FILE "/tmp/mezzanine/gpio/RST_CTLREG/15/direction"

#define VPATH_SIZE  128
#define DELAY_GRACEFUL_SHUTDOWN 10
#define DELAY_POWER_OFF 1
#define DELAY_POWER_CYCLE 10
#define DELAY_48V_CYCLE 5
#define LARGEST_DEVICE_NAME 120
#define CRASHDUMP_BIN       "/usr/local/bin/dump.sh"
#define CRASHDUMP_FILE      "/mnt/data/crashdump_"

#define MAX_READ_RETRY 10
#define MAX_CHECK_RETRY 2

#define CRASHDUMP_KEY      "slot%d_crashdump"

#define LED_ON_TXT "0"
#define LED_OFF_TXT "1"
#define GPIO_HIGH "1"
#define GPIO_LOW "0"

// GPIO Expander bit fields for the Site
// Accessed via
#define SITE_GPIO_BIT_PWR_UP_LED 31
#define SITE_GPIO_BIT_PWR_DWN_LED 30
#define SITE_GPIO_BIT_PWR_DATA_LED 29
#define SITE_GPIO_BIT_UNUSED_1 28
#define SITE_GPIO_BIT_UNUSED_2 27
#define SITE_GPIO_BIT_UNUSED_3 26
#define SITE_GPIO_BIT_UNUSED_4 25
#define SITE_GPIO_BIT_PHY_RESET 24
#define SITE_GPIO_BIT_PWR_UP 23
#define SITE_GPIO_BIT_SRESET 22
#define SITE_GPIO_BIT_FS_RESET 21
#define SITE_GPIO_BIT_FN_RESET 20
#define SITE_GPIO_BIT_IRQ3 19
#define SITE_GPIO_BIT_IRQ2 18
#define SITE_GPIO_BIT_IRQ1 17
#define SITE_GPIO_BIT_IRQ0 16

#define SITE_GPIO_BIT_STAT_EX0 15
#define SITE_GPIO_BIT_STAT_EX1 14
#define SITE_GPIO_BIT_ALARM 13
#define SITE_GPIO_BIT_POSITION0 12
#define SITE_GPIO_BIT_POSITION1 11
#define SITE_GPIO_BIT_UNUSED_6 10
#define SITE_GPIO_BIT_UNUSED_7 9
#define SITE_GPIO_BIT_UNUSED_8 8
#define SITE_GPIO_BIT_UNUSED_9 7
#define SITE_GPIO_BIT_BOOT_SEQ2 6
#define SITE_GPIO_BIT_BOOT_SEQ1 5
#define SITE_GPIO_BIT_BOOT_SEQ0 4
#define SITE_GPIO_BIT_STAT_FS 3
#define SITE_GPIO_BIT_STAT_FN 2
#define SITE_GPIO_BIT_STAT_ALARM 1
#define SITE_GPIO_BIT_STAT_ON_OFF 0


const char pal_server_list[] = "tpdb-b, tpdb-a, kdb-b, kdb-a," \
          " qfdb-c, qfdb-d,  qfdb-b, qfdb-a";

const char pal_fru_list[] = "all, tpdb-b, tpdb-a, kdb-b, kdb-a," \
          " qfdb-c, qfdb-d, qfdb-b, qfdb-a, bmc";


/****
const char pal_server_list[] = "slot1, slot2, slot3, slot4," \
          " slot5, slot6, slot7, slot8";

const char pal_fru_list[] = "all, slot1, slot2, slot3, slot4," \
          " slot5, slot6, slot7, slot8, bmc";
*/

char *key_list[] = {
"identify_board",
"identify_slot1",
"identify_slot2",
"identify_slot3",
"identify_slot4",
"identify_slot5",
"identify_slot6",
"identify_slot7",
"identify_slot8",
"slot1_sel_error",
"slot2_sel_error",
"slot3_sel_error",
"slot4_sel_error",
"slot5_sel_error",
"slot6_sel_error",
"slot7_sel_error",
"slot8_sel_error",
"slot1_sensor_health",
"slot2_sensor_health",
"slot3_sensor_health",
"slot4_sensor_health",
"slot5_sensor_health",
"slot6_sensor_health",
"slot7_sensor_health",
"slot8_sensor_health",
"bmc_sensor_health",
"slot1_last_state",
"slot2_last_state",
"slot3_last_state",
"slot4_last_state",
"slot5_last_state",
"slot6_last_state",
"slot7_last_state",
"slot8_last_state",
"slot1_restart_cause",
"slot2_restart_cause",
"slot3_restart_cause",
"slot4_restart_cause",
"slot5_restart_cause",
"slot6_restart_cause",
"slot7_restart_cause",
"slot8_restart_cause",
"sysfw_ver_slot1",
"sysfw_ver_slot2",
"sysfw_ver_slot3",
"sysfw_ver_slot4",
"sysfw_ver_slot5",
"sysfw_ver_slot6",
"sysfw_ver_slot7",
"sysfw_ver_slot8",
"slot1_boot_order",
"slot2_boot_order",
"slot3_boot_order",
"slot4_boot_order",
"slot5_boot_order",
"slot6_boot_order",
"slot7_boot_order",
"slot8_boot_order",
"slot1_por_cfg",
"slot2_por_cfg",
"slot3_por_cfg",
"slot4_por_cfg",
"slot5_por_cfg",
"slot6_por_cfg",
"slot7_por_cfg",
"slot8_por_cfg",
"server_48v_status",
"timestamp_sled",
/* Add more Keys here */
LAST_KEY /* This is the last key of the list */
};

char * def_val_list[] = {
  "off", /* "identify_board" */
  "off", /* "identify_slot1" */
  "off", /* "identify_slot2" */
  "off", /* "identify_slot3" */
  "off", /* "identify_slot4" */
  "off", /* "identify_slot5" */
  "off", /* "identify_slot6" */
  "off", /* "identify_slot7" */
  "off", /* "identify_slot8" */
  "1", /* "slot1_sel_error" */
  "1", /* "slot2_sel_error" */
  "1", /* "slot3_sel_error" */
  "1", /* "slot4_sel_error" */
  "1", /* "slot5_sel_error" */
  "1", /* "slot6_sel_error" */
  "1", /* "slot7_sel_error" */
  "1", /* "slot8_sel_error" */
  "1", /* "slot1_sensor_health" */
  "1", /* "slot2_sensor_health" */
  "1", /* "slot3_sensor_health" */
  "1", /* "slot4_sensor_health" */
  "1", /* "slot5_sensor_health" */
  "1", /* "slot6_sensor_health" */
  "1", /* "slot7_sensor_health" */
  "1", /* "slot8_sensor_health" */
  "1", /* "bmc_sensor_health" */
  "off", /* "slot1_last_state" */
  "off", /* "slot2_last_state" */
  "off", /* "slot3_last_state" */
  "off", /* "slot4_last_state" */
  "off", /* "slot5_last_state" */
  "off", /* "slot6_last_state" */
  "off", /* "slot7_last_state" */
  "off", /* "slot8_last_state" */
  "0", /* "slot1_restart_cause" */
  "0", /* "slot2_restart_cause" */
  "0", /* "slot3_restart_cause" */
  "0", /* "slot4_restart_cause" */
  "0", /* "slot5_restart_cause" */
  "0", /* "slot6_restart_cause" */
  "0", /* "slot7_restart_cause" */
  "0", /* "slot8_restart_cause" */
  "0", /* "sysfw_ver_slot1" */
  "0", /* "sysfw_ver_slot2" */
  "0", /* "sysfw_ver_slot3" */
  "0", /* "sysfw_ver_slot4" */
  "0", /* "sysfw_ver_slot5" */
  "0", /* "sysfw_ver_slot6" */
  "0", /* "sysfw_ver_slot7" */
  "0", /* "sysfw_ver_slot8" */
  "0", /* "slot1_boot_order" */
  "0", /* "slot2_boot_order" */
  "0", /* "slot3_boot_order" */
  "0", /* "slot4_boot_order" */
  "0", /* "slot5_boot_order" */
  "0", /* "slot6_boot_order" */
  "0", /* "slot7_boot_order" */
  "0", /* "slot8_boot_order" */
  "off", /* "slot1_por_cfg" */
  "off", /* "slot2_por_cfg" */
  "off", /* "slot3_por_cfg" */
  "off", /* "slot4_por_cfg" */
  "off", /* "slot5_por_cfg" */
  "off", /* "slot6_por_cfg" */
  "off", /* "slot7_por_cfg" */
  "off", /* "slot8_por_cfg" */
  "0", /* "server_48v_status" */
  "0", /* timestamp_sled" */
/* Add more Keys here */
LAST_KEY /* This is the last key of the list */
};

typedef struct {
  uint16_t flag;
  float ucr;
  float unc;
  float unr;
  float lcr;
  float lnc;
  float lnr;

} _sensor_thresh_t;

typedef struct {
  uint16_t flag;
  float ucr;
  float lcr;
  uint8_t retry_cnt;
  uint8_t val_valid;
  float last_val;

} sensor_check_t;

static sensor_check_t m_snr_chk[MAX_NUM_FRUS+1][MAX_SENSOR_NUM] = {0};


// Helper functions

/* Adjust power value */
//static void
//power_value_adjust(float *value)
//{
    ////~ float x0, x1, y0, y1
    //float x;
    ////~ int i;
    //x = *value;
    ////~ x0 = power_table[0].ein;
    ////~ y0 = power_table[0].coeff;
    ////~ if (x0 > *value) {
      ////~ *value = x * y0;
      ////~ return;
    ////~ }
    ////~ for (i = 0; power_table[i].ein > 0.0; i++) {
       ////~ if (*value < power_table[i].ein)
         ////~ break;
      ////~ x0 = power_table[i].ein;
      ////~ y0 = power_table[i].coeff;
    ////~ }
    ////~ if (power_table[i].ein <= 0.0) {
      ////~ *value = x * y0;
      ////~ return;
    ////~ }
   ////~ //if value is bwtween x0 and x1, use linear interpolation method.
   ////~ x1 = power_table[i].ein;
   ////~ y1 = power_table[i].coeff;
   ////~ *value = (y0 + (((y1 - y0)/(x1 - x0)) * (x - x0))) * x;
   //*value = x;
   //return;
//}

// Helper Functions

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
    return PAL_EOK;
  }
}

static int
write_device(const char *device, const char *value) {
  FILE *fp;
  int rc;

  fp = fopen(device, "w");
  if (!fp) {
    int err = errno;
#ifdef DEBUG
    syslog(LOG_INFO, "failed to open device for write %s", device);
#endif
    return err;
  }

  rc = fputs(value, fp);
  fclose(fp);

  if (rc < 0) {
#ifdef DEBUG
    syslog(LOG_INFO, "failed to write device %s", device);
#endif
    return ENOENT;
  } else {
    return PAL_EOK;
  }
}

static int
pal_key_check(char *key) {
  int i;

  i = 0;
  while(strcmp(key_list[i], LAST_KEY)) {

    // If Key is valid, return success
    if (!strcmp(key, key_list[i]))
      return PAL_EOK;

    i++;
  }

#ifdef DEBUG
  syslog(LOG_WARNING, "pal_key_check: invalid key - %s", key);
#endif
  return -1;
}

int
pal_get_key_value(char *key, char *value) {

  // Check is key is defined and valid
  if (pal_key_check(key))
    return -1;

  return kv_get(key, value, NULL, KV_FPERSIST);
}
int
pal_set_key_value(char *key, char *value) {

  // Check is key is defined and valid
  if (pal_key_check(key))
    return -1;

  return kv_set(key, value, 0, KV_FPERSIST);
}

// Power On the server in a given slot
static int
server_power_on(uint8_t slot_id) {
  char vpath[VPATH_SIZE] = {0};

  if (slot_id < 1 || slot_id > TRACK1_MAX_NUM_SLOTS) {
    return -1;
  }

  // Set the resets, to all on following the tcl we got sent.

  sprintf(vpath, SITE_GPIO_VAL, slot_id, SITE_GPIO_BIT_FN_RESET);
  if (write_device(vpath, "0")) {
    return -1;
  }

  sprintf(vpath, SITE_GPIO_VAL, slot_id, SITE_GPIO_BIT_FS_RESET);
  if (write_device(vpath, "0")) {
    return -1;
  }

  sprintf(vpath, SITE_GPIO_VAL, slot_id, SITE_GPIO_BIT_SRESET);
  if (write_device(vpath, "0")) {
    return -1;
  }

  // Apply the power on line now.
  sprintf(vpath, SITE_GPIO_VAL, slot_id, SITE_GPIO_BIT_PWR_UP);
  if (write_device(vpath, "0")) {
    return -1;
  }

  // Do the LED now
  sprintf(vpath, SITE_GPIO_VAL, slot_id, SITE_GPIO_BIT_PWR_UP_LED);
  if (write_device(vpath, LED_ON_TXT)) {
    return -1;
  }

  sprintf(vpath, SITE_GPIO_VAL, slot_id, SITE_GPIO_BIT_PWR_DWN_LED);
  if (write_device(vpath, LED_OFF_TXT)) {
    return -1;
  }

  return PAL_EOK;
}

// Power Off the server in given slot
static int
server_power_off(uint8_t slot_id, bool gs_flag) {
  char vpath[VPATH_SIZE] = {0};

  if (slot_id < 1 || slot_id > TRACK1_MAX_NUM_SLOTS) {
    return -1;
  }

  sprintf(vpath, SITE_GPIO_VAL, slot_id, SITE_GPIO_BIT_PWR_DATA_LED);
  if (write_device(vpath, LED_ON_TXT)) {
    return -1;
  }

  // TODO: <MPK> Some signal needs to be set here to tell them to shutdown?
  // Check with freeRTOS version
  if (gs_flag) {
    sleep(DELAY_GRACEFUL_SHUTDOWN);
  } else {
    sleep(DELAY_POWER_OFF);
  }

  sprintf(vpath, SITE_GPIO_VAL, slot_id, SITE_GPIO_BIT_PWR_DATA_LED);
  if (write_device(vpath, LED_OFF_TXT)) {
    return -1;
  }

  sprintf(vpath, SITE_GPIO_VAL, slot_id, SITE_GPIO_BIT_PWR_UP);
  if (write_device(vpath, "1")) {
    return -1;
  }

  sprintf(vpath, SITE_GPIO_VAL, slot_id, SITE_GPIO_BIT_PWR_UP_LED);
  if (write_device(vpath, LED_OFF_TXT)) {
    return -1;
  }
  sprintf(vpath, SITE_GPIO_VAL, slot_id, SITE_GPIO_BIT_PWR_DWN_LED);
  if (write_device(vpath, LED_ON_TXT)) {
    return -1;
  }
  return PAL_EOK;
}

// Control 48V to the server in a given slot
// MPK, we can'yt toggle this, as it will mess up the retimers
// and lock the i2c up, so we just turn on the 12/48 .
static int
server_48v_on(uint8_t slot_id) {
  char vpath[MAX_VALUE_LEN] = {0};
  int val;
/***
  uint32_t power_mask = 0;

  if (slot_id < 1 || slot_id > TRACK1_MAX_NUM_SLOTS) {
    return -1;
  }

  // Read keyvalue, or in our sever value, save it out
  // Then turn on 48V via gpio (0 indicates success)
  // In theory
  if (!pal_get_key_value("server_48v_status",vpath)) {
    // Convert to binary
    sscanf(vpath, "%u", &power_mask);
  }
  printf("MPK: power_mask = 0x%02x\n", power_mask);
  power_mask |= (1 << (slot_id));
  printf("MPK: power_mask = 0x%02x\n", power_mask);

  // Convert to ascii
  sprintf(vpath, "%d", power_mask);
  printf("MPK: power_mask (txt) = %s\n", vpath);
  pal_set_key_value("server_48v_status",vpath);
***/
  // Now do the hardware vlaue

  sprintf(vpath, GPIO_PS_ON_VAL);

  if (read_device(vpath, &val)) {
    return -1;
  }

  if (val == 0x0) {
    return 1;
  }
  printf("MPK: Turning on main PS_ON\n");
  if (write_device(vpath, GPIO_HIGH)) {
    return -1;
  }

  return PAL_EOK;
}

// Turn off 48V for the server in given slot
// MPK, we can'yt toggle this, as it will mess up the retimers
// and lock the i2c up, so we just ignore it.
static int
server_48v_off(uint8_t slot_id) {
/*******
  char vpath[MAX_VALUE_LEN] = {0};
  int val;
  uint32_t power_mask = 0;
  if (slot_id < 1 || slot_id > TRACK1_MAX_NUM_SLOTS) {
    return -1;
  }

 // Read keyvalue, or in our sever value, save it out
  // Then turn on 48V via gpio (0 indicates success)
  //
  if (!pal_get_key_value("server_48v_status",vpath)) {
    // Convert to binary
    sscanf(vpath, "%u", &power_mask);
  }
  printf("MPK: power_mask = 0x%02x\n", power_mask);
  power_mask &= ~(1 << (slot_id));
  printf("MPK: power_mask = 0x%02x\n", power_mask);
  // Convert to ascii
  sprintf(vpath, "%d", power_mask);
  printf("MPK: power_mask (txt) = %s\n", vpath);
  pal_set_key_value("server_48v_status",vpath);

  // Now do the hardware value, if it's zero,
  // otherwise something is using the 48V.
  if (0x0 == power_mask) {
    sprintf(vpath, GPIO_PS_ON_VAL);


    if (read_device(vpath, &val)) {
      return -1;
    }
    // Device is already off
    if (val == 0x0) {
      return 1;
    }

    // Set it off
    printf("MPK: Turning off main PS_ON\n");
    if (write_device(vpath, GPIO_HIGH)) {
      return -1;
    }
  }
  *******/
  return PAL_EOK;
}


///////////////////////////////////////////////////////////////
// Track1 specific Platform Abstraction Layer (PAL) Functions
int
pal_get_platform_name(char *name) {
  // Return Track1 Specific value
  strcpy(name, TRACK1_PLATFORM_NAME);
  return PAL_EOK;
}

int
pal_get_num_slots(uint8_t *num) {
  // Return Track1 Specific Value
  *num = TRACK1_MAX_NUM_SLOTS;
  return PAL_EOK;
}

bool
pal_is_fru_x86(uint8_t fru)
{
  return false;
}

// Power Off, Power On, or Power Reset the server in given slot
int
pal_set_server_power(uint8_t slot_id, uint8_t cmd) {
  int ret;
  uint8_t status;
  bool gs_flag = false;

  if (slot_id < 1 || slot_id > FRU_QFDB_A) {
    return -1;
  }

  // If we're not messing with the 12V(48V) then check that the fru is ready
  if ((cmd != SERVER_12V_OFF) && (cmd != SERVER_12V_ON) && (cmd != SERVER_12V_CYCLE)) {
    ret = pal_is_fru_ready(slot_id, &status); //Break out if fru is not ready
    if ((ret < 0) || (status == 0)) {
      return -2;
    }

    if (pal_get_server_power(slot_id, &status) < 0) {
      return -1;
    }
   }

  switch(cmd) {
    case SERVER_POWER_ON:
      if (status == SERVER_POWER_ON)
        return 1;
      else
        // check 12V is on for this slot, if not, we can't
        // turn on!
        pal_is_server_12v_on(slot_id, &status);
        if (status)
          return server_power_on(slot_id);
        else
          return -1;
      break;

    case SERVER_POWER_OFF:
      if (status == SERVER_POWER_OFF)
        return 1;
      else
        return server_power_off(slot_id, gs_flag);
      break;

    case SERVER_POWER_CYCLE:
      if (status == SERVER_POWER_ON) {
        if (server_power_off(slot_id, gs_flag))
          return -1;

        sleep(DELAY_POWER_CYCLE);

        return server_power_on(slot_id);

      } else if (status == SERVER_POWER_OFF) {

        return (server_power_on(slot_id));
      }
      break;

    case SERVER_POWER_RESET:
      if (status == SERVER_POWER_ON) {
        ret = pal_set_rst_btn(slot_id, 0);
        if (ret < 0)
          return ret;
        msleep(100); //some server miss to detect a quick pulse, so delay 100ms between low high
        ret = pal_set_rst_btn(slot_id, 1);
        if (ret < 0)
          return ret;
      } else if (status == SERVER_POWER_OFF) {
        printf("Ignore to execute power reset action when the power status of server is off\n");
        return -2;
      }
      break;

    case SERVER_GRACEFUL_SHUTDOWN:
      if (status == SERVER_POWER_OFF) {
        return 1;
      } else {
        gs_flag = true;
        return server_power_off(slot_id, gs_flag);
      }
      break;

    case SERVER_12V_ON:
      return server_48v_on(slot_id);

    case SERVER_12V_OFF:
      // Turn off server first.
      if (pal_get_server_power(slot_id, &status) < 0) {
        return -1;
      }
      if (status == SERVER_POWER_ON)
        server_power_off(slot_id, gs_flag);

      return server_48v_off(slot_id);

    case SERVER_12V_CYCLE:
      // Turn off server first.
      if (pal_get_server_power(slot_id, &status) < 0) {
        return -1;
      }
      if (status == SERVER_POWER_ON)
        server_power_off(slot_id, gs_flag);

      if (server_48v_off(slot_id) < 0) {
        return -1;
      }

      sleep(DELAY_48V_CYCLE);

      return (server_48v_on(slot_id));

      // <MPK> Should we re-power server? if it was on?

    case SERVER_GLOBAL_RESET:
      return server_power_off(slot_id, false);

    default:
      return -1;
  }

  return PAL_EOK;
}

// Is the high power on, (48 v in track 1)
int
pal_is_server_12v_on(uint8_t slot_id, uint8_t *status) {

  char vpath[MAX_VALUE_LEN] = {0};
  int val;
  uint32_t power_mask = 0;

  if (slot_id < 1 || slot_id > TRACK1_MAX_NUM_SLOTS) {
    return -1;
  }

  // Read keyvalue, or in our sever value, save it out
  // Then turn on 48V via gpio (0 indicates success)
  // In theory
  if (!pal_get_key_value("server_48v_status",vpath)) {
    // Convert to binary
    sscanf(vpath, "%u", &power_mask);
  }

  sprintf(vpath, GPIO_PS_ON_VAL);

  if (read_device(vpath, &val)) {
    return -1;
  }

  // PS is on and slot is "masked as being on", PS_ON is active low
  // MPK Changed this to just say if the PS_ON is set, we'll ignore the rest
  if ( (val == 0x1) /* && ((power_mask & (1<<slot_id)) != 0x0 ) */ ) {
    *status = 1;
  } else {
    *status = 0;
  }

  return PAL_EOK;
}

// Get the server power status

int
pal_get_server_power(uint8_t slot_id, uint8_t *status) {
  int ret;
  int val;
  uint8_t status_12v;
  char vpath[VPATH_SIZE] = {0};
  static uint8_t last_status[MAX_NODES+1] = {0};

  /* Check whether the system is 12V off or on */
  ret = pal_is_server_12v_on(slot_id, &status_12v);
  if (ret < 0) {
    syslog(LOG_ERR, "pal_get_server_power: pal_is_server_12v_on failed");
    return -1;
  }

  /* If 12V-off, return */
  if (!(status_12v)) {
    *status = SERVER_12V_OFF;
    last_status[slot_id] = SERVER_POWER_OFF;
    return PAL_EOK;
  }

  /* If 12V-on, check if the CPU is turned on or not */
  // Check the Site GPIO for PWR_UP.
  sprintf(vpath, SITE_GPIO_VAL, slot_id, SITE_GPIO_BIT_PWR_UP);
  if (read_device(vpath, &val)) {
      *status = last_status[slot_id];
      return -1;

  }

  if (val == 0x0) {
    *status = SERVER_POWER_ON;
  } else {
    *status = SERVER_POWER_OFF;
  }
  last_status[slot_id] = *status;

  return PAL_EOK;
}

// Does a slot represent a server?
int
pal_is_slot_server(uint8_t fru) {
  int ret = 0;
  switch(fru) {
    //case FRU_TPDB_B:
    //case FRU_TPDB_A:
    case FRU_KDB_B:
    case FRU_KDB_A:
    case FRU_QFDB_D:
    case FRU_QFDB_C:
    case FRU_QFDB_B:
    case FRU_QFDB_A:
      ret = 1;
      break;
  }
  return ret;
}

// Is something present?
// We can check the fileystem where the auto-detect has checked?

int
pal_is_fru_prsnt(uint8_t fru, uint8_t *status) {
  uint8_t val;

  switch (fru) {
    case FRU_TPDB_B:
    case FRU_TPDB_A:
    case FRU_KDB_B:
    case FRU_KDB_A:
    case FRU_QFDB_D:
    case FRU_QFDB_C:
    case FRU_QFDB_B:
    case FRU_QFDB_A:
      // what DB is in the fru slot?
      track1_get_fru_type(fru, &val);

      // Is it present, and less than the count?
      if ( (val != DB_TYPE_NONE) ) {
        *status = 1;
      } else {
        *status = 0;
      }
      break;
    case FRU_BMC:
      *status = 1;
      break;
    default:
      return -1;
  }

  return PAL_EOK;
}

// Is the FRU ready?
// We're going to assume if it's present, it' ready!
int
pal_is_fru_ready(uint8_t fru, uint8_t *status) {
  return pal_is_fru_prsnt(fru, status);
}

// We have no debug card/mech at present.. so say no
int
pal_is_debug_card_prsnt(uint8_t *status) {
  *status = 0;
  return PAL_EOK;
}


// We assume we'll have a power button??
// Return the Front panel Power Button
int
pal_get_pwr_btn(uint8_t *status) {
  //char vpath[MAX_VALUE_LEN] = {0};
  //int val;

  //sprintf(vpath, GPIO_VAL, GPIO_PWR_BTN);
  //if (read_device(vpath, &val)) {
    //return -1;
  //}

  //if (val) {
    //*status = 0x0;
  //} else {
    //*status = 0x1;
  //}
  status = 0x0;
  return PAL_EOK;
}

// We assume we'll have a Reset button??
// Return the front panel's Reset Button status
#define BUTTON_VAL "/tmp/mezzanine/gpio/RST_BTN/value"
int
pal_get_rst_btn(uint8_t *status) {
  int val;

  if (read_device(BUTTON_VAL, &val)) {
    return -1;
  }

  if (val) {
    *status = 0x0;
  } else {
    *status = 0x1;
  }
  return PAL_EOK;
}

// Update the Reset button input to the server at given slot
// status = 0 for reset, active low
int
pal_set_rst_btn(uint8_t slot, uint8_t status) {
  char vpath[MAX_VALUE_LEN] = {0};

  if (slot < 1 || slot > TRACK1_MAX_NUM_SLOTS) {
    return -1;
  }

  if (status) {

    // Come out of reset
    // Release SRESET
    sprintf(vpath, SITE_GPIO_VAL, slot, SITE_GPIO_BIT_SRESET);
    if (write_device(vpath, "1")) {
      return -1;
    }
    // Hold state state for 1sec
    msleep(2000);
    // re-assurt sreset?!?!?
    if (write_device(vpath, "0")) {
      return -1;
    }
    msleep(1000);
    // Now release them all.
    sprintf(vpath, SITE_GPIO_VAL, slot, SITE_GPIO_BIT_FN_RESET);
    if (write_device(vpath, "1")) {
      return -1;
    }

    sprintf(vpath, SITE_GPIO_VAL, slot, SITE_GPIO_BIT_FS_RESET);
    if (write_device(vpath, "1")) {
      return -1;
    }

    sprintf(vpath, SITE_GPIO_VAL, slot, SITE_GPIO_BIT_SRESET);
    if (write_device(vpath, "1")) {
      return -1;
    }

  } else {
    // Go into reset
    msleep(2000);
    sprintf(vpath, SITE_GPIO_VAL, slot, SITE_GPIO_BIT_FN_RESET);
    if (write_device(vpath, "0")) {
      return -1;
    }

    sprintf(vpath, SITE_GPIO_VAL, slot, SITE_GPIO_BIT_FS_RESET);
    if (write_device(vpath, "0")) {
      return -1;
    }

    sprintf(vpath, SITE_GPIO_VAL, slot, SITE_GPIO_BIT_SRESET);
    if (write_device(vpath, "0")) {
      return -1;
    }

  }

  return PAL_EOK;
}

// Set a named LED
int
pal_set_led(uint8_t led, uint8_t status)
{
  char *val;

  switch (led) {
    case BMC_LED:
      if (LED_ON == status) {
        val = "high";
      } else {
        val = "low";
      }
      if (write_device(BMC_LED_FILE, val)) {
        return -1;
      }
      break;
    default:
      break;

  }
  return PAL_EOK;
}

// Update Heartbeet LED
// <MPK> HB Led currently running Kernel HB info
int
pal_set_hb_led(uint8_t status) {
  char vpath[MAX_VALUE_LEN] = {0};
  char *val;

  if (status) {
    val = "255";
  } else {
    val = "0";
  }


  sprintf(vpath, HB_LED);
  if (write_device(vpath, val)) {
    return -1;
  }

  return PAL_EOK;
}

// Gets a list of the FRUs (daughterboards basically)
int
pal_get_fru_list(char *list) {

  strcpy(list, pal_fru_list);
  return PAL_EOK;
}


int
pal_get_fru_id(char *str, uint8_t *fru) {

  return track1_common_fru_id(str, fru);
}

int
pal_get_fru_name(uint8_t fru, char *name) {

  return track1_common_fru_name(fru, name);
}

// Not sure if this is needed, I dont think so
int
pal_get_fru_sdr_path(uint8_t fru, char *path) {

  return track1_sensor_sdr_path(fru, path);
}

// This is the list of sensors etc.
// these need to be in track1_sensor.c
// Ask what type of board is in the slot, and return list
// appropriate to it.

int
pal_get_fru_sensor_list(uint8_t fru, uint8_t **sensor_list, int *cnt) {
  uint8_t fru_type = DB_TYPE_NONE;
  if (PAL_EOK == track1_get_fru_type(fru, &fru_type)) {
    switch(fru_type) {

      case DB_TYPE_QFDB:
        *sensor_list = (uint8_t *) qfdb_sensor_list;
        *cnt = qfdb_sensor_cnt;
        break;
      case DB_TYPE_KDB:
        *sensor_list = (uint8_t *) kdb_sensor_list;
        *cnt = kdb_sensor_cnt;
        break;
      case DB_TYPE_TPDB:
        *sensor_list = (uint8_t *) tpdb_sensor_list;
        *cnt = tpdb_sensor_cnt;
        break;
      case DB_TYPE_BMC:
        *sensor_list = (uint8_t *) bmc_sensor_list;
        *cnt = bmc_sensor_cnt;
        break;
      case DB_TYPE_NONE:
        *sensor_list = (uint8_t *) NULL;
        *cnt = 0;
        break;
      default:
        *sensor_list = (uint8_t *) NULL;
        *cnt = 0;
  #ifdef DEBUG
        syslog(LOG_WARNING, "pal_get_fru_sensor_list: Wrong fru id %u or type %u", fru, fru_type);
  #endif
        return -1;
    }

  } else {
    return -1;
  }

  return PAL_EOK;
}

// We should do this somehow? maybe into boards eeprom?
int
pal_fruid_write(uint8_t fru, char *path) {
  //return bic_write_fruid(fru, 0, path);
  return PAL_EOK;
}

// Not sure what calls this?
int
pal_sensor_sdr_init(uint8_t fru, sensor_info_t *sinfo) {
  uint8_t status;

  switch(fru) {
    case FRU_TPDB_B:
    case FRU_TPDB_A:
    case FRU_KDB_B:
    case FRU_KDB_A:
    case FRU_QFDB_D:
    case FRU_QFDB_C:
    case FRU_QFDB_B:
    case FRU_QFDB_A:
      pal_is_fru_prsnt(fru, &status);
      break;
    case FRU_BMC:
      status = 1;
      break;
  }

  if (status)
    return track1_sensor_sdr_init(fru, sinfo);
  else
    return -1;
}

// Some kind sensor check thing?
// Looks in an array for the data.
static sensor_check_t *
get_sensor_check(uint8_t fru, uint8_t snr_num) {

  if (fru < 1 || fru > MAX_NUM_FRUS) {
    syslog(LOG_WARNING, "get_sensor_check: Wrong FRU ID %d\n", fru);
    return NULL;
  }

  return &m_snr_chk[fru-1][snr_num];
}

// Main sensor read code

int
pal_sensor_read_raw(uint8_t fru, uint8_t sensor_num, void *value) {

  uint8_t status;
  char key[MAX_KEY_LEN] = {0};
  char str[MAX_VALUE_LEN] = {0};
  int ret;
  uint8_t retry = MAX_READ_RETRY;
  sensor_check_t *snr_chk;

  switch(fru) {
    case FRU_TPDB_B:
    case FRU_TPDB_A:
    case FRU_KDB_B:
    case FRU_KDB_A:
    case FRU_QFDB_D:
    case FRU_QFDB_C:
    case FRU_QFDB_B:
    case FRU_QFDB_A:
      sprintf(key, "slot%d_sensor%d", fru, sensor_num);
      if(pal_is_fru_prsnt(fru, &status) < 0)
         return -1;
      if (!status) {
         return -1;
      }
      break;

    case FRU_BMC:
      sprintf(key, "bmc_sensor%d", sensor_num);
      break;
    default:
      return -1;
  }
  snr_chk = get_sensor_check(fru, sensor_num);

  while (retry) {
    ret = track1_sensor_read(fru, sensor_num, value);
    if(ret >= 0)
      break;
    msleep(50);
    retry--;
  }
  if(ret < 0) {
    snr_chk->val_valid = 0;

    if(fru == FRU_BMC)
      return -1;
    if(pal_get_server_power(fru, &status) < 0)
      return -1;
    // This check helps interpret the IPMI packet loss scenario
    if(status == SERVER_POWER_ON)
      return -1;
    strcpy(str, "NA");
  }
  else {
    // On successful sensor read

    // Do any corrections and such here.
//    if (fru == FRU_SPB) {
//      if (sensor_num == SP_SENSOR_INLET_TEMP) {
//        apply_inlet_correction((float *)value);
//      } else if (sensor_num == SP_SENSOR_HSC_IN_POWER) {
//        power_value_adjust((float *)value);
//      }
//    }

    if ((GETBIT(snr_chk->flag, UCR_THRESH) && (*((float*)value) >= snr_chk->ucr)) ||
        (GETBIT(snr_chk->flag, LCR_THRESH) && (*((float*)value) <= snr_chk->lcr))) {
      if (snr_chk->retry_cnt < MAX_CHECK_RETRY) {
        snr_chk->retry_cnt++;
        if (!snr_chk->val_valid)
          return -1;

        *((float*)value) = snr_chk->last_val;
      }
    }
    else {
      snr_chk->last_val = *((float*)value);
      snr_chk->val_valid = 1;
      snr_chk->retry_cnt = 0;
    }

    sprintf(str, "%.2f",*((float*)value));
  }

  if(kv_set(key, str, 0, 0) < 0) {
#ifdef DEBUG
     syslog(LOG_WARNING, "pal_sensor_read_raw: cache_set key = %s, str = %s failed.", key, str);
#endif
    return -1;
  }
  else {
    return ret;
  }
}

// This seems to set some flags based on what sensor?
int
pal_sensor_threshold_flag(uint8_t fru, uint8_t snr_num, uint16_t *flag) {

  switch(fru) {
    case FRU_TPDB_B:
    case FRU_TPDB_A:
    case FRU_KDB_B:
    case FRU_KDB_A:
    case FRU_QFDB_D:
    case FRU_QFDB_C:
    case FRU_QFDB_B:
    case FRU_QFDB_A:
//      if (snr_num == BIC_SENSOR_SOC_THERM_MARGIN)
//        *flag = GETMASK(SENSOR_VALID) | GETMASK(UCR_THRESH);
//      else if (snr_num == BIC_SENSOR_SOC_PACKAGE_PWR)
//        *flag = GETMASK(SENSOR_VALID);
//      else if (snr_num == BIC_SENSOR_SOC_TJMAX)
//        *flag = GETMASK(SENSOR_VALID);
      break;
    case FRU_BMC:
      /*
       * TODO: This is a HACK (t11229576)
       */
//      switch(snr_num) {
//        case SP_SENSOR_P12V_SLOT1:
//        case SP_SENSOR_P12V_SLOT2:
//        case SP_SENSOR_P12V_SLOT3:
//        case SP_SENSOR_P12V_SLOT4:
//          *flag = GETMASK(SENSOR_VALID);
//          break;

      break;

  }

  return PAL_EOK;
}

int
pal_get_sensor_threshold(uint8_t fru, uint8_t sensor_num, uint8_t thresh, void *value) {
  return track1_sensor_threshold(fru, sensor_num, thresh, value);
}

int
pal_get_sensor_name(uint8_t fru, uint8_t sensor_num, char *name) {
  return track1_sensor_name(fru, sensor_num, name);
}

int
pal_get_sensor_units(uint8_t fru, uint8_t sensor_num, char *units) {
  return track1_sensor_units(fru, sensor_num, units);
}

int
pal_get_fruid_path(uint8_t fru, char *path) {
  return track1_get_fruid_path(fru, path);
}

int
pal_get_fruid_eeprom_path(uint8_t fru, char *path) {
  return track1_get_fruid_eeprom_path(fru, path);
}

int
pal_get_fruid_name(uint8_t fru, char *name) {
  return track1_get_fruid_name(fru, name);
}

// This does the setup for the key pairs

int
pal_set_def_key_value(void) {

  int ret;
  int i;
  int fru;
  char key[MAX_KEY_LEN] = {0};
  char kpath[MAX_KEY_PATH_LEN] = {0};

  i = 0;
  while(strcmp(key_list[i], LAST_KEY)) {

    memset(key, 0, MAX_KEY_LEN);
    memset(kpath, 0, MAX_KEY_PATH_LEN);

    sprintf(kpath, KV_STORE, key_list[i]);

    if (access(kpath, F_OK) == -1) {
      if ((ret = kv_set(key_list[i], def_val_list[i], 0, KV_FPERSIST | KV_FCREATE)) < 0) {
#ifdef DEBUG
          syslog(LOG_WARNING, "pal_set_def_key_value: kv_set failed. %d", ret);
#endif
      }
    }

    i++;
  }

  /* Actions to be taken on Power On Reset */
  if (pal_is_bmc_por()) {

    for (fru = 1; fru <= MAX_NUM_FRUS; fru++) {

      /* Clear all the SEL errors */
      memset(key, 0, MAX_KEY_LEN);

      switch(fru) {
        case FRU_TPDB_B:
        case FRU_TPDB_A:
        case FRU_KDB_B:
        case FRU_KDB_A:
        case FRU_QFDB_D:
        case FRU_QFDB_C:
        case FRU_QFDB_B:
        case FRU_QFDB_A:
          sprintf(key, "slot%d_sel_error", fru);
        break;

        case FRU_BMC:
          continue;

        default:
          return -1;
      }

      /* Write the value "1" which means FRU_STATUS_GOOD */
      ret = pal_set_key_value(key, "1");

      /* Clear all the sensor health files*/
      memset(key, 0, MAX_KEY_LEN);

      switch(fru) {
        case FRU_TPDB_B:
        case FRU_TPDB_A:
        case FRU_KDB_B:
        case FRU_KDB_A:
        case FRU_QFDB_D:
        case FRU_QFDB_C:
        case FRU_QFDB_B:
        case FRU_QFDB_A:
          sprintf(key, "slot%d_sensor_health", fru);
        break;

        case FRU_BMC:
          continue;

        default:
          return -1;
      }

      /* Write the value "1" which means FRU_STATUS_GOOD */
      ret = pal_set_key_value(key, "1");
    }
  }

  return PAL_EOK;
}

/* Return the tty of the first port. */
int
pal_get_fru_devtty(uint8_t fru, char *devtty) {

  switch(fru) {
    case FRU_KDB_B:
      sprintf(devtty, "/dev/ttyUL10");
      break;

    case FRU_KDB_A:
      sprintf(devtty, "/dev/ttyUL12");
      break;

    case FRU_QFDB_D:
      sprintf(devtty, "/dev/ttyUL6");
      break;

    case FRU_QFDB_C:
      sprintf(devtty, "/dev/ttyUL8");
      break;

    case FRU_QFDB_B:
      sprintf(devtty, "/dev/ttyUL2");
      break;

    case FRU_QFDB_A:
      sprintf(devtty, "/dev/ttyUL4");
      break;

    default:
#ifdef DEBUG
      syslog(LOG_WARNING, "pal_get_fru_devtty: Wrong fru id %u", fru);
#endif
      return -1;
  }
    return PAL_EOK;
}
/* Return the tty of the second port. */
/* Which is numerically lower for some reason */
int
pal_get_fru_devtty2(uint8_t fru, char *devtty) {

  switch(fru) {
    case FRU_KDB_B:
      sprintf(devtty, "/dev/ttyUL9");
      break;

    case FRU_KDB_A:
      sprintf(devtty, "/dev/ttyUL11");
      break;

    case FRU_QFDB_D:
      sprintf(devtty, "/dev/ttyUL5");
      break;

    case FRU_QFDB_C:
      sprintf(devtty, "/dev/ttyUL7");
      break;

    case FRU_QFDB_B:
      sprintf(devtty, "/dev/ttyUL1");
      break;

    case FRU_QFDB_A:
      sprintf(devtty, "/dev/ttyUL3");
      break;

    default:
#ifdef DEBUG
      syslog(LOG_WARNING, "pal_get_fru_devtty: Wrong fru id %u", fru);
#endif
      return -1;
  }
    return PAL_EOK;
}

void
pal_dump_key_value(void) {
  int i = 0;
  int ret;

  char value[MAX_VALUE_LEN] = {0x0};

  while (strcmp(key_list[i], LAST_KEY)) {
    printf("%s:", key_list[i]);
    if ((ret = kv_get(key_list[i], value, NULL, KV_FPERSIST)) < 0) {
      printf("\n");
    } else {
      printf("%s\n",  value);
    }
    i++;
    memset(value, 0, MAX_VALUE_LEN);
  }
}

int
pal_set_last_pwr_state(uint8_t fru, char *state) {

  int ret;
  char key[MAX_KEY_LEN] = {0};

  switch(fru) {
    case FRU_TPDB_B:
    case FRU_TPDB_A:
    case FRU_KDB_B:
    case FRU_KDB_A:
    case FRU_QFDB_D:
    case FRU_QFDB_C:
    case FRU_QFDB_B:
    case FRU_QFDB_A:
      sprintf(key, "slot%d_last_state", (int) fru);

      ret = pal_set_key_value(key, state);
      if (ret < 0) {
#ifdef DEBUG
        syslog(LOG_WARNING, "pal_set_last_pwr_state: pal_set_key_value failed for "
          "fru %u", fru);
#endif
      }
      break;

    case FRU_BMC:
      ret = PAL_EOK;
      break;

    default:
        ret = PAL_EOK;
        break;
      }
  return ret;
}

int
pal_get_last_pwr_state(uint8_t fru, char *state) {
  int ret;
  char key[MAX_KEY_LEN] = {0};

  switch(fru) {
     case FRU_TPDB_B:
     case FRU_TPDB_A:
     case FRU_KDB_B:
     case FRU_KDB_A:
     case FRU_QFDB_D:
     case FRU_QFDB_C:
     case FRU_QFDB_B:
     case FRU_QFDB_A:
       sprintf(key, "slot%d_last_state", (int) fru);

       ret = pal_get_key_value(key, state);
       if (ret < 0) {
#ifdef DEBUG
        syslog(LOG_WARNING, "pal_get_last_pwr_state: pal_get_key_value failed for "
            "fru %u", fru);
#endif
      }
      return ret;
    case FRU_BMC:
      sprintf(state, "on");
      return PAL_EOK;
  }

  return PAL_EOK;
}

// Needs to get GUID from someplace? eeprom on Mezzanine?
int
pal_get_sys_guid(uint8_t slot, char *guid) {
  //return bic_get_sys_guid(slot, (uint8_t*) guid);
  return PAL_EOK;
}

int
pal_set_sysfw_ver(uint8_t slot, uint8_t *ver) {
  int i;
  char key[MAX_KEY_LEN] = {0};
  char str[MAX_VALUE_LEN] = {0};
  char tstr[10] = {0};

  sprintf(key, "sysfw_ver_slot%d", (int) slot);

  for (i = 0; i < SIZE_SYSFW_VER; i++) {
    sprintf(tstr, "%02x", ver[i]);
    strcat(str, tstr);
  }

  return pal_set_key_value(key, str);
}

int
pal_get_sysfw_ver(uint8_t slot, uint8_t *ver) {
  int i;
  int j = 0;
  int ret;
  int msb, lsb;
  char key[MAX_KEY_LEN] = {0};
  char str[MAX_VALUE_LEN] = {0};
  char tstr[4] = {0};

  sprintf(key, "sysfw_ver_slot%d", (int) slot);

  ret = pal_get_key_value(key, str);
  if (ret) {
    return ret;
  }

  for (i = 0; i < 2*SIZE_SYSFW_VER; i += 2) {
    sprintf(tstr, "%c\n", str[i]);
    msb = strtol(tstr, NULL, 16);

    sprintf(tstr, "%c\n", str[i+1]);
    lsb = strtol(tstr, NULL, 16);
    ver[j++] = (msb << 4) | lsb;
  }

  return PAL_EOK;
}

// Need to make this work out if we're POR or not.
int
pal_is_bmc_por(void) {
/*  uint32_t scu_fd;
  uint32_t wdt;
  void *scu_reg;
  void *scu_wdt;

  scu_fd = open("/dev/mem", O_RDWR | O_SYNC );
  if (scu_fd < 0) {
    return 0;
  }

  scu_reg = mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, scu_fd,
             AST_SCU_BASE);
  scu_wdt = (char*)scu_reg + WDT_OFFSET;

  wdt = *(volatile uint32_t*) scu_wdt;

  munmap(scu_reg, PAGE_SIZE);
  close(scu_fd);

  if (wdt & 0x6) {
    return 0;
  } else {
    return 1;
  }
*/

  return 1;
}

// Not sure which sensors should be in this?
int
pal_get_fru_discrete_list(uint8_t fru, uint8_t **sensor_list, int *cnt) {

  switch(fru) {
     case FRU_TPDB_B:
     case FRU_TPDB_A:
     case FRU_KDB_B:
     case FRU_KDB_A:
     case FRU_QFDB_D:
     case FRU_QFDB_C:
     case FRU_QFDB_B:
     case FRU_QFDB_A:
      *sensor_list = NULL;
      *cnt = 0;
      break;

    case FRU_BMC:
      *sensor_list = NULL;
      *cnt = 0;
      break;
    default:
#ifdef DEBUG
      syslog(LOG_WARNING, "pal_get_fru_discrete_list: Wrong fru id %u", fru);
#endif
      return -1;
  }
  return PAL_EOK;
}

// Not used, so remove it, as it'll abort the build due to Werror
#if 0
static void
_print_sensor_discrete_log(uint8_t fru, uint8_t snr_num, char *snr_name,
    uint8_t val, char *event) {
  if (val) {
    syslog(LOG_CRIT, "ASSERT: %s discrete - raised - FRU: %d, num: 0x%X,"
        " snr: %-16s val: %d", event, fru, snr_num, snr_name, val);
  } else {
    syslog(LOG_CRIT, "DEASSERT: %s discrete - settled - FRU: %d, num: 0x%X,"
        " snr: %-16s val: %d", event, fru, snr_num, snr_name, val);
  }
  pal_update_ts_sled();
}
#endif
// This needs to be updated with what ever sensors we decide to use?
int
pal_sensor_discrete_check(uint8_t fru, uint8_t snr_num, char *snr_name,
    uint8_t o_val, uint8_t n_val) {

/**************
  char name[32];
  bool valid = false;
  uint8_t diff = o_val ^ n_val;

  if (GETBIT(diff, 0)) {
    switch(snr_num) {
      case BIC_SENSOR_SYSTEM_STATUS:
        sprintf(name, "SOC_Thermal_Trip");
        valid = true;
        break;
      case BIC_SENSOR_VR_HOT:
        sprintf(name, "SOC_VR_Hot");
        valid = true;
        break; case FRU_TPDB_B:
     case FRU_TPDB_A:
     case FRU_KDB_B:
     case FRU_KDB_A:
     case FRU_QFDB_D:
     case FRU_QFDB_C:
     case FRU_QFDB_B:
     case FRU_QFDB_A:
    }
    if (valid) {
      _print_sensor_discrete_log( fru, snr_num, snr_name, GETBIT(n_val, 0), name);
      valid = false;
    }
  }

  if (GETBIT(diff, 1)) {
    switch(snr_num) {
      case BIC_SENSOR_SYSTEM_STATUS:
        sprintf(name, "SOC_FIVR_Fault");
        valid = true;
        break;
      case BIC_SENSOR_VR_HOT:
        sprintf(name, "SOC_DIMM_VR_Hot");
        valid = true;
        break;
      case BIC_SENSOR_CPU_DIMM_HOT:
        sprintf(name, "SOC_MEMHOT");
        valid = true;
        break;
    }
    if (valid) {
      _print_sensor_discrete_log( fru, snr_num, snr_name, GETBIT(n_val, 1), name);
      valid = false;
    }
  }

  if (GETBIT(diff, 2)) {
    switch(snr_num) {
      case BIC_SENSOR_SYSTEM_STATUS:
        sprintf(name, "SOC_Throttle");
        valid = true;
        break;
    }
    if (valid) {
      _print_sensor_discrete_log( fru, snr_num, snr_name, GETBIT(n_val, 2), name);
      valid = false;
    }
  }
**********/
  return PAL_EOK;
}

int
pal_set_sensor_health(uint8_t fru, uint8_t value) {

  char key[MAX_KEY_LEN] = {0};
  char cvalue[MAX_VALUE_LEN] = {0};

  switch(fru) {
    case FRU_TPDB_B:
    case FRU_TPDB_A:
    case FRU_KDB_B:
    case FRU_KDB_A:
    case FRU_QFDB_D:
    case FRU_QFDB_C:
    case FRU_QFDB_B:
    case FRU_QFDB_A:
      sprintf(key, "slot%d_sensor_health", fru);
      break;

    case FRU_BMC:
      sprintf(key, "bmc_sensor_health");
      break;

    default:
      return -1;
  }

  sprintf(cvalue, (value > 0) ? "1": "0");

  return pal_set_key_value(key, cvalue);
}

int
pal_get_fru_health(uint8_t fru, uint8_t *value) {

  char cvalue[MAX_VALUE_LEN] = {0};
  char key[MAX_KEY_LEN] = {0};
  int ret;

  switch(fru) {
    case FRU_TPDB_B:
    case FRU_TPDB_A:
    case FRU_KDB_B:
    case FRU_KDB_A:
    case FRU_QFDB_D:
    case FRU_QFDB_C:
    case FRU_QFDB_B:
    case FRU_QFDB_A:
      sprintf(key, "slot%d_sensor_health", fru);
      break;

    case FRU_BMC:
      sprintf(key, "bmc_sensor_health");
      break;

    default:
      return -1;
  }

  ret = pal_get_key_value(key, cvalue);
  if (ret) {
    return ret;
  }

  *value = atoi(cvalue);

  memset(key, 0, MAX_KEY_LEN);
  memset(cvalue, 0, MAX_VALUE_LEN);

  switch(fru) {
    case FRU_TPDB_B:
    case FRU_TPDB_A:
    case FRU_KDB_B:
    case FRU_KDB_A:
    case FRU_QFDB_D:
    case FRU_QFDB_C:
    case FRU_QFDB_B:
    case FRU_QFDB_A:
      sprintf(key, "slot%d_sel_error", fru);
      break;
    case FRU_BMC:
      return 0;

    default:
      return -1;
  }

  ret = pal_get_key_value(key, cvalue);
  if (ret) {
    return ret;
  }

  *value = *value & atoi(cvalue);
  return 0;
}

void
pal_update_ts_sled(void)
{
  char key[MAX_KEY_LEN] = {0};
  char tstr[MAX_VALUE_LEN] = {0};
  struct timespec ts;

  clock_gettime(CLOCK_REALTIME, &ts);
  sprintf(tstr, "%d", (int) ts.tv_sec);

  sprintf(key, "timestamp_sled");

  pal_set_key_value(key, tstr);
}

void
pal_log_clear(char *fru) {
  char key[MAX_KEY_LEN] = {0};

  if (!strcasecmp(fru, "slot1")) {
    pal_set_key_value("slot1_sensor_health", "1");
    pal_set_key_value("slot1_sel_error", "1");
  } else if (!strcasecmp(fru, "slot2")) {
    pal_set_key_value("slot2_sensor_health", "1");
    pal_set_key_value("slot2_sel_error", "1");
  } else if (!strcasecmp(fru, "slot3")) {
    pal_set_key_value("slot3_sensor_health", "1");
    pal_set_key_value("slot3_sel_error", "1");
  } else if (!strcasecmp(fru, "slot4")) {
    pal_set_key_value("slot4_sensor_health", "1");
    pal_set_key_value("slot4_sel_error", "1");

  } else if (!strcasecmp(fru, "bmc")) {
    pal_set_key_value("bmc_sensor_health", "1");
  } else if (!strcmp(fru, "all")) {
    int i;
    for (i = FRU_TPDB_B; i <= FRU_QFDB_A; i++) {
      sprintf(key, "slot%d_sensor_health", i);
      pal_set_key_value(key, "1");
      sprintf(key, "slot%d_sel_error", i);
      pal_set_key_value(key, "1");
    }
    pal_set_key_value("bmc_sensor_health", "1");
  }
}

// We could use these to set the boot seq GPIO pins on the site?
// Would make sense.
//
// "JTAG"    {set bootval 0x00 }
// "QSPI-32" {set bootval 0x60 }
//
// *boot points to an array, of at least SIZE_BOOT_ORDER, we only use
// the first byte.
int
pal_get_boot_order(uint8_t slot, uint8_t *req_data, uint8_t *boot, uint8_t *res_len) {
  int ret;
  char key[MAX_KEY_LEN] = {0};
  char str[MAX_VALUE_LEN] = {0};
  char tstr[4] = {0};

  sprintf(key, "slot%u_boot_order", slot);
  ret = pal_get_key_value(key, str);
  if (ret) {
    *res_len = 0;
     return ret;
  }

  memset(boot, 0x00, 1);
  sprintf(tstr, "%c\n", str[0]);
  boot[0] = strtol(tstr, NULL, 16);

  *res_len = 1;
  return 0;
}

int
pal_set_boot_order(uint8_t slot, uint8_t *boot, uint8_t *res_data, uint8_t *res_len) {
  char key[MAX_KEY_LEN] = {0};
  char str[MAX_VALUE_LEN] = {0};
  char tstr[10] = {0};

  sprintf(key, "slot%u_boot_order", slot);

  snprintf(tstr, 3, "%02x", boot[0]);

  *res_len = 0;
  return pal_set_key_value(key, str);
}

int
pal_is_crashdump_ongoing(uint8_t slot)
{
  char key[MAX_KEY_LEN] = {0};
  char value[MAX_VALUE_LEN] = {0};
  int ret;
  sprintf(key, CRASHDUMP_KEY, slot);
  ret = kv_get(key, value, NULL, 0);
  if (ret < 0) {
#ifdef DEBUG
     syslog(LOG_INFO, "pal_is_crashdump_ongoing: failed");
#endif
     return 0;
  }
  if (atoi(value) > 0)
     return 1;
  return 0;
}

bool
pal_is_fw_update_ongoing(uint8_t fru) {

  char key[MAX_KEY_LEN];
  char value[MAX_VALUE_LEN] = {0};
  int ret;
  struct timespec ts;

  switch (fru) {
    case FRU_TPDB_B:
    case FRU_TPDB_A:
    case FRU_KDB_B:
    case FRU_KDB_A:
    case FRU_QFDB_D:
    case FRU_QFDB_C:
    case FRU_QFDB_B:
    case FRU_QFDB_A:
      sprintf(key, "slot%d_fwupd", fru);
      break;
    case FRU_BMC:
    default:
      return false;
  }

  ret = kv_get(key, value, NULL, 0);
  if (ret < 0) {
     return false;
  }

  clock_gettime(CLOCK_MONOTONIC, &ts);
  if (strtoul(value, NULL, 10) > ts.tv_sec)
     return true;

  return false;
}

// Pull this from eeprom?
int
pal_get_board_rev_id(uint8_t *id) {

      return 0;
}

// Pull this from slot id switches?
int
pal_get_mb_slot_id(uint8_t *id) {

      return 0;
}

int
pal_get_slot_cfg_id(uint8_t *id) {

      return 0;
}

void
pal_get_chassis_status(uint8_t slot, uint8_t *req_data, uint8_t *res_data, uint8_t *res_len) {

  char key[MAX_KEY_LEN] = {0};
  sprintf(key, "slot%d_por_cfg", slot);
  char buff[MAX_VALUE_LEN];
  int policy = 3;
  uint8_t status, ret;
  unsigned char *data = res_data;

  // Platform Power Policy
  if (pal_get_key_value(key, buff) == 0)
  {
    if (!memcmp(buff, "off", strlen("off")))
      policy = 0;
    else if (!memcmp(buff, "lps", strlen("lps")))
      policy = 1;
    else if (!memcmp(buff, "on", strlen("on")))
      policy = 2;
    else
      policy = 3;
  }

  // Current Power State
  ret = pal_get_server_power(slot, &status);
  if (ret >= 0) {
    *data++ = status | (policy << 5);
  } else {
    // load default
    syslog(LOG_WARNING, "ipmid: pal_get_server_power failed for slot1\n");
    *data++ = 0x00 | (policy << 5);
  }
  *data++ = 0x00;   // Last Power Event
  *data++ = 0x40;   // Misc. Chassis Status
  *data++ = 0x00;   // Front Panel Button Disable
  *res_len = data - res_data;
}

uint8_t
pal_set_power_restore_policy(uint8_t slot, uint8_t *pwr_policy, uint8_t *res_data) {

  uint8_t completion_code;
  char key[MAX_KEY_LEN] = {0};
  sprintf(key, "slot%d_por_cfg", slot);
  completion_code = CC_SUCCESS;   // Fill response with default values
  unsigned char policy = *pwr_policy & 0x07;  // Power restore policy

  switch (policy)
  {
    case 0:
      if (pal_set_key_value(key, "off") != 0)
        completion_code = CC_UNSPECIFIED_ERROR; // -1
      break;
    case 1:
      if (pal_set_key_value(key, "lps") != 0)
        completion_code = CC_UNSPECIFIED_ERROR; // -1
      break;
    case 2:
      if (pal_set_key_value(key, "on") != 0)
        completion_code = CC_UNSPECIFIED_ERROR; // -1
      break;
    case 3:
    // no change (just get present policy support)
      break;
    default:
        completion_code = CC_PARAM_OUT_OF_RANGE; // -1
      break;
  }
  return completion_code;
}

// Should this return the "type" of device?
int
pal_get_platform_id(uint8_t *id) {
   return 0;
}

int
pal_get_fw_info(uint8_t fru, unsigned char target, unsigned char* res, unsigned char* res_len)
{
    return -1;
}

//For OEM command "CMD_OEM_GET_PLAT_INFO" 0x7e
int pal_get_plat_sku_id(void){
  return 0; // Yosemite V1
}


void
pal_get_me_name(uint8_t fru, char *target_name) {
  strcpy(target_name, "ICE");
}
