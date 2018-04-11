/*
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
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
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>

#include <linux/i2c-dev.h>

#include <dirent.h>
#include <openbmc/obmc-i2c.h>
#include "track1_sensor.h"

#define LARGEST_DEVICE_NAME 120


/******************************************
 * Starting of port..
 ******************************************/

#define TRACK1_SDR_PATH "/tmp/sdr_%s.bin"
#define XADC_PATH "/tmp/mezzanine/bmc/xadc"
#define XADC_TEMP_OFFSET "in_temp0_offset"
#define XADC_TEMP_RAW "in_temp0_raw"
#define XADC_TEMP_SCALE "in_temp0_scale"

#define XADC_TEMP_OFFSET "in_temp0_offset"
#define XADC_TEMP_RAW "in_temp0_raw"
#define XADC_TEMP_SCALE "in_temp0_scale"

#define XADC_VOLTAGE_RAW_FMT "in_voltage%s_raw"
#define XADC_VOLTAGE_SCALE_FMT "in_voltage%s_scale"
#define DB_TYPE_FILE "/tmp/mezzanine/db_%d/type"
#define UNIT_DIV 1000

// Base board site root bus, retimers and power chip etc.
#define I2C_DEV_MEZZ "/dev/i2c-1"
#define I2C_SLAVE_LTC2946_ADDR 0x67

/*
 * XADC files are these
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage0_vccint_raw
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage0_vccint_scale
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage1_vccaux_raw
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage1_vccaux_scale
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage2_vccbram_raw
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage2_vccbram_scale
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage3_vccpint_raw
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage3_vccpint_scale
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage4_vccpaux_raw
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage4_vccpaux_scale
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage5_vccoddr_raw
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage5_vccoddr_scale
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage6_vrefp_raw
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage6_vrefp_scale
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage7_vrefn_raw
 * -rw-r--r--    1 root     root          4096 Apr  9 13:44 in_voltage7_vrefn_scale
 **/
const char *voltagenames[8] = {"0_vccint",
  "1_vccaux",
  "2_vccbram",
  "3_vccpint",
  "4_vccpaux",
  "5_vccoddr",
  "6_vrefp",
  "7_vrefn"
  };

const uint8_t qfdb_sensor_list[] = {
  QFDB_SENSOR_DUMMY
};

const uint8_t tpdb_sensor_list[] = {
  TPDB_SENSOR_W1_1,
  TPDB_SENSOR_W1_2,
  TPDB_SENSOR_W1_3,
  TPDB_SENSOR_W1_4,
  TPDB_SENSOR_W1_5,
  TPDB_SENSOR_W1_6,
  TPDB_SENSOR_W1_7,
  TPDB_SENSOR_W1_8
};

const uint8_t kdb_sensor_list[] = {
  KDB_SENSOR_DUMMY
};

const uint8_t bmc_sensor_list[] = {
  BMC_SENSOR_TEMP,
  BMC_SENSOR_V0,
  BMC_SENSOR_V1,
  BMC_SENSOR_V2,
  BMC_SENSOR_V3,
  BMC_SENSOR_V4,
  BMC_SENSOR_V5,
  BMC_SENSOR_V48,
  BMC_SENSOR_I48,
  BMC_SENSOR_P48

};

size_t qfdb_sensor_cnt = sizeof(qfdb_sensor_list)/sizeof(uint8_t);
size_t tpdb_sensor_cnt = sizeof(tpdb_sensor_list)/sizeof(uint8_t);
size_t kdb_sensor_cnt = sizeof(kdb_sensor_list)/sizeof(uint8_t);
size_t bmc_sensor_cnt = sizeof(bmc_sensor_list)/sizeof(uint8_t);

static sensor_info_t g_sinfo[MAX_NUM_FRUS][MAX_SENSOR_NUM] = {0};

/* Helper code to read the device nodes */
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

static int
read_device_float(const char *device, float *value) {
  FILE *fp;
  int rc;
  char tmp[10];

  fp = fopen(device, "r");
  if (!fp) {
    int err = errno;
#ifdef DEBUG
    syslog(LOG_INFO, "failed to open device %s", device);
#endif
    return err;
  }

  rc = fscanf(fp, "%s", tmp);
  fclose(fp);

  if (rc != 1) {
#ifdef DEBUG
    syslog(LOG_INFO, "failed to read device %s", device);
#endif
    return ENOENT;
  }

  *value = atof(tmp);

  return 0;
}

