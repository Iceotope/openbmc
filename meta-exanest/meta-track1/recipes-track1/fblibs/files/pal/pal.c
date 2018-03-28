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


#include "pal.h"

#define SITE_GPIO_VAL "/tmp/mezzanine/site_%d/gpio/IO/%d/value"
#define SITE_GPIO_DIR "/tmp/mezzanine/site_%d/gpio/IO/%d/direction"
#define GPIO_PS_ON_VAL "/tmp/mezzanine/gpio/PS_ON/value"
#define HB_LED "/sys/class/leds/led-ds23/brightness"
#define DB_TYPE_FILE "/tmp/mezzanine/db_%d/type"

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

#define LED_ON "0"
#define LED_OFF "1"
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
#define SITE_GPIO_BIT_UNUSED_5 24
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

const char pal_server_list[] = "tpdb-b, tbpd-a, kdb-a, kdb-b," \
          " qfdb-d, qfdb-c, qfdb-b, qfdb-a";
const char pal_fru_list[] = "all, tpdb-b, tbpd-a, kdb-a, kdb-b," \
          " qfdb-d, qfdb-c, qfdb-b, qfdb-a, bmc";

char * key_list[] = {
"identify_board",
"identify_slot1",
"identify_slot2",
"identify_slot3",
"identify_slot4",
"identify_slot5",
"identify_slot6",
"identify_slot7",
"identify_slot8",
"server_48v_status",
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

  return kv_get(key, value);
}
int
pal_set_key_value(char *key, char *value) {

  // Check is key is defined and valid
  if (pal_key_check(key))
    return -1;

  return kv_set(key, value);
}

// Power On the server in a given slot
static int
server_power_on(uint8_t slot_id) {
  char vpath[VPATH_SIZE] = {0};

  if (slot_id < 1 || slot_id > TRACK1_MAX_NUM_SLOTS) {
    return -1;
  }
  sprintf(vpath, SITE_GPIO_VAL, slot_id-1, SITE_GPIO_BIT_PWR_UP);

  if (write_device(vpath, "1")) {
    return -1;
  }

  // Do the LED now
  sprintf(vpath, SITE_GPIO_VAL, slot_id-1, SITE_GPIO_BIT_PWR_UP_LED);
  if (write_device(vpath, LED_ON)) {
    return -1;
  }

  sprintf(vpath, SITE_GPIO_VAL, slot_id-1, SITE_GPIO_BIT_PWR_DWN_LED);
  if (write_device(vpath, LED_OFF)) {
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

  sprintf(vpath, SITE_GPIO_VAL, slot_id-1, SITE_GPIO_BIT_PWR_DATA_LED);
  if (write_device(vpath, LED_ON)) {
    return -1;
  }

  // TODO: <MPK> Some signal needs to be set here to tell them to shutdown?
  // Check with freeRTOS version
  if (gs_flag) {
    sleep(DELAY_GRACEFUL_SHUTDOWN);
  } else {
    sleep(DELAY_POWER_OFF);
  }

  sprintf(vpath, SITE_GPIO_VAL, slot_id-1, SITE_GPIO_BIT_PWR_UP);
  if (write_device(vpath, "0")) {
    return -1;
  }

  sprintf(vpath, SITE_GPIO_VAL, slot_id-1, SITE_GPIO_BIT_PWR_UP_LED);
  if (write_device(vpath, LED_OFF)) {
    return -1;
  }
  sprintf(vpath, SITE_GPIO_VAL, slot_id-1, SITE_GPIO_BIT_PWR_DWN_LED);
  if (write_device(vpath, LED_ON)) {
    return -1;
  }
  return PAL_EOK;
}

// Control 48V to the server in a given slot
static int
server_48v_on(uint8_t slot_id) {
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
    power_mask = sscanf(vpath, "%u", &power_mask);
  }
  power_mask |= (1 << (slot_id));

  // Convert to ascii
  sprintf(vpath, "%d", power_mask);

  pal_set_key_value("server_48v_status",vpath);

  // Now do the hardware vlaue

  sprintf(vpath, GPIO_PS_ON_VAL);

  if (read_device(vpath, &val)) {
    return -1;
  }

  if (val == 0x1) {
    return 1;
  }

  if (write_device(vpath, GPIO_HIGH)) {
    return -1;
  }

  return PAL_EOK;
}

// Turn off 48V for the server in given slot
static int
server_48v_off(uint8_t slot_id) {
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
    power_mask = sscanf(vpath, "%u", &power_mask);
  }
  power_mask &= ~(1 << (slot_id));

  // Convert to ascii
  sprintf(vpath, "%d", power_mask);

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
    if (write_device(vpath, GPIO_LOW)) {
      return -1;
    }
  }
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

// Power Off, Power On, or Power Reset the server in given slot
int
pal_set_server_power(uint8_t slot_id, uint8_t cmd) {
  int ret;
  uint8_t status;
  bool gs_flag = false;

  if (slot_id < 1 || slot_id > 4) {
    return -1;
  }

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
        return server_power_on(slot_id);
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
      return server_48v_off(slot_id);

    case SERVER_12V_CYCLE:
      if (server_48v_off(slot_id) < 0) {
        return -1;
      }

      sleep(DELAY_48V_CYCLE);

      return (server_48v_on(slot_id));

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
    power_mask = sscanf(vpath, "%u", &power_mask);
  }

   sprintf(vpath, GPIO_PS_ON_VAL);

  if (read_device(vpath, &val)) {
    return -1;
  }

  if ( (val == 0x1) || (power_mask & (1<<slot_id)) != 0x0 ) {
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
  char vpath[VPATH_SIZE] = {0};
  static uint8_t last_status[MAX_NODES+1] = {0};

  /* Check whether the system is 12V off or on */
  ret = pal_is_server_12v_on(slot_id, status);
  if (ret < 0) {
    syslog(LOG_ERR, "pal_get_server_power: pal_is_server_12v_on failed");
    return -1;
  }

  /* If 12V-off, return */
  if (!(*status)) {
    *status = SERVER_12V_OFF;
    last_status[slot_id] = SERVER_POWER_OFF;
    return PAL_EOK;
  }

  /* If 12V-on, check if the CPU is turned on or not */
  // Check the Site GPIO for PWR_UP.
  sprintf(vpath, SITE_GPIO_VAL, slot_id-1, SITE_GPIO_BIT_PWR_UP);
  if (read_device(vpath, &val)) {
      *status = last_status[slot_id];
      return -1;

  }

  if (val == 0x1) {
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
  int val;
  char vpath[MAX_VALUE_LEN] = {0};

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
      if ( (val > DB_TYPE_NONE) ||(val < DB_TYPE_COUNT) ) {
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
int
pal_get_rst_btn(uint8_t *status) {
  //char vpath[MAX_VALUE_LEN] = {0};
  //int val;

  //sprintf(vpath, GPIO_VAL, GPIO_RST_BTN);
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

// Update the Reset button input to the server at given slot
int
pal_set_rst_btn(uint8_t slot, uint8_t status) {
  char vpath[MAX_VALUE_LEN] = {0};
  char *val;

  if (slot < 1 || slot > TRACK1_MAX_NUM_SLOTS) {
    return -1;
  }

  if (status) {
    val = GPIO_HIGH;
  } else {
    val = GPIO_LOW;
  }

  sprintf(vpath, SITE_GPIO_VAL, slot-1, SITE_GPIO_BIT_SRESET);

  if (write_device(vpath, val)) {
    return -1;
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
