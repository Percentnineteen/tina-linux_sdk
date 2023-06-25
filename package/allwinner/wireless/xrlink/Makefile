include $(TOPDIR)/rules.mk

PKG_NAME:=xrlink
PKG_VERSION:=0.1
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  SUBMENU:=wireless
  TITLE:=xrlink
endef

define Package/$(PKG_NAME)
  $(call Package/xrlink/Default)
  MENU:=1
  DEFAULT:=1
endef

define Package/$(PKG_NAME)-demo
  $(call Package/xrlink/Default)
  TITLE:=xrlink app
  DEPENDS := xrlink
endef

define Package/$(PKG_NAME)/description
	Tina xrlink manager
endef

define Package/$(PKG_NAME)-demo/description
	Tina xrlink app demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

TARGET_CFLAGS += $(if $(CONFIG_USE_GLIBC),-pthread)
TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread -lrt -ldl)

define Build/Compile/$(PKG_NAME)
	$(MAKE) -C $(PKG_BUILD_DIR)/src/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

ifeq ($(CONFIG_PACKAGE_xrlink-demo),y)
define Build/Compile/$(PKG_NAME)-demo
	$(MAKE) -C $(PKG_BUILD_DIR)/demo/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef
endif

define Build/Compile
	$(Build/Compile/$(PKG_NAME))
	$(Build/Compile/$(PKG_NAME)-demo)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libxrlink.so $(1)/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libxrlink.so $(1)/lib
endef

define Package/$(PKG_NAME)-demo/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_daemon $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi $(1)/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))
