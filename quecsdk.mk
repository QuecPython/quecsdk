NAME := QUECSDK

#include config/$(KCONFIG_CONFIG)

#ifeq ($(CONFIG_QUECTHING), y)

#ifeq ($(CONFIG_MBEDTLS), y)

$(NAME)_DEFINE := CLOUD_ENABLE_QUEC_MQTT ENABLE_QUEC_PYTHON


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
	thirdLib/mqtt/MQTTUnsubscribeServer.c \
	cloud/common/ql_iotCmdBus.c \
	cloud/common/ql_iotCmdLan.c \
	cloud/common/ql_iotCmdLoc.c \
	cloud/common/ql_iotCmdOTA.c \
	cloud/common/ql_iotCmdSys.c \
	cloud/common/ql_iotConfig.c \
	cloud/common/ql_iotConn.c \
	cloud/common/ql_iotDp.c \
	cloud/common/ql_iotConn.c \
	cloud/common/ql_iotSecure.c \
	cloud/common/ql_iotTtlv.c \
	kernel/quos_aes.c \
	kernel/quos_base64.c \
	kernel/quos_cjson.c \
	kernel/quos_coap.c \
	kernel/quos_dataStore.c \
	kernel/quos_event.c \
	kernel/quos_fifo.c \
	kernel/quos_http.c \
	kernel/Quos_kernel.c \
	kernel/quos_log.c \
	kernel/quos_lwm2m.c \
	kernel/quos_md5.c \
	kernel/quos_mqtt.c \
	kernel/quos_net.c \
	kernel/quos_sha1.c \
	kernel/quos_sha256.c \
	kernel/quos_signal.c \
	kernel/quos_socket.c \
	kernel/quos_SupportTool.c \
	kernel/quos_swTimer.c \
	kernel/quos_sysTick.c \
	kernel/quos_twll.c \


GLOBAL_INCS += \
	driverLayer \
	thirdLib/mqtt \
	thirdLib/cJSON \
	cloud \
	cloud/common \
	kernel

#endif
#endif


$(NAME)_COMPONENTS = system 


