# Copyright 2015-present Facebook. All Rights Reserved.
all: track1-sensors

CFLAGS += -Wall -Werror -g

track1-sensors: track1-sensors.c
	$(CC) $(CFLAGS) -ltrack1_sensor -ledb -std=c99 -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf *.o track1-sensors
