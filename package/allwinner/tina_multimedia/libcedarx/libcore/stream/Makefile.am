SUBDIRS =  udp tcp

if FILE_STREAM_ENABLE
SUBDIRS += file
endif

if HTTP_STREAM_ENABLE
SUBDIRS += http
endif

if HTTPS_STREAM_ENABLE
SUBDIRS += ssl aes
endif

if RTSP_STREAM_ENABLE
SUBDIRS += rtsp
endif

if RTMP_STREAM_ENABLE
SUBDIRS += rtmp
endif

SUBDIRS += base
