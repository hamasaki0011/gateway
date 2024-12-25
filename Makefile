common_sources = common.h common.c
i2c_sources = i2c.h i2c.c
sfa3x_sources = sfa3x_i2c.h sfa3x_i2c.c

CFLAGS = -Os -Wall -fstrict-aliasing -Wstrict-aliasing -Wsign-conversion -fPIC -I.

ifdef CI
    CFLAGS += -Werror
endif

.PHONY: all clean

all: main

main: clean
	$(CC) $(CFLAGS) -o $ sensing  ${sfa3x_sources} ${i2c_sources} ${common_sources} define.h main.c
clean:
	$(RM) main
