Target = tplayerdemo

SourceIncludePath := -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
CompileFlags = $(CFLAGS) $(SourceIncludePath)

ifeq ($(ONLY_ENABLE_AUDIO),y)
CompileFlags += -DONLY_ENABLE_AUDIO
endif

LoadFlags +=  -ltplayer -lcdx_base -lrt -lstdc++

$(Target): tplayerdemo.c
	$(CC) -o $@ $^ $(CompileFlags) $(LDFLAGS) $(LoadFlags)
