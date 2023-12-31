include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/package.mk

target = libwifimg-v2.0.so

CONFIG_WMG_SUPPORT_STA_MODE=y
CONFIG_WMG_SUPPORT_AP_MODE=y
CONFIG_WMG_SUPPORT_MONITOR_MODE=y

INCLUDES        += -I ./include
INCLUDES        += -I ./include/os
INCLUDES        += -I ./include/os/linux
INCLUDES        += -I src/os/linux/linkd
INCLUDES        += -I $(CONFIG_PREFIX)/usr/include
INCLUDES        += -I $(STAGING_DIR)/usr/include/libnl-tiny
LIBS            += -lpthread -ldl -lrt -lnl-tiny -lwirelesscom

SRCS += src/wifimg.c
SRCS += src/wmg_common.c
SRCS += src/log/wifi_log.c
SRCS += src/os/linux/linkd/linkd.c

ifeq ($(CONFIG_WMG_SUPPORT_STA_MODE), y)
SRCS += src/wmg_sta.c
SRCS += src/os/linux/udhcpc.c
SRCS += src/os/linux/event.c
SRCS += src/os/linux/linux_wpa.c
SRCS += src/os/linux/linux_common.c
SRCS += src/os/linux/scan.c
SRCS += src/os/linux/utils.c
endif
ifeq ($(CONFIG_WMG_SUPPORT_AP_MODE), y)
SRCS += src/wmg_ap.c
SRCS += src/os/linux/linux_hapd.c
endif
ifeq ($(CONFIG_WMG_SUPPORT_MONITOR_MODE), y)
SRCS += src/wmg_monitor.c
SRCS += src/os/linux/linux_nl.c
endif

ifdef CONFIG_WMG_PROTOCOL_SOFTAP
SRCS += src/os/linux/linkd/linkd_softap/linkd_softap.c
endif

ifdef CONFIG_WMG_PROTOCOL_BLE
SRCS += src/os/linux/linkd/linkd_ble/linkd_ble.c
LIBS += -lbtmg
LIBS += -lbluetooth
LIBS += -lbluetooth-internal
LIBS += -lglib-2.0 -lsbc -ljson-c -lgio-2.0 -lgobject-2.0 -lasound -lbtmg -ldl -lm -lresolv
LIBS += -lshared-mainloop
LIBS += -ldbus-1
endif

ifdef CONFIG_WMG_PROTOCOL_XCONFIG
INCLUDES += -I src/os/linux/linkd/lib/lib_xconfig
SRCS += src/os/linux/linkd/linkd_xconfig/linkd_xconfig.c
LIBS += -lcrypto
ifeq ($(CONFIG_ARCH), "aarch64")
    SRCS += src/os/linux/linkd/lib/lib_xconfig/libdecode_64.a
else
    ifeq ($(TARGET_ARCH), "riscv")
        SRCS += src/os/linux/linkd/lib/lib_xconfig/libdecode_riscv.a
    else
        SRCS += src/os/linux/linkd/lib/lib_xconfig/libdecode.a
    endif
endif
endif

ifdef CONFIG_WMG_PROTOCOL_SOUNDWAVE
INCLUDES += -I src/os/linux/linkd/lib/lib_soundwave
LIBS += -lasound -luci
SRCS += src/os/linux/linkd/linkd_soundwave/linkd_soundwave.c
ifeq ($(CONFIG_LIBC), "glibc")
    LIBS += -lm
ifeq ($(CONFIG_ARCH), "aarch64")
    ADTLIB=src/os/linux/linkd/lib/lib_soundwave/glibc/aarch64/libADT.a
else
    ifeq ($(TARGET_ARCH), "riscv")
        ADTLIB=src/os/linux/linkd/lib/lib_soundwave/glibc/riscv/libADT.a
    else
        ADTLIB=src/os/linux/linkd/lib/lib_soundwave/glibc/arm/libADT.a
    endif
endif
else
ifeq ($(CONFIG_ARCH), "aarch64")
    ADTLIB=src/os/linux/linkd/lib/lib_soundwave/musl/aarch64/libADT.a
else
    ifeq ($(TARGET_ARCH), "riscv")
        ADTLIB=src/os/linux/linkd/lib/lib_soundwave/musl/riscv/libADT.a
    else
        ADTLIB=src/os/linux/linkd/lib/lib_soundwave/musl/arm/libADT.a
    endif
endif
endif
endif

LOCAL_CFLAGS = -D_GNU_SOURCE=1
LOCAL_CFLAGS = -DOS_NET_LINUX_OS

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp ./include/*.h $(CONFIG_PREFIX)/usr/include

#libwifimg-v2.0.so
#ifdef CONFIG_LINUX_5_4 || CONFIG_LINUX_4_9
ifeq ($(CONFIG_ARCH), "aarch64")
    SRCS += ./src/os/linux/lib/libwpa_client_64.a
else
    ifeq ($(CONFIG_SOFT_FLOAT), y)
        SRCS += ./src/os/linux/lib/libwpa_client_softft.a
    else
         ifeq ($(TARGET_ARCH), riscv)
              SRCS += ./src/os/linux/lib/libwpa_client_riscv.a
         else
              SRCS += ./src/os/linux/lib/libwpa_client.a
         endif
    endif
endif
#endif
$(target): $(SRCS) $(ADTLIB)
	$(CC) -fPIC -shared $(INCLUDES) $(LOCAL_CFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)
	@cp $@ $(CONFIG_PREFIX)/usr/lib

all: install $(target)
clean:
	rm -rf $(target)
