/*
 * fpc-util
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
 * Copyright 2018-present Iceotope. All Rights Reserved.
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
#include <openbmc/pal.h>
#include <openbmc/ipmi.h>

static void
print_usage_help(void) {
  printf("Usage: fpc-util --identify <on/off>\n");
}

int
main(int argc, char **argv) {
  char tstr[64] = {0};

  if (argc < 2) {
    goto err_exit;
  }

  if (strcmp(argv[1], "--identify")) {
    goto err_exit;
  }

  if ( (strcmp(argv[2], "on") != 0) && (strcmp(argv[2], "off") != 0) ) {
    goto err_exit;
  }

  if (argc != 3) {
    goto err_exit;
  }

  printf("fpc-util: identification is %s\n", argv[2]);

  sprintf(tstr, "identify_board");

  return pal_set_key_value(tstr, argv[2]);

err_exit:
  print_usage_help();
  return -1;
}
