common_sources = common.h common.c
i2c_sources = i2c_hal.h i2c.h i2c.c
sfa3x_sources = sfa3x_i2c.h sfa3x_i2c.c
#comspec_sources = comspec.h comspec.c

i2c_implementation ?= i2c_hal.c

CFLAGS = -Os -Wall -fstrict-aliasing -Wstrict-aliasing -Wsign-conversion -fPIC -I.
#CFLAGS = -Os -Wall -fstrict-aliasing -Wstrict-aliasing=1 -Wsign-conversion -fPIC -I.

ifdef CI
    CFLAGS += -Werror
endif

.PHONY: all clean

all: main

main: clean
	#$(CC) $(CFLAGS) -o $ sensing  ${sfa3x_sources} ${i2c_sources} \
	#	${i2c_implementation} ${common_sources} main.c
	$(CC) $(CFLAGS) -o $ sensing  ${sfa3x_sources} ${i2c_sources} ${i2c_implementation} ${common_sources} main.c
clean:
	$(RM) main
