
include $(top_srcdir)/Makefile.inc

noinst_LTLIBRARIES = libcdx_mp4_muxer.la

## set the source files.
libcdx_mp4_muxer_la_SOURCES =  CdxMp4Muxer.c \
			CdxMp4MuxerLib.c \
			ByteIOContext.c \
			CdxHevc.c \
			CdxHevcMem.c \
			CdxHevcIntMath.c


libcdx_mp4_muxer_la_CFLAGS = $(CFLAGS_CDXG)
LOCAL_INCLUDE = -I../include \
		-I../../include \
		-I./include \
		-I../../base/include \
		-I../base \
                -I$(top_srcdir)/external/include/aencoder \
		-I$(top_srcdir)/../mpp_libcedarc/include \
                -I$(top_srcdir)/../libcedarc/include \
		-I$(top_srcdir)/

libcdx_mp4_muxer_la_CFLAGS += $(LOCAL_INCLUDE) -DHAVE_AW_CONFIG_H


libcdx_mp4_muxer_la_LDFLAGS = $(LDFLAGS_CDXG)

libcdx_mp4_muxer_la_LIBADD = $(top_srcdir)/libcore/base/libcdx_base.la

#LOCAL_LIB = -lpthread
libcdx_mp4_muxer_la_LDFLAGS += $(LOCAL_LIB)
