common_sources = sensirion_config.h sensirion_common.h sensirion_common.c
i2c_sources = sensirion_i2c_hal.h sensirion_i2c.h sensirion_i2c.c
sfa3x_sources = sfa3x_i2c.h sfa3x_i2c.c
tcp_sources = tcp_com.h tcp_com.c
#comspec_sources = comspec.h comspec.c

i2c_implementation ?= sensirion_i2c_hal.c

CFLAGS = -Os -Wall -fstrict-aliasing -Wstrict-aliasing -Wsign-conversion -fPIC -I.
#CFLAGS = -Os -Wall -fstrict-aliasing -Wstrict-aliasing=1 -Wsign-conversion -fPIC -I.

ifdef CI
    CFLAGS += -Werror
endif

.PHONY: all clean

all: main

main: clean

	$(CC) $(CFLAGS) -o $ gateway  ${sfa3x_sources} ${i2c_sources} ${tcp_sources} \
		${i2c_implementation} ${common_sources} main.c

clean:
	$(RM) main
