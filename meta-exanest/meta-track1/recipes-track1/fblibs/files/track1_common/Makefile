# Copyright 2018-present Iceotope. All Rights Reserved.
lib: libtrack1_common.so

CFLAGS += -Wall -Werror

libtrack1_common.so: track1_common.c
	$(CC) $(CFLAGS) -fPIC -pthread -c -o track1_common.o track1_common.c
	$(CC) -lpthread -shared -o libtrack1_common.so track1_common.o -lc  $(LDFLAGS)


.PHONY: clean

clean:
	rm -rf *.o libtrack1_common.so
