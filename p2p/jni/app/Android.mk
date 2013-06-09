# Android.mk

# center
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_MODULE := center
LOCAL_SRC_FILES := \
		$(wildcard ./common/*.c)	\
		./server/server.c		

#	LOCAL_LDLIBS := -lpthread

include $(BUILD_EXECUTABLE)

# pc_client
#LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_MODULE := pc_client
LOCAL_SRC_FILES := \
		$(wildcard ./common/*.c)	\
		./pc/pc.c		

#	LOCAL_LDLIBS := -lpthread

include $(BUILD_EXECUTABLE)

# ipc_client
#LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_MODULE := ipc_client
LOCAL_SRC_FILES := \
		$(wildcard ./common/*.c)	\
		./ipc/ipc.c

#	LOCAL_LDLIBS := -lpthread

include $(BUILD_EXECUTABLE)
