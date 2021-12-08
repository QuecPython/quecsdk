#ifndef __QIOT_SECURE_H__
#define __QIOT_SECURE_H__
#include "Ql_iotApi.h"

char *Ql_iotSecureVerGet(void);
char *Ql_iotSecureGenMqttConnData(const char *ps, const char *ds);
char *Ql_iotSecureGenMqttAuthData(const char *pk, const char *ps, const char *dk);
char *Ql_iotSecureGenCoapConnEndpoint(const char *pk, const char *ps, const char *dk, const char *ds);
char *Ql_iotSecureGenCoapAuthPayload(const char *pk, const char *ps, const char *dk);
char *Ql_iotSecureDecodeDs(const char *ps, const quint8_t *encryData, quint32_t len);
quint32_t Ql_iotSecureEncryptPayload(quint8_t *data, quint32_t len, uint8_t **outData, const char *key, const char *iv);
quint32_t Ql_iotSecureDecryptPayload(quint8_t *data, quint32_t len, const char *key, const char *iv);
char *ql_iotSecureGenAuthKey(const char *pk, const char *ps, const char *dk, const char *random);
char *ql_iotSecureGenConnSign(const char *ps, const char *ds, const char *random);

char *Ql_iotSecureDecodeSessionKey(const char *ds, const quint8_t *encryData, quint32_t len);
#endif