/* Populates all sensor_info_t struct using the path to SDR dump */
int
sdr_init(char *path, sensor_info_t *sinfo) {
  int fd;
  uint8_t buf[MAX_SDR_LEN] = {0};
  uint8_t bytes_rd = 0;
  uint8_t snr_num = 0;
  sdr_full_t *sdr;

  while (access(path, F_OK) == -1) {
    sleep(5);
  }

  fd = open(path, O_RDONLY);
  if (fd < 0) {
    syslog(LOG_ERR, "sdr_init: open failed for %s\n", path);
  return -1;
  }

  while ((bytes_rd = read(fd, buf, sizeof(sdr_full_t))) > 0) {
    if (bytes_rd != sizeof(sdr_full_t)) {
      syslog(LOG_ERR, "sdr_init: read returns %d bytes\n", bytes_rd);
      return -1;
    }

    sdr = (sdr_full_t *) buf;
    snr_num = sdr->sensor_num;
    sinfo[snr_num].valid = true;
    memcpy(&sinfo[snr_num].sdr, sdr, sizeof(sdr_full_t));
  }

  return 0;
}

static int
track1_sdr_init(uint8_t fru) {

  static bool init_done[MAX_NUM_FRUS] = {false};

  if (!init_done[fru - 1]) {

    sensor_info_t *sinfo = g_sinfo[fru-1];

    if (track1_sensor_sdr_init(fru, sinfo) < 0)
      return ERR_NOT_READY;

    init_done[fru - 1] = true;
  }

  return 0;
}

/* Simple check to see if server/slot is there */
static bool
is_server_prsnt(uint8_t fru) {
 char path[LARGEST_DEVICE_NAME];
  int val = 0;

  switch(fru) {
    case FRU_QFDB_A:
    case FRU_QFDB_B:
    case FRU_QFDB_C:
    case FRU_QFDB_D:
    case FRU_KDB_A:
    case FRU_KDB_B:
    case FRU_TPDB_A:
    case FRU_TPDB_B:
      sprintf(path, DB_TYPE_FILE, fru-1);
      if (read_device(path, &val)) {
        return 0;
      }

      if (val != 0x0) {
        return 1;
      } else {
        return 0;
      }
      break;

    case FRU_BMC:
      return 1;
      break;

    default:
      return 0;
  }
}

/* Read XADC temperature sensor */
int
read_xadc_temp( float *value) {
  char vname[LARGEST_DEVICE_NAME];
  float scale = 0.0;
  int raw, offset = 0;
  int err = 0;


  sprintf(vname, "%s/%s", XADC_PATH, XADC_TEMP_OFFSET);
  err |= read_device(vname,&offset);

  sprintf(vname, "%s/%s", XADC_PATH, XADC_TEMP_RAW);
  err |= read_device(vname,&raw);

  sprintf(vname, "%s/%s", XADC_PATH, XADC_TEMP_SCALE);
  err |= read_device_float(vname,&scale);


  if (err)
    return -1;

  *value = ( (float)(raw+offset) * scale)/(float)UNIT_DIV;

  return 0;
}

/* Read main v48 power sensor
 * BMC_SENSOR_V48 and BMC_SENSOR_I48
 *
 * Need to do raw i2c operations to read out the registers.
 * (Really needs a kernel driver, but we don't have one..yet!)
 *
 * Chip is on the SITE bus (i2c-1) at location 0x67 ?
 */

#define LTC2946_REG_GPIO        0x33
#define LTC2946_GPIO1_ON        0b01000000
#define LTC2946_GPIO1_OFF       0b00000000
#define LTC2946_REG_CHIPID_LOW  0xE8
#define LTC2946_REG_CHIPID_HIGH 0xE7
#define LTC2946_REG_CLKDIV      0x43

#define LTC2946_REG_POWER       0x5
#define LTC2946_REG_DELTASENSE  0x14
#define LTC2946_REG_VIN         0x1E
#define LTC2946_REG_ADIN        0x28
#define LTC2946_REG_ENERGY      0x3C

#define LTC2946_REG_POWER_LEN   0x3
#define LTC2946_REG_DELTASENSE_LEN 0x2
#define LTC2946_REG_VIN_LEN     0x2
#define LTC2946_REG_ADIN_LEN    0x2
#define LTC2946_REG_ENERGY_LEN  0x4

#define LTC2946_COUNT_TO_MV       25 //25mV per LSB
#define LTC2946_COUNT_TO_MV_SCALE 1  // Scaling factor
#define LTC2946_COUNT_TO_MA       5 //5mA per LSB
#define LTC2946_COUNT_TO_MA_SCALE 1 //Scaling factor // answer in mA

#define LTC2946_COUNT_TO_MW       125 //125uW per LSB
#define LTC2946_COUNT_TO_MW_SCALE 1000 //Scaling factor, convert to mW

