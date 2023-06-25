include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/package.mk
include ../sunxifb.mk

PKG_NAME:=lv_1024
PKG_VERSION:=8.1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

define Package/$(PKG_NAME)
  SECTION:=gui
  SUBMENU:=Littlevgl
  CATEGORY:=Gui
  DEPENDS:=+LVGL8_USE_SUNXIFB_G2D:libuapi +LVGL8_USE_SUNXIFB_G2D:kmod-sunxi-g2d \
           +LVGL8_USE_FREETYPE:libfreetype
  TITLE:=lvgl g2d test use lvgl-8.1.0
endef

PKG_CONFIG_DEPENDS := \
	CONFIG_LVGL8_USE_SUNXIFB_DOUBLE_BUFFER \
	CONFIG_LVGL8_USE_SUNXIFB_CACHE \
	CONFIG_LVGL8_USE_SUNXIFB_G2D \
	CONFIG_LVGL8_USE_SUNXIFB_G2D_ROTATE

define Package/$(PKG_NAME)/config
endef

define Package/$(PKG_NAME)/Default
endef

define Package/$(PKG_NAME)/description
  a lvgl 1024 game using lvgl v8.1.0
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) ./../lvgl $(PKG_BUILD_DIR)/src/
	$(CP) ./../lv_drivers $(PKG_BUILD_DIR)/src/
endef

define Build/Configure
endef

TARGET_CFLAGS+=-I$(PKG_BUILD_DIR)/src

ifeq ($(CONFIG_LVGL8_USE_SUNXIFB_G2D),y)
TARGET_CFLAGS+=-DLV_USE_SUNXIFB_G2D_FILL \
				-DLV_USE_SUNXIFB_G2D_BLEND \
				-DLV_USE_SUNXIFB_G2D_BLIT \
				-DLV_USE_SUNXIFB_G2D_SCALE
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src\
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		-j \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/lv_1024
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/$(PKG_NAME) $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
