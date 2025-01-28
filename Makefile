commons = common.h common.c
foperations = foperation.h foperation.c
devices = device.h device.c

CFLAGS = -Os -Wall -fstrict-aliasing -Wstrict-aliasing -Wsign-conversion -fPIC -I.

ifdef CI
    CFLAGS += -Werror
endif

.PHONY: all clean

all: main

main: clean
	$(CC) $(CFLAGS) -o $ sense ${devices} ${foperations} ${commons} main.h main.c

clean:
	$(RM) main
