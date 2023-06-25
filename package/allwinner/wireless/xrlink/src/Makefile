
target = libxrlink.so

INCLUDES += -I$(STAGING_DIR)/usr/include/ -I. -I$(STAGING_DIR)/usr/ -I./common/tracing \
			-I./common/os -I./common/os/linux -I./common/api_action

LIBS +=	-L$(STAGING_DIR)/usr/lib/ -lpthread

LOCAL_CFLAGS += -DOS_NET_LINUX_OS

SRC = $(wildcard ./core/*.c)
SRC += $(wildcard ./common/api_action/*.c)
SRC += $(wildcard ./common/os/linux/*.c)
SRC += $(wildcard ./common/tracing/*.c)

OBJS = $(SRC:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(USRFLAGS) -c -o $@ $<

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/include
	@cp wifimg.h $(CONFIG_PREFIX)/include
	@cp common/tracing/log_core.h $(CONFIG_PREFIX)/include
	@cp common/os/os_net_utils.h $(CONFIG_PREFIX)/include

$(target): $(SRC)
	$(CC) -fPIC -shared $(INCLUDES) $(LOCAL_CFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)
	@cp $@ $(CONFIG_PREFIX)/usr/lib

all: install $(target)

clean:
	rm -rf $(TARGET) *.o *.a *~
	rm -rf ./core/*.o
	rm -rf ./common/api_action/*.o
	rm -rf ./common/os/linux/*.o
	rm -rf ./common/tracing/*.o
