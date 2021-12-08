#include "quos_sha256.h"
#include "Quos_kernel.h"
#if (SDK_ENABLE_SHA256 == 1)

static const quint32_t K[] = {
    0x428A2F98,
    0x71374491,
    0xB5C0FBCF,
    0xE9B5DBA5,
    0x3956C25B,
    0x59F111F1,
    0x923F82A4,
    0xAB1C5ED5,
    0xD807AA98,
    0x12835B01,
    0x243185BE,
    0x550C7DC3,
    0x72BE5D74,
    0x80DEB1FE,
    0x9BDC06A7,
    0xC19BF174,
    0xE49B69C1,
    0xEFBE4786,
    0x0FC19DC6,
    0x240CA1CC,
    0x2DE92C6F,
    0x4A7484AA,
    0x5CB0A9DC,
    0x76F988DA,
    0x983E5152,
    0xA831C66D,
    0xB00327C8,
    0xBF597FC7,
    0xC6E00BF3,
    0xD5A79147,
    0x06CA6351,
    0x14292967,
    0x27B70A85,
    0x2E1B2138,
    0x4D2C6DFC,
    0x53380D13,
    0x650A7354,
    0x766A0ABB,
    0x81C2C92E,
    0x92722C85,
    0xA2BFE8A1,
    0xA81A664B,
    0xC24B8B70,
    0xC76C51A3,
    0xD192E819,
    0xD6990624,
    0xF40E3585,
    0x106AA070,
    0x19A4C116,
    0x1E376C08,
    0x2748774C,
    0x34B0BCB5,
    0x391C0CB3,
    0x4ED8AA4A,
    0x5B9CCA4F,
    0x682E6FF3,
    0x748F82EE,
    0x78A5636F,
    0x84C87814,
    0x8CC70208,
    0x90BEFFFA,
    0xA4506CEB,
    0xBEF9A3F7,
    0xC67178F2,
};

static const quint8_t sha256_padding[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define SHR(x, n) ((x & 0xFFFFFFFF) >> n)
#define ROTR(x, n) (SHR(x, n) | (x << (32 - n)))

#define S0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define S1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

#define S2(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S3(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))

#define F0(x, y, z) ((x & y) | (z & (x | y)))
#define F1(x, y, z) (z ^ (x & (y ^ z)))

#define R(t) W[t] = S1(W[t - 2]) + W[t - 7] + S0(W[t - 15]) + W[t - 16]

#define P(a, b, c, d, e, f, g, h, x, K)                   \
    do                                                    \
    {                                                     \
        quint32_t temp = h + S3(e) + F1(e, f, g) + K + x; \
        d += temp;                                        \
        h = temp + S2(a) + F0(a, b, c);                   \
    } while (0)

static void quos_sha256process(SHA256_ctx_t *ctx, const quint8_t data[64])
{
    quint32_t temp, W[64];
    quint32_t A[8];
    quint32_t i;

    for (i = 0; i < 8; i++)
    {
        A[i] = ctx->state[i];
    }

    for (i = 0; i < 64; i++)
    {
        if (i < 16)
        {
            W[i] = _ARRAY0123_U32(data + 4 * i);
        }
        else
        {
            R(i);
        }

        P(A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7], W[i], K[i]);

        temp = A[7];
        A[7] = A[6];
        A[6] = A[5];
        A[5] = A[4];
        A[4] = A[3];
        A[3] = A[2];
        A[2] = A[1];
        A[1] = A[0];
        A[0] = temp;
    }
    for (i = 0; i < 8; i++)
    {
        ctx->state[i] += A[i];
    }
}

void Quos_sha256init(SHA256_ctx_t *ctx)
{
    HAL_MEMSET(ctx, 0, sizeof(SHA256_ctx_t));
    ctx->total[0] = 0;
    ctx->total[1] = 0;
    ctx->state[0] = 0x6A09E667;
    ctx->state[1] = 0xBB67AE85;
    ctx->state[2] = 0x3C6EF372;
    ctx->state[3] = 0xA54FF53A;
    ctx->state[4] = 0x510E527F;
    ctx->state[5] = 0x9B05688C;
    ctx->state[6] = 0x1F83D9AB;
    ctx->state[7] = 0x5BE0CD19;
    ctx->is224 = FALSE;
}

