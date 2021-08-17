NAME := QUECSDK

#include config/$(KCONFIG_CONFIG)

#ifeq ($(CONFIG_QUECTHING), y)

#ifeq ($(CONFIG_MBEDTLS), y)

$(NAME)_DEFINE := CLOUD_ENABLE_QUEC_MQTT ENABLE_QUEC_PYTHON

#编译平台为ASR
ifeq ($(strip $(PLAT)),ASR)
$(NAME)_ARCHIVES := platform/ASR_lib/lib/libquecsdk_in.a
else
#编译平台为展锐
ifeq ($(strip $(PLAT)),Unisoc)
$(NAME)_ARCHIVES := platform/Unisoc_lib/lib/libquecsdk_in.a
else
$(error current platform does not support quecIot)
endif
endif

$(NAME)_SRCS += \
	driverLayer/qhal_Dev.c \
	driverLayer/qhal_FlashOpt.c \
	driverLayer/qhal_property.c \
	driverLayer/qhal_Socket.c \
	driverLayer/qhal_patch.c \
	thirdLib/mqtt/MQTTConnectClient.c \
	thirdLib/mqtt/MQTTConnectServer.c \
	thirdLib/mqtt/MQTTDeserializePublish.c \
	thirdLib/mqtt/MQTTFormat.c \
	thirdLib/mqtt/MQTTPacket.c \
	thirdLib/mqtt/MQTTSerializePublish.c \
	thirdLib/mqtt/MQTTSubscribeClient.c \
	thirdLib/mqtt/MQTTSubscribeServer.c \
	thirdLib/mqtt/MQTTUnsubscribeClient.c \
	thirdLib/mqtt/MQTTUnsubscribeServer.c

GLOBAL_INCS += \
	driverLayer \
	thirdLib/mqtt \
	thirdLib/cJSON \
	cloud \
	kernel

#endif
#endif


$(NAME)_COMPONENTS = system 


