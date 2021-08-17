#ifndef _QUOS_SHA256_H__
#define _QUOS_SHA256_H__
#include "quos_config.h"
#if (SDK_ENABLE_SHA256 == 1)
#define QUOS_SHA256_DIGEST_LENGTH (32)
#define QUOS_SHA256_KEY_IOPAD_SIZE (64)

/**
 * \brief          SHA-256 context structure
 */
typedef struct
{
    quint32_t total[2];  /*!< number of bytes processed  */
    quint32_t state[8];  /*!< intermediate digest state  */
    quint8_t buffer[64]; /*!< data block being processed */
    qbool is224;         /*!< 0 => SHA-256, else SHA-224 */
    quint8_t k_opad[QUOS_SHA256_KEY_IOPAD_SIZE];
    qbool isHmac;
} SHA256_ctx_t;

void Quos_sha256init(SHA256_ctx_t *ctx);
void Quos_sha256key(SHA256_ctx_t *ctx, quint8_t *key, quint8_t key_len);
void Quos_sha256update(SHA256_ctx_t *ctx, const quint8_t *input, quint32_t ilen);
void Quos_sha256finish(SHA256_ctx_t *ctx, quint8_t output[QUOS_SHA256_DIGEST_LENGTH]);
#endif
#endif