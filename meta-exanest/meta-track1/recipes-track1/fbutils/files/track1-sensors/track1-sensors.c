/*
 * track1-sensors
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
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
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <stdint.h>
#include <pthread.h>
#include <openbmc/ipmi.h>
#include <facebook/track1_sensor.h>


// Do a list of sensors, with 0 as the terminator, then we can just loop
// over the list a lot easier, saves redundent code.

uint8_t BMC_SENSOR_LIST[] = {
  BMC_SENSOR_TEMP,
  BMC_SENSOR_V0,
  BMC_SENSOR_V1,
  BMC_SENSOR_V2,
  BMC_SENSOR_V3,
  BMC_SENSOR_V4,
  BMC_SENSOR_V5,
  BMC_SENSOR_V48,
  BMC_SENSOR_I48,
  BMC_SENSOR_P48,
  0
  };

int
main(int argc, char **argv) {
  float fvalue;
  char units[80];
  char name[80];
  uint8_t slot_id = 0;

  if (argc >1)
    slot_id = atoi(argv[1]);

/* Need to read out all the sensors here, via the sensorlibrary
 * so that needs fixing up.
 * It will talk to the PAL library, so that needs doing as well
 */

  if (slot_id == FRU_BMC) {
    int i = 0;

    while (0 != BMC_SENSOR_LIST[i]) {
      track1_sensor_units(FRU_BMC, BMC_SENSOR_LIST[i], units);
      track1_sensor_name(FRU_BMC, BMC_SENSOR_LIST[i], name);

      if (track1_sensor_read(FRU_BMC, BMC_SENSOR_LIST[i], &fvalue)) {
        printf("track1_sensor_read(%d) failed:\n", i);
      } else {
        printf("Sensor %d (%s): %.2f %s\n", i, name, fvalue, units);
      }

      i++;

    }
  }
  // Now do the slots.
  //if (track1_sensor_read(slot_id, SP_SENSOR_INLET_TEMP, &fvalue)) {
    //printf("track1_sensor_read failed: SP_SENSOR_INLET_TEMP\n");
  //} else {
    //printf("SP_SENSOR_INLET_TEMP: %.2f C\n", fvalue);
  //}

  return 0;
}