int
read_v48_sensor(uint8_t snr_num, float *value) {
  int ret = -1;
  int dev;
  uint8_t read_buffer[4] = {0};
  uint8_t read_reg = LTC2946_REG_VIN; // default to something sane

// Buffers used for the read ops.
  struct i2c_msg msg[2] = {
    {
      .addr = I2C_SLAVE_LTC2946_ADDR,
      .len = 1,
      .buf = &read_reg,
    },
    {
      .addr = I2C_SLAVE_LTC2946_ADDR,
      .flags = I2C_M_RD,
      .len = 2,   // Default
      .buf = read_buffer,
    }
  };

  struct i2c_rdwr_ioctl_data msg_top;

   if ( (snr_num == BMC_SENSOR_V48) ||
        (snr_num == BMC_SENSOR_I48) ||
        (snr_num == BMC_SENSOR_P48) ) {
    dev = open(I2C_DEV_MEZZ, O_RDWR);
    if (dev < 0) {
      syslog(LOG_ERR, "read_v48_sensor(): open() failed,  errno=%s",
             strerror(errno));
      return -1;
    }
    /* Assign the i2c device address */
    ret = ioctl(dev, I2C_SLAVE, I2C_SLAVE_LTC2946_ADDR);
    if (ret < 0) {
      syslog(LOG_ERR, "read_v48_sensor(): ioctl(I2C_SLAVE) failed");
    } else {
      // Device is open and ready
      //

      switch (snr_num) {
        case BMC_SENSOR_V48:
          read_reg = LTC2946_REG_VIN;
          msg[1].len = LTC2946_REG_VIN_LEN; // 2 Byte op for this.
          break;
        case BMC_SENSOR_I48:
          read_reg = LTC2946_REG_DELTASENSE;
          msg[1].len = LTC2946_REG_DELTASENSE_LEN; // 2 Byte op for this.
          break;
        case BMC_SENSOR_P48:
          read_reg = LTC2946_REG_POWER;
          msg[1].len = LTC2946_REG_POWER_LEN; // 3 Byte op for this.
          break;
      }

      msg_top.msgs = msg;
      msg_top.nmsgs = 2;

      if(ioctl(dev, I2C_RDWR, &msg_top) < 0) {
         syslog(LOG_ERR, "read_v48_sensor(): ioctl(I2C_RDWR) failed");
      } else {
        uint8_t gpio_buf[2];
        // Convert to float value.
        switch (snr_num) {

        case BMC_SENSOR_V48:
          *value = (float)( ((read_buffer[0]<<4) | (read_buffer[1] >>4)) *
                        LTC2946_COUNT_TO_MV) / LTC2946_COUNT_TO_MV_SCALE;

          /* Do a check and set GPIO1 (48V LED) on/off if within limits. */
          gpio_buf[0] = LTC2946_REG_GPIO;
          gpio_buf[1] = LTC2946_GPIO1_OFF;

          if ( ((float)MIN_48V_LIMIT_MIN < (*value)) && ( (*value) < (float)MIN_48V_LIMIT_MAX) )
            gpio_buf[1] = LTC2946_GPIO1_ON;
          break;

          // Write
          write(dev, gpio_buf, 2);

        case BMC_SENSOR_I48:
          *value = (float)( ((read_buffer[0]<<4) | (read_buffer[1] >>4)) *
                        LTC2946_COUNT_TO_MA) / LTC2946_COUNT_TO_MA_SCALE;
          break;
        case BMC_SENSOR_P48:
          *value = (float)(((read_buffer[0]<<16) | (read_buffer[1]<<8) | read_buffer[2]) *
                        LTC2946_COUNT_TO_MW) / LTC2946_COUNT_TO_MW_SCALE;

          break;
        }
      }
    *value = *value/(float)UNIT_DIV;
    }
    if (dev)
      close(dev);
  }
  return ret;
}

/* Read XADC voltage sensor */
/* Sensor num - BMC_SENSOR_V0 givews us the text file index into
 * the naming array
 */
int
read_xadc_voltage(uint8_t sensor_num, float *value) {
  char vname[LARGEST_DEVICE_NAME];
  char fname[LARGEST_DEVICE_NAME];
  float raw, scale = 0.0;
  int err = 0;

  sprintf(fname, XADC_VOLTAGE_RAW_FMT, voltagenames[sensor_num-BMC_SENSOR_V0]);
  sprintf(vname, "%s/%s", XADC_PATH, fname);
  err |= read_device_float(vname,&raw);

  sprintf(fname, XADC_VOLTAGE_SCALE_FMT, voltagenames[sensor_num-BMC_SENSOR_V0]);
  sprintf(vname, "%s/%s", XADC_PATH, fname);
  err |= read_device_float(vname,&scale);

  if (err)
    return -1;

  *value = (raw*scale)/(float)UNIT_DIV;
  return 0;

}

