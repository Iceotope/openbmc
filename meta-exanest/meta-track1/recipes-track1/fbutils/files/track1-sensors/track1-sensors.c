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

int
main(int argc, char **argv) {
  int value;
  float fvalue;
  uint8_t slot_id = 0;

  if (argc >1)
    slot_id = atoi(argv[1]);

/* Need to read out all the sensors here, via the sensorlibrary
 * so that needs fixing up.
 * It will talk to the PAL library, so that needs doing as well
 */

  //if (track1_sensor_read(slot_id, SP_SENSOR_INLET_TEMP, &fvalue)) {
    //printf("track1_sensor_read failed: SP_SENSOR_INLET_TEMP\n");
  //} else {
    //printf("SP_SENSOR_INLET_TEMP: %.2f C\n", fvalue);
  //}

  return 0;
}
