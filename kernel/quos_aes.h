#ifndef _AES_H_
#define _AES_H_
#include "quos_config.h"

#if (SDK_ENABLE_AES == 1)
#define QUOS_AES128 1
/*#define QUOS_AES192 1 */
/*#define QUOS_AES256 1 */

#define QUOS_AES_BLOCKLEN 16 /*Block length in bytes AES is 128b block only */

#if defined(QUOS_AES256) && (QUOS_AES256 == 1)
#define QUOS_AES_KEYLEN 32
#define QUOS_AES_keyExpSize 240
#elif defined(QUOS_AES192) && (QUOS_AES192 == 1)
#define QUOS_AES_KEYLEN 24
#define QUOS_AES_keyExpSize 208
#else
#define QUOS_AES_KEYLEN 16 /* Key length in bytes */
#define QUOS_AES_keyExpSize 176
#endif

typedef struct
{
    quint8_t RoundKey[QUOS_AES_keyExpSize];
    quint8_t Iv[QUOS_AES_BLOCKLEN];
} AES_ctx_t;

void Quos_aesInitCtx(AES_ctx_t *ctx, const char *key);
void Quos_aesInitCtxIv(AES_ctx_t *ctx, const char *key, const char *iv);
void Quos_aesCtxSetIv(AES_ctx_t *ctx, const char *iv);

/* buffer size MUST be mutile of QUOS_AES_BLOCKLEN; */
/* you need only Quos_aesInitCtx as IV is not used in ECB */
/* NB: ECB is considered insecure for most uses */
quint32_t Quos_aesEcbEncrypt(AES_ctx_t *ctx, const quint8_t *buf, quint32_t length);
void Quos_aesEcbDecrypt(AES_ctx_t *ctx, const quint8_t *buf, quint32_t length);

/* buffer size MUST be mutile of QUOS_AES_BLOCKLEN; */
/* Suggest https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme */
/* NOTES: you need to set IV in ctx via Quos_aesInitCtxIv() or Quos_aesCtxSetIv() */
/*        no IV should ever be reused with the same key */
void Quos_aesCbcEncrypt(AES_ctx_t *ctx, void *buf, quint32_t length);
void Quos_aesCbcDecrypt(AES_ctx_t *ctx, void *buf, quint32_t length);

/* Same function for encrypting as for decrypting. */
/* IV is incremented for every block, and used after encryption as XOR-compliment for output */
/* Suggesting https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme */
/* NOTES: you need to set IV in ctx with Quos_aesInitCtxIv() or Quos_aesCtxSetIv() */
/*        no IV should ever be reused with the same key */
void Quos_aesCtrXCrypt(AES_ctx_t *ctx, void *buf, quint32_t length);

void Quos_aesPadding(quint8_t *dest, quint8_t *src, quint32_t srcLen);
quint32_t Quos_aesPaddingBack(quint8_t *src,quint32_t srcLen);
#endif /*_AES_H_ */
#endif