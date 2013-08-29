CFLAGS += -Wall
CFLAGS += -std=gnu99

# Makedepend
CFLAGS += -MD -MP

LDLIBS += -lm

.PHONY: all clean

include mode.inc
