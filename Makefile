common_sources = common.h common.c
device_sources = device.h device.c

CFLAGS = -Os -Wall -fstrict-aliasing -Wstrict-aliasing -Wsign-conversion -fPIC -I.

ifdef CI
    CFLAGS += -Werror
endif

.PHONY: all clean

all: main

main: clean
	$(CC) $(CFLAGS) -o $ sense ${device_sources} ${common_sources} main.h main.c

clean:
	$(RM) main