/* Main Sensor reading code */
int track1_sensor_read(uint8_t fru, uint8_t sensor_num, void *value) {

  float volt;
  float curr;
  int ret;
  bool discrete;
  int i;

  if (!(is_server_prsnt(fru))) {
    return -1;
  }

// No SDR, as no IMPI link to dbs
//  ret = track1_sdr_init(fru);
//  if (ret < 0) {
//     return ret;
//  }

  switch (fru) {

    case FRU_QFDB_A:
    case FRU_QFDB_B:
    case FRU_QFDB_C:
    case FRU_QFDB_D:
     /* Fake this currently, as we have no data! */
      switch(sensor_num) {
        case QFDB_SENSOR_DUMMY:
        *(float *)value = (0xA0 +fru);
        break;
      }
      break;

    case FRU_KDB_A:
    case FRU_KDB_B:
     /* Fake this currently, as we have no data! */
      switch(sensor_num) {
        case KDB_SENSOR_DUMMY:
        *(float *)value = (0x50 + fru);
        break;
      }
      break;


    case FRU_TPDB_A:
    case FRU_TPDB_B:
      switch(sensor_num) {
        case TPDB_SENSOR_W1_1:
        case TPDB_SENSOR_W1_2:
        case TPDB_SENSOR_W1_3:
        case TPDB_SENSOR_W1_4:
        case TPDB_SENSOR_W1_5:
        case TPDB_SENSOR_W1_6:
        case TPDB_SENSOR_W1_7:
        case TPDB_SENSOR_W1_8:
        // i2c->1W bus chat, as we don't have a driver?
        // need to try instansiating a driver via user space
        // not sure I can however. TEST it! If we can, it's a lot
        // easier to pull from sysfs space
        break;
      }

    case FRU_BMC:
      switch(sensor_num) {
        /* XADC Temp sensor */
        case BMC_SENSOR_TEMP:
          // Calculate ((raw+offset)*scale)/DIV
          // read_xadc_temp()
          return read_xadc_temp( (float *)value);
          break;

        /* XADC Voltage sensors */
        case BMC_SENSOR_V0:
        case BMC_SENSOR_V1:
        case BMC_SENSOR_V2:
        case BMC_SENSOR_V3:
        case BMC_SENSOR_V4:
        case BMC_SENSOR_V5:
          // Calculate ((raw*scale)/DIV
          return read_xadc_voltage(sensor_num, (float *)value);
          break;
        /* Power Mon */
        case BMC_SENSOR_V48:
        case BMC_SENSOR_I48:
        case BMC_SENSOR_P48:
          // i2c chat to the power chip, as no driver currently!
          // do raw i2c operations?
          return read_v48_sensor(sensor_num, (float *)value);
          break;
      }
      break;

  }
  return 0;
}

