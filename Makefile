LOCAL_ARCH := $(shell uname --m)
BUILD_TARGET := linux-$(LOCAL_ARCH)

PROJECT_PATH = $(shell pwd)
DEBUG := true
#=============================================================================

LIBEV_PATH = /home/wutings/part1/workspace-neon/libev-4.22/libs-build/$(BUILD_TARGET)

INCLUDES += -I$(LIBEV_PATH)/include

INCLUDES += -Iux/include

ifeq ($(DEBUG),true)
MY_DEF := -O0 -g -DDEBUG -Wno-comment 
else
MY_DEF := -O2 -Wno-comment 
endif

MY_DEF += 

EXT_LIBS := $(LIBEV_PATH)/lib/libev.a 


#=============================================================================
COMPILE_OPTS = -I. -I./include $(INCLUDES) -Wall $(MY_DEF) 
C =			c
C_COMPILER =$(CROSS_COMPILE)gcc
C_FLAGS =	$(COMPILE_OPTS) 
CPP =		cpp
CPLUSPLUS_COMPILER =$(CROSS_COMPILE)g++
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -std=gnu++11 -fmessage-length=0 -fpermissive 
OBJ =			o
LINK =			$(CROSS_COMPILE)g++ -o 
LINK_OPTS =		-L.
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		$(CROSS_COMPILE)ar cr 
LIBRARY_LINK_OPTS =	
LIB_SUFFIX =		a
LIBS_FOR_CONSOLE_APPLICATION = $(EXT_LIBS) -pthread -ldl -lrt 
LIBS_FOR_GUI_APPLICATION =
EXE =
##### End of variables to change

SRC_PATH := ./src

AWE_NAME_STATIC = libawe
LIBAWE_STATIC_LIB = $(AWE_NAME_STATIC).$(LIB_SUFFIX)


AWE_APP := test-log

AWE_OBJS := $(SRC_PATH)/buffer.o \
	$(SRC_PATH)/connection_impl.o \
	$(SRC_PATH)/connection_ux_impl.o \
	$(SRC_PATH)/connection.o \
	$(SRC_PATH)/connector.o \
	$(SRC_PATH)/hexdump.o \
	$(SRC_PATH)/linkedlist.o \
	$(SRC_PATH)/log.o \
	$(SRC_PATH)/looper.o \
	$(SRC_PATH)/mem.o \
	$(SRC_PATH)/packet.o \
	$(SRC_PATH)/pool.o \
	$(SRC_PATH)/ringarray.o \
	$(SRC_PATH)/ringbuffer.o \
	$(SRC_PATH)/scheduler.o \
	$(SRC_PATH)/socket_helper.o \
	$(SRC_PATH)/socketpair.o \
	$(SRC_PATH)/time.o \
	$(SRC_PATH)/timer_task.o \
	$(SRC_PATH)/uxscheduler.o \


AWE_OBJS += $(SRC_PATH)/unix/thread.o \
	$(SRC_PATH)/unix/thread_cond.o \
	$(SRC_PATH)/unix/thread_mutex.o \
	$(SRC_PATH)/unix/thread_rwlock.o


AWE_OBJS += ./ux/src/ux_dummy.o

APP_OBJS += test/test-log.o



LOCAL_LIBS	:= $(LIBAWE_STATIC_LIB)
LIBS		:= $(LOCAL_LIBS) $(LIBS_FOR_CONSOLE_APPLICATION)

ALL = $(LIBAWE_STATIC_LIB) $(AWE_APP) 

all: $(ALL)

.$(C).$(OBJ):
	$(C_COMPILER) -c $(C_FLAGS) $< -o $@ 

.$(CPP).$(OBJ):
	$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $< -o $@ 

$(LIBAWE_STATIC_LIB): $(AWE_OBJS)
	$(LIBRARY_LINK) $@ $(LIBRARY_LINK_OPTS) $(AWE_OBJS)

$(AWE_APP)$(EXE):$(APP_OBJS) $(LIBAWE_STATIC_LIB)
	$(LINK) $@ $(CONSOLE_LINK_OPTS) $(APP_OBJS) $(LIBS)

clean:
	find . -name "*.o" | xargs rm -f
	rm -rf $(ALL)
