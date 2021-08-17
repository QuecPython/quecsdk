#ifndef _QUOS_SHA1_H__
#define _QUOS_SHA1_H__
#include "quos_config.h"
#if (SDK_ENABLE_SHA1 == 1)
#define QUOS_SHA1_DIGEST_LENGTH (20)
#define QUOS_SHA1_KEY_IOPAD_SIZE (64)


typedef struct
{
    quint32_t total[2];  /*!< number of bytes processed  */
    quint32_t state[5];  /*!< intermediate digest state  */
    quint8_t buffer[64]; /*!< data block being processed */
    quint8_t k_opad[QUOS_SHA1_KEY_IOPAD_SIZE];
    qbool isHmac;
} SHA1_ctx_t;

void Quos_sha1init(SHA1_ctx_t *ctx);
void Quos_sha1key(SHA1_ctx_t *ctx, quint8_t *key, quint8_t key_len);
void Quos_sha1update(SHA1_ctx_t *ctx, const quint8_t *input, quint32_t ilen);
void Quos_sha1finish(SHA1_ctx_t *ctx, quint8_t output[QUOS_SHA1_DIGEST_LENGTH]);
#endif
#endif