/* Gives sensor readable name */
int track1_sensor_name(uint8_t fru, uint8_t sensor_num, char *name) {
  switch(fru) {
    case FRU_QFDB_A:
    case FRU_QFDB_B:
    case FRU_QFDB_C:
    case FRU_QFDB_D:
      switch(sensor_num) {
        case QFDB_SENSOR_DUMMY:
          sprintf(name, "QFDB_DUMMY_SENSOR");
          break;

        default:
          sprintf(name, "");
          break;
      }
      break;

    case FRU_KDB_A:
    case FRU_KDB_B:
      switch(sensor_num) {
        case KDB_SENSOR_DUMMY:
          sprintf(name, "KDB_DUMMY_SENSOR");
          break;
         default:
          sprintf(name, "");
          break;
      }
      break;

    case FRU_BMC:
      switch(sensor_num) {
        case BMC_SENSOR_TEMP:
          sprintf(name, "BMC_Die_Temp");
          break;
        case BMC_SENSOR_V0:
          sprintf(name, "BMC_Vcc_int");
          break;
        case BMC_SENSOR_V1:
          sprintf(name, "BMC_Vcc_aux");
          break;
        case BMC_SENSOR_V2:
          sprintf(name, "BMC_Vcc_bram");
          break;
        case BMC_SENSOR_V3:
          sprintf(name, "BMC_Vcc_pint");
          break;
        case BMC_SENSOR_V4:
          sprintf(name, "BMC_Vcc_paux");
          break;
        case BMC_SENSOR_V5:
          sprintf(name, "BMC_Vcc_ddr");
          break;
        case BMC_SENSOR_V48:
          sprintf(name, "48V Supply (Volts)");
          break;
        case BMC_SENSOR_I48:
          sprintf(name, "48V Supply (Current)");
          break;
        case BMC_SENSOR_P48:
          sprintf(name, "48V Supply (Power)");
          break;
        default:
          sprintf(name, "");
          break;
      }
      break;

    case FRU_TPDB_A:
    case FRU_TPDB_B:
      switch(sensor_num) {
        case TPDB_SENSOR_W1_1:
          sprintf(name, "TPDB_Temp_ext0");
          break;
        case TPDB_SENSOR_W1_2:
          sprintf(name, "TPDB_Temp_ext1");
          break;
        case TPDB_SENSOR_W1_3:
          sprintf(name, "TPDB_Temp_ext2");
          break;
        case TPDB_SENSOR_W1_4:
          sprintf(name, "TPDB_Temp_ext3");
          break;
        case TPDB_SENSOR_W1_5:
          sprintf(name, "TPDB_Temp_pwm0");
          break;
        case TPDB_SENSOR_W1_6:
          sprintf(name, "TPDB_Temp_pwm1");
          break;
        case TPDB_SENSOR_W1_7:
          sprintf(name, "TPDB_Temp_pwm2");
          break;
        case TPDB_SENSOR_W1_8:
          sprintf(name, "TPDB_Temp_pwm3");
          break;

        default:
          sprintf(name, "");
          break;
      }
      break;
    }
  return 0;
}
/* Gives sensor readable unit name */
int track1_sensor_units(uint8_t fru, uint8_t sensor_num, char *units) {
  uint8_t op, modifier;
  sensor_info_t *sinfo;

//  if (is_server_prsnt(fru) && (track1_sdr_init(fru) != 0)) {
//    return -1;
//    }

  switch(fru) {
    case FRU_QFDB_A:
    case FRU_QFDB_B:
    case FRU_QFDB_C:
    case FRU_QFDB_D:
      switch(sensor_num) {
        case QFDB_SENSOR_DUMMY:
          sprintf(units, "C");
          break;
        default:
          sprintf(units, "");
          break;
      }
      break;

    case FRU_KDB_A:
    case FRU_KDB_B:
      switch(sensor_num) {
        case KDB_SENSOR_DUMMY:
          sprintf(units, "C");
          break;
        default:
          sprintf(units, "");
          break;
      }
      break;

    case FRU_BMC:
      switch(sensor_num) {
        case BMC_SENSOR_TEMP:
          sprintf(units, "C");
          break;
        case BMC_SENSOR_V0:
          sprintf(units, "V");
          break;
        case BMC_SENSOR_V1:
          sprintf(units, "V");
          break;
        case BMC_SENSOR_V2:
          sprintf(units, "V");
          break;
        case BMC_SENSOR_V3:
          sprintf(units, "V");
          break;
        case BMC_SENSOR_V4:
          sprintf(units, "V");
          break;
        case BMC_SENSOR_V5:
          sprintf(units, "V");
          break;
        case BMC_SENSOR_V48:
          sprintf(units, "V");
          break;
        case BMC_SENSOR_I48:
          sprintf(units, "A");
          break;
        case BMC_SENSOR_P48:
          sprintf(units, "W");
          break;
         default:
          sprintf(units, "");
          break;
      }
      break;

    case FRU_TPDB_A:
    case FRU_TPDB_B:
      switch(sensor_num) {
        case TPDB_SENSOR_W1_1:
          sprintf(units, "C");
          break;
        case TPDB_SENSOR_W1_2:
          sprintf(units, "C");
          break;
        case TPDB_SENSOR_W1_3:
          sprintf(units, "C");
          break;
        case TPDB_SENSOR_W1_4:
          sprintf(units, "C");
          break;
        case TPDB_SENSOR_W1_5:
          sprintf(units, "C");
          break;
        case TPDB_SENSOR_W1_6:
          sprintf(units, "C");
          break;
        case TPDB_SENSOR_W1_7:
          sprintf(units, "C");
          break;
        case TPDB_SENSOR_W1_8:
          sprintf(units, "C");
          break;

        default:
          sprintf(units, "");
          break;
      }
      break;
    }
  return 0;
}

/* Does the thresholds */
int
track1_sensor_threshold(uint8_t fru, uint8_t sensor_num,
  uint8_t thresh, float *value) {

  return 0;
}

/* Path to the sensor data base */
int
track1_sensor_sdr_path(uint8_t fru, char *path) {

  char fru_name[16] = {0};

  switch(fru) {
    case FRU_QFDB_A:
      sprintf(fru_name, "%s", "qfdb-a");
      break;
    case FRU_QFDB_B:
      sprintf(fru_name, "%s", "qfdb-b");
      break;
    case FRU_QFDB_C:
      sprintf(fru_name, "%s", "qfdb-c");
      break;
    case FRU_QFDB_D:
      sprintf(fru_name, "%s", "qfdb-d");
      break;

    case FRU_KDB_A:
      sprintf(fru_name, "%s", "kdb-a");
      break;

    case FRU_KDB_B:
      sprintf(fru_name, "%s", "kdb-b");
      break;

    case FRU_TPDB_A:
      sprintf(fru_name, "%s", "tpdb-a");
      break;

    case FRU_TPDB_B:
      sprintf(fru_name, "%s", "tpdb-b");
      break;

    case FRU_BMC:
      sprintf(fru_name, "%s", "bmc");
      break;
    default:
#ifdef DEBUG
      syslog(LOG_WARNING, "track1_sensor_sdr_path: Wrong Slot ID\n");
#endif
    return -1;
  }
  sprintf(path, TRACK1_SDR_PATH, fru_name);

  if (access(path, F_OK) == -1) {
    return -1;
  }

  return 0;
}

