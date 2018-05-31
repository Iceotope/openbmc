/*
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
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <openbmc/edb.h>
#include <openbmc/ipmi.h>
#include <openbmc/ipmb.h>
#include <openbmc/pal.h>

#define BTN_MAX_SAMPLES   200
#define BTN_POWER_OFF     40

// Thread for monitoring scm plug
static void *
scm_monitor_handler(){
  int curr = -1;
  int prev = -1;
  int ret;
  uint8_t prsnt = 0;
  uint8_t power;
  uint8_t pos = FRU_SCM;

  while (1) {
    ret = pal_is_scm_prsnt(&prsnt);
    if (ret) {
      goto scm_mon_out;
    }
    curr = prsnt;
    if (curr != prev) {
      if (curr) {
        // SCM was inserted
        syslog(LOG_WARNING, "SCM Insertion\n");

        ret = pal_get_server_power(pos, &power);
        if (ret) {
          goto scm_mon_out;
        }
        if (power == SERVER_POWER_OFF) {
          sleep(3);
          syslog(LOG_WARNING, "SCM power on\n");
          pal_set_server_power(pos, SERVER_POWER_ON);
          goto scm_mon_out;
        }
      } else {
        // SCM was removed
        syslog(LOG_WARNING, "SCM Extraction\n");
      }
    }
scm_mon_out:
    prev = curr;
      sleep(1);
  }
  return 0;
}

// Thread for monitoring debug card hotswap
static void *
debug_card_handler() {
  int curr = -1;
  int prev = -1;
  int ret;
  uint8_t prsnt = 0;
  uint8_t lpc;

  while (1) {
 
    // Check if debug card present or not
    ret = pal_is_debug_card_prsnt(&prsnt);

    if (ret) {
      goto debug_card_out;
    }
    curr = prsnt;

    // Check if Debug Card was either inserted or removed
    if (curr != prev) {
      if (!curr) {
        // Debug Card was removed
        syslog(LOG_WARNING, "Debug Card Extraction\n");
      } else {
        // Debug Card was inserted
        syslog(LOG_WARNING, "Debug Card Insertion\n");
      }
    }

    // If Debug Card is present
    if (curr) {

      // Enable POST codes for scm slot
      ret = pal_post_enable(IPMB_BUS);
      if (ret) {
        goto debug_card_done;
      }

      // Get last post code and display it
      ret = pal_post_get_last(IPMB_BUS, &lpc);
      if (ret) {
        goto debug_card_done;
      }

      ret = pal_post_handle(IPMB_BUS, lpc);
      if (ret) {
        goto debug_card_out;
      }
    }

debug_card_done:
    prev = curr;
debug_card_out:
    if (curr == 1)
      msleep(500);
    else
      sleep(1);
  }

  return 0;
}

int
main (int argc, char * const argv[]) {
  pthread_t tid_scm_monitor;
  pthread_t tid_debug_card;
  int rc;
  int pid_file;
  int brd_rev;

  pid_file = open("/var/run/front-paneld.pid", O_CREAT | O_RDWR, 0666);
  rc = flock(pid_file, LOCK_EX | LOCK_NB);
  if(rc) {
    if(EWOULDBLOCK == errno) {
      printf("Another front-paneld instance is running...\n");
      exit(-1);
    }
  } else {
   openlog("front-paneld", LOG_CONS, LOG_DAEMON);
  }

  if (pal_get_board_rev(&brd_rev)) {
    syslog(LOG_WARNING, "Get board revision failed\n");
    exit(-1);
  }

  if (pthread_create(&tid_scm_monitor, NULL, scm_monitor_handler, NULL) < 0) {
    syslog(LOG_WARNING, "pthread_create for scm monitor error\n");
    exit(1);
  }

  if (brd_rev != BOARD_REV_EVTA) {
    if (pthread_create(&tid_debug_card, NULL, debug_card_handler, NULL) < 0) {
        syslog(LOG_WARNING, "pthread_create for debug card error\n");
        exit(1);
    }
    pthread_join(tid_debug_card, NULL);
  }

  pthread_join(tid_scm_monitor, NULL);

  return 0;
}
