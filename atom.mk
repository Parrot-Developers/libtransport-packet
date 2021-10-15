
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libtransport-packet
LOCAL_CATEGORY_PATH := libs
LOCAL_DESCRIPTION := Transport packet library
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_CFLAGS := -DTPKT_API_EXPORTS -fvisibility=hidden -std=gnu99
LOCAL_SRC_FILES := \
	src/tpkt.c \
	src/tpkt_list.c
LOCAL_LIBRARIES := \
	libfutils \
	libpomp \
	libulog

include $(BUILD_LIBRARY)
