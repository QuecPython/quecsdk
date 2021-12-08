#ifndef __QUOS_KERNEL_H__
#define __QUOS_KERNEL_H__
#include "quos_log.h"
#include "quos_signal.h"
#include "quos_event.h"
#include "quos_SupportTool.h"
#include "quos_swTimer.h"
#include "quos_sysTick.h"
#include "quos_twll.h"
#include "quos_aes.h"
#include "quos_md5.h"
#include "quos_sha1.h"
#include "quos_sha256.h"
#include "quos_base64.h"
#include "quos_dataStore.h"
#include "quos_socket.h"
#include "quos_http.h"
#include "quos_mqtt.h"
#include "quos_coap.h"
#include "quos_lwm2m.h"
#include "quos_fifo.h"
#include "quos_net.h"
#include "quos_cjson.h"

void Quos_kernelInit(void);
quint32_t Quos_kernelTask(void);
void Quos_kernelResume(void);
#endif