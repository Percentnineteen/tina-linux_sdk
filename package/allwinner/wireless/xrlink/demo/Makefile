
INCLUDES += -I$(CONFIG_PREFIX)/include

LIBS +=	-lpthread -L$(STAGING_DIR)/ -L$(CONFIG_PREFIX)/usr/lib -lxrlink

USRFLAGS += -DOS_NET_LINUX_OS

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin

wifi_daemon: wifi_daemon.c
	$(CC) -o $@ $^ $(LIBS) $(LDFLAGS) $(USRFLAGS) $(INCLUDES)

wifi: wifi_main.c
	$(CC) -o $@ $^ $(LIBS) $(LDFLAGS) $(USRFLAGS) $(INCLUDES)

all: install wifi_daemon wifi
	@cp -f wifi_daemon $(CONFIG_PREFIX)/usr/bin
	@cp -f wifi $(CONFIG_PREFIX)/usr/bin

clean:
	rm -rf wifi_daemon wifi
	rm -rf $(TARGET) *.o *.a *~