void Quos_sha256update(SHA256_ctx_t *ctx, const quint8_t *input, quint32_t ilen)
{
    quint32_t fill;
    quint32_t left;

    if (ilen == 0)
    {
        return;
    }

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += (quint32_t)ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if (ctx->total[0] < (quint32_t)ilen)
    {
        ctx->total[1]++;
    }

    if (left && ilen >= fill)
    {
        HAL_MEMCPY((void *)(ctx->buffer + left), input, fill);
        quos_sha256process(ctx, ctx->buffer);
        input += fill;
        ilen -= fill;
        left = 0;
    }

    while (ilen >= 64)
    {
        quos_sha256process(ctx, input);
        input += 64;
        ilen -= 64;
    }

    if (ilen > 0)
    {
        HAL_MEMCPY((void *)(ctx->buffer + left), input, ilen);
    }
}

void Quos_sha256key(SHA256_ctx_t *ctx, quint8_t *key, quint8_t key_len)
{
    quint8_t k_ipad[QUOS_SHA256_KEY_IOPAD_SIZE];
    key_len = key_len > QUOS_SHA256_KEY_IOPAD_SIZE ? QUOS_SHA256_KEY_IOPAD_SIZE : key_len;
    HAL_MEMSET(ctx->k_opad, 0, sizeof(ctx->k_opad));
    HAL_MEMCPY(ctx->k_opad, key, key_len);
    HAL_MEMSET(k_ipad, 0, sizeof(k_ipad));
    HAL_MEMCPY(k_ipad, key, key_len);
    quint8_t i = 0;
    for (i = 0; i < QUOS_SHA256_KEY_IOPAD_SIZE; i++)
    {
        k_ipad[i] ^= 0x36;
        ctx->k_opad[i] ^= 0x5c;
    }
    ctx->isHmac = TRUE;
    Quos_sha256update(ctx, k_ipad, QUOS_SHA256_KEY_IOPAD_SIZE);
}

void Quos_sha256finish(SHA256_ctx_t *ctx, quint8_t output[QUOS_SHA256_DIGEST_LENGTH])
{
    quint32_t last, padn;
    quint32_t high, low;
    quint8_t msglen[8];

    high = (ctx->total[0] >> 29) | (ctx->total[1] << 3);
    low = (ctx->total[0] << 3);

    _U32_ARRAY0123(high, msglen);
    _U32_ARRAY0123(low, msglen + 4);

    last = ctx->total[0] & 0x3F;
    padn = (last < 56) ? (56 - last) : (120 - last);

    Quos_sha256update(ctx, sha256_padding, padn);
    Quos_sha256update(ctx, msglen, 8);

    _U32_ARRAY0123(ctx->state[0], output);
    _U32_ARRAY0123(ctx->state[1], output + 4);
    _U32_ARRAY0123(ctx->state[2], output + 8);
    _U32_ARRAY0123(ctx->state[3], output + 12);
    _U32_ARRAY0123(ctx->state[4], output + 16);
    _U32_ARRAY0123(ctx->state[5], output + 20);
    _U32_ARRAY0123(ctx->state[6], output + 24);

    if (FALSE == ctx->is224)
    {
        _U32_ARRAY0123(ctx->state[7], output + 28);
    }

    if (ctx->isHmac)
    {
        SHA256_ctx_t context;
        Quos_sha256init(&context);                                            /* init context for 2nd pass */
        Quos_sha256update(&context, ctx->k_opad, QUOS_SHA256_KEY_IOPAD_SIZE); /* start with outer pad */
        Quos_sha256update(&context, output, QUOS_SHA256_DIGEST_LENGTH);       /* then results of 1st hash */
        Quos_sha256finish(&context, output);                                  /* finish up 2nd pass */
    }
}

#endif