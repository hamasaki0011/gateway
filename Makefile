commons = common.h common.c
devices = device.h device.c

CFLAGS = -Os -Wall -fstrict-aliasing -Wstrict-aliasing -Wsign-conversion -Wno-unused-result -fPIC -I.

ifdef CI
    CFLAGS += -Werror
endif

.PHONY: all clean

all: main

main: clean
	$(CC) $(CFLAGS) -o $ sensing ${devices} ${commons} main.h main.c

clean:
	$(RM) main