/* Initialise the sensor database */
int track1_sensor_sdr_init(uint8_t fru, sensor_info_t *sinfo) {
  int fd;
  uint8_t buf[MAX_SDR_LEN] = {0};
  uint8_t bytes_rd = 0;
  uint8_t sn = 0;
  char path[64] = {0};

  switch(fru) {

// We do not use SDR, as we're not using ipmi comms to daughtboards
    case FRU_TPDB_B:
    case FRU_TPDB_A:
    case FRU_KDB_B:
    case FRU_KDB_A:
    case FRU_QFDB_D:
    case FRU_QFDB_C:
    case FRU_QFDB_B:
    case FRU_QFDB_A:

      return -1;
/********************************************************************

      if (track1_sensor_sdr_path(fru, path) < 0) {
#ifdef DEBUG
        syslog(LOG_WARNING, "track1_sensor_sdr_init: get_fru_sdr_path failed\n");
#endif
        return ERR_NOT_READY;
      }

      if (sdr_init(path, sinfo) < 0) {
#ifdef DEBUG
        syslog(LOG_ERR, "track1_sensor_sdr_init: sdr_init failed for FRU %d", fru);
#endif
      }
***********************************************************************/
      break;

    case FRU_BMC:
      return -1;
      break;
    default:
      return -1;
      break;
  }

  return 0;
}


/******************************************
 * MPK Removed for now.. needs porting!!
 ******************************************/

//#define GPIO_VAL "/sys/class/gpio/gpio%d/value"

//#define I2C_BUS_9_DIR "/sys/class/i2c-adapter/i2c-9/"
//#define I2C_BUS_10_DIR "/sys/class/i2c-adapter/i2c-10/"

//#define TACH_DIR "/sys/devices/platform/ast_pwm_tacho.0"
//#define ADC_DIR "/sys/devices/platform/ast_adc.0"

//#define SP_INLET_TEMP_DEVICE I2C_BUS_9_DIR "9-004e"
//#define SP_OUTLET_TEMP_DEVICE I2C_BUS_9_DIR "9-004f"
//#define HSC_DEVICE I2C_BUS_10_DIR "10-0040"

//#define FAN_TACH_RPM "tacho%d_rpm"
//#define ADC_VALUE "adc%d_value"
//#define HSC_IN_VOLT "in1_input"
//#define HSC_OUT_CURR "curr1_input"
//#define HSC_TEMP "temp1_input"
//#define HSC_IN_POWER "power1_input"



//#define I2C_DEV_NIC "/dev/i2c-11"
//#define I2C_NIC_ADDR 0x1f
//#define I2C_NIC_SENSOR_TEMP_REG 0x01

//#define BIC_SENSOR_READ_NA 0x20

//#define MAX_SENSOR_NUM 0xFF
//#define ALL_BYTES 0xFF
//#define LAST_REC_ID 0xFFFF

//#define YOSEMITE_SDR_PATH "/tmp/sdr_%s.bin"
//#define ADM1278_R_SENSE 0.5

//static float hsc_r_sense = ADM1278_R_SENSE;


//float spb_sensor_threshold[MAX_SENSOR_NUM][MAX_SENSOR_THRESHOLD + 1] = {0};
//float nic_sensor_threshold[MAX_SENSOR_NUM][MAX_SENSOR_THRESHOLD + 1] = {0};

