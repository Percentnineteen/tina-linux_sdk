include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/package.mk

INCLUDES += -I $(CONFIG_PREFIX)/usr/include \
			-I $(STAGING_DIR)/usr/include \
			-I ../core/include \
			-I ../core/include/linux \
			-I ./linkd/lib \
			-I ./linkd/ \

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin

DEAMON_LIBS += -lwifimg-v2.0
DEAMON_LIBS += -lwirelesscom

DEAMON_SRCS += wifi_deamon.c

wifi_deamon: $(DEAMON_SRCS)
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -L$(CONFIG_PREFIX)/lib $(DEAMON_LIBS)
	@cp -f wifi_deamon $(CONFIG_PREFIX)/usr/bin
wifi: wifi.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg-v2.0 -lwirelesscom
	@cp -f wifi $(CONFIG_PREFIX)/usr/bin

all: install wifi_deamon wifi

clean:
	rm -rf wifi_deamon wifi
