Target = camerapreviewerdemo
## set the include path for compile flags.
SourceIncludePath := -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
CompileFlags = $(CFLAGS) $(SourceIncludePath)
LoadFlags += -ltrecorder -lawrecorder -lvencoder -laencoder \
               -lcdx_base -lcdx_muxer -lcdc_base -lVE \
               -lMemAdapter  -lpthread -luapi -ldl -lrt -lz -lasound -lvdecoder -lm

$(Target): camerapreviewer.c 
	$(CC) -o $@ $^ $(CompileFlags) $(LDFLAGS) $(LoadFlags)
