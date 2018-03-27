# Copyright 2018-present Iceotope. All Rights Reserved.
lib: libtrack1_sensor.so

libtrack1_sensor.so: track1_sensor.c
	$(CC) $(CFLAGS) -fPIC -c -o track1_sensor.o track1_sensor.c
	$(CC) -lm -lipmi -lipmb -ltrack1_common -shared -o libtrack1_sensor.so track1_sensor.o -lc  $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf *.o libtrack1_sensor.so
