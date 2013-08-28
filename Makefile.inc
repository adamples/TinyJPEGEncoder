CFLAGS += -Wall
CFLAGS += -std=gnu99
# Debug:
CFLAGS += -g
CFLAGS += -O0
CFLAGS += -DDEBUG
#~ # Release:
#~ CFLAGS += -O3
#~ CFLAGS += -DNDEBUG
# Makedepend
CFLAGS += -MD -MP

LDLIBS += -lm