//static void
//sensor_thresh_array_init() {
  //static bool init_done = false;

  //if (init_done)
    //return;

  //spb_sensor_threshold[SP_SENSOR_INLET_TEMP][UCR_THRESH] = 40;
  //spb_sensor_threshold[SP_SENSOR_OUTLET_TEMP][UCR_THRESH] = 70;
  //spb_sensor_threshold[SP_SENSOR_FAN0_TACH][UCR_THRESH] = 11500;
  //spb_sensor_threshold[SP_SENSOR_FAN0_TACH][UNC_THRESH] = 8500;
  //spb_sensor_threshold[SP_SENSOR_FAN0_TACH][LCR_THRESH] = 500;
  //spb_sensor_threshold[SP_SENSOR_FAN1_TACH][UCR_THRESH] = 11500;
  //spb_sensor_threshold[SP_SENSOR_FAN1_TACH][UNC_THRESH] = 8500;
  //spb_sensor_threshold[SP_SENSOR_FAN1_TACH][LCR_THRESH] = 500;
  ////spb_sensor_threshold[SP_SENSOR_AIR_FLOW][UCR_THRESH] =  {75.0, 0, 0, 0, 0, 0, 0, 0};
  //spb_sensor_threshold[SP_SENSOR_P5V][UCR_THRESH] = 5.493;
  //spb_sensor_threshold[SP_SENSOR_P5V][LCR_THRESH] = 4.501;
  //spb_sensor_threshold[SP_SENSOR_P12V][UCR_THRESH] = 13.216;
  //spb_sensor_threshold[SP_SENSOR_P12V][LCR_THRESH] = 11.269;
  //spb_sensor_threshold[SP_SENSOR_P3V3_STBY][UCR_THRESH] = 3.625;
  //spb_sensor_threshold[SP_SENSOR_P3V3_STBY][LCR_THRESH] = 2.973;
  //spb_sensor_threshold[SP_SENSOR_P12V_SLOT1][UCR_THRESH] = 13.216;
  //spb_sensor_threshold[SP_SENSOR_P12V_SLOT1][LCR_THRESH] = 11.269;
  //spb_sensor_threshold[SP_SENSOR_P12V_SLOT2][UCR_THRESH] = 13.216;
  //spb_sensor_threshold[SP_SENSOR_P12V_SLOT2][LCR_THRESH] = 11.269;
  //spb_sensor_threshold[SP_SENSOR_P12V_SLOT3][UCR_THRESH] = 13.216;
  //spb_sensor_threshold[SP_SENSOR_P12V_SLOT3][LCR_THRESH] = 11.269;
  //spb_sensor_threshold[SP_SENSOR_P12V_SLOT4][UCR_THRESH] = 13.216;
  //spb_sensor_threshold[SP_SENSOR_P12V_SLOT4][LCR_THRESH] = 11.269;
  //spb_sensor_threshold[SP_SENSOR_P3V3][UCR_THRESH] = 3.625;
  //spb_sensor_threshold[SP_SENSOR_P3V3][LCR_THRESH] = 2.973;
  //spb_sensor_threshold[SP_SENSOR_HSC_IN_VOLT][UCR_THRESH] = 13.2;
  //spb_sensor_threshold[SP_SENSOR_HSC_IN_VOLT][LCR_THRESH] = 10.8;
  //spb_sensor_threshold[SP_SENSOR_HSC_OUT_CURR][UCR_THRESH] = 47.705;
  //spb_sensor_threshold[SP_SENSOR_HSC_TEMP][UCR_THRESH] = 120;
  //spb_sensor_threshold[SP_SENSOR_HSC_IN_POWER][UCR_THRESH] = 525;

  //nic_sensor_threshold[MEZZ_SENSOR_TEMP][UCR_THRESH] = 95;

  //init_done = true;
//}







//static int
//get_current_dir(const char *device, char *dir_name) {
  //char full_name[LARGEST_DEVICE_NAME + 1];
  //DIR *dir = NULL;
  //struct dirent *ent;

  //snprintf(full_name, sizeof(full_name), "%s/hwmon", device);
  //dir = opendir(full_name);
  //if (dir == NULL) {
    //goto close_dir_out;
  //}
  //while ((ent = readdir(dir)) != NULL) {
    //if (strstr(ent->d_name, "hwmon")) {
      //// found the correct 'hwmon??' directory
      //snprintf(dir_name, sizeof(full_name), "%s/hwmon/%s/",
      //device, ent->d_name);
      //goto close_dir_out;
    //}
  //}

//close_dir_out:
  //if (dir != NULL) {
    //if (closedir(dir)) {
      //syslog(LOG_ERR, "%s closedir failed, errno=%s\n",
              //__FUNCTION__, strerror(errno));
    //}
  //}
  //return 0;
//}


//static int
//read_temp_attr(const char *device, const char *attr, float *value) {
  //char full_dir_name[LARGEST_DEVICE_NAME + 1];
  //char dir_name[LARGEST_DEVICE_NAME + 1];
  //int tmp;

  //// Get current working directory
  //if (get_current_dir(device, dir_name))
  //{
    //return -1;
  //}
  //snprintf(
      //full_dir_name, LARGEST_DEVICE_NAME, "%s/%s", dir_name, attr);


  //if (read_device(full_dir_name, &tmp)) {
     //return -1;
  //}

  //*value = ((float)tmp)/UNIT_DIV;

  //return 0;
//}


//static int
//read_temp(const char *device, float *value) {
  //return read_temp_attr(device, "temp1_input", value);
//}

//static int
//read_fan_value(const int fan, const char *device, float *value) {
  //char device_name[LARGEST_DEVICE_NAME];
  //char full_name[LARGEST_DEVICE_NAME];

  //snprintf(device_name, LARGEST_DEVICE_NAME, device, fan);
  //snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", TACH_DIR, device_name);
  //return read_device_float(full_name, value);
//}

//static int
//read_adc_value(const int pin, const char *device, float *value) {
  //char device_name[LARGEST_DEVICE_NAME];
  //char full_name[LARGEST_DEVICE_NAME];

  //snprintf(device_name, LARGEST_DEVICE_NAME, device, pin);
  //snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", ADC_DIR, device_name);
  //return read_device_float(full_name, value);
