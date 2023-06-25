$(call inherit-product-if-exists, target/allwinner/d1-h-common/d1-h-common.mk)

PRODUCT_PACKAGES +=

PRODUCT_COPY_FILES +=

PRODUCT_AAPT_CONFIG := large xlarge hdpi xhdpi
PRODUCT_AAPT_PERF_CONFIG := xhdpi
PRODUCT_CHARACTERISTICS := musicbox

PRODUCT_BRAND := allwinner
PRODUCT_NAME := d1-h_nezha_min
PRODUCT_DEVICE := d1-h-nezha_min
PRODUCT_MODEL := Allwinner d1-h dev board min system