//}

//static int
//read_hsc_value(const char* attr, const char *device, float r_sense, float *value) {
  //char full_dir_name[LARGEST_DEVICE_NAME];
  //char dir_name[LARGEST_DEVICE_NAME + 1];
  //int tmp;

  //// Get current working directory
  //if (get_current_dir(device, dir_name))
  //{
    //return -1;
  //}
  //snprintf(
      //full_dir_name, LARGEST_DEVICE_NAME, "%s/%s", dir_name, attr);

  //if(read_device(full_dir_name, &tmp)) {
    //return -1;
  //}

  //if ((strcmp(attr, HSC_OUT_CURR) == 0) || (strcmp(attr, HSC_IN_POWER) == 0)) {
    //*value = ((float) tmp)/r_sense/UNIT_DIV;
  //}
  //else {
    //*value = ((float) tmp)/UNIT_DIV;
  //}

  //return 0;
//}

//static int
//read_nic_temp(uint8_t snr_num, float *value) {
  //char command[64];
  //int dev;
  //int ret;
  //uint8_t tmp_val;

  //if (snr_num == MEZZ_SENSOR_TEMP) {
    //dev = open(I2C_DEV_NIC, O_RDWR);
    //if (dev < 0) {
      //syslog(LOG_ERR, "open() failed for read_nic_temp, errno=%s",
             //strerror(errno));
      //return -1;
    //}
    ///* Assign the i2c device address */
    //ret = ioctl(dev, I2C_SLAVE, I2C_NIC_ADDR);
    //if (ret < 0) {
      //syslog(LOG_ERR, "read_nic_temp: ioctl() assigning i2c addr failed");
    //}

    //tmp_val = i2c_smbus_read_byte_data(dev, I2C_NIC_SENSOR_TEMP_REG);

    //close(dev);

    //// TODO: This is a HACK till we find the actual root cause
    //// This condition implies that the I2C bus is busy
    //if (tmp_val == 0xFF) {
      //syslog(LOG_INFO, "read_nic_temp: value 0xFF - i2c bus is busy");
      //return -1;
    //}

    //*value = (float) tmp_val;
  //}

  //return 0;
//}

//static int
//bic_read_sensor_wrapper(uint8_t fru, uint8_t sensor_num, bool discrete,
    //void *value) {

  //int ret;
  //sdr_full_t *sdr;
  //ipmi_sensor_reading_t sensor;

  //ret = bic_read_sensor(fru, sensor_num, &sensor);
  //if (ret) {
    //return ret;
  //}

  //if (sensor.flags & BIC_SENSOR_READ_NA) {
//#ifdef DEBUG
    //syslog(LOG_ERR, "bic_read_sensor_wrapper: Reading Not Available");
    //syslog(LOG_ERR, "bic_read_sensor_wrapper: sensor_num: 0x%X, flag: 0x%X",
        //sensor_num, sensor.flags);
//#endif
    //return -1;
  //}

  //if (discrete) {
    //*(float *) value = (float) sensor.status;
    //return 0;
  //}

  //sdr = &g_sinfo[fru-1][sensor_num].sdr;

  //// If the SDR is not type1, no need for conversion
  //if (sdr->type !=1) {
    //*(float *) value = sensor.value;
    //return 0;
  //}

  //// y = (mx + b * 10^b_exp) * 10^r_exp
  //uint8_t x;
  //uint8_t m_lsb, m_msb, m;
  //uint8_t b_lsb, b_msb, b;
  //int8_t b_exp, r_exp;

  //x = sensor.value;

  //m_lsb = sdr->m_val;
  //m_msb = sdr->m_tolerance >> 6;
  //m = (m_msb << 8) | m_lsb;

  //b_lsb = sdr->b_val;
  //b_msb = sdr->b_accuracy >> 6;
  //b = (b_msb << 8) | b_lsb;

  //// exponents are 2's complement 4-bit number
  //b_exp = sdr->rb_exp & 0xF;
  //if (b_exp > 7) {
    //b_exp = (~b_exp + 1) & 0xF;
    //b_exp = -b_exp;
  //}
  //r_exp = (sdr->rb_exp >> 4) & 0xF;
  //if (r_exp > 7) {
    //r_exp = (~r_exp + 1) & 0xF;
    //r_exp = -r_exp;
  //}

  ////printf("m:%d, x:%d, b:%d, b_exp:%d, r_exp:%d\n", m, x, b, b_exp, r_exp);

  //* (float *) value = ((m * x) + (b * pow(10, b_exp))) * (pow(10, r_exp));

  //if ((sensor_num == BIC_SENSOR_SOC_THERM_MARGIN) && (* (float *) value > 0)) {
   //* (float *) value -= (float) THERMAL_CONSTANT;
  //}

  //return 0;
//}


