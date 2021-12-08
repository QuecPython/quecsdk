#include "quos_sha1.h"
#include "Quos_kernel.h"

#if (SDK_ENABLE_SHA1 == 1)
static const quint32_t K[] = {
    0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};

static const quint8_t sha1_padding[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define S(x, n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define R(t) \
    (temp = W[(t - 3) & 0x0F] ^ W[(t - 8) & 0x0F] ^ W[(t - 14) & 0x0F] ^ W[t & 0x0F], (W[t & 0x0F] = S(temp, 1)))

#define P(a, b, c, d, e, x, K)             \
    do                                     \
    {                                      \
        e += S(a, 5) + F(b, c, d) + K + x; \
        b = S(b, 30);                      \
    } while (0)

static void quos_sha1process(SHA1_ctx_t *ctx, const quint8_t data[64])
{
    quint32_t temp, W[16];
    quint32_t A[5];
    quint32_t i;
    for (i = 0; i < 5; i++)
    {
        A[i] = ctx->state[i];
    }
    for (i = 0; i < 16; i++)
    {
        W[i] = _ARRAY0123_U32(data + 4 * i);
    }

#define F(x, y, z) (z ^ (x & (y ^ z)))

    for (i = 0; i < 16; i++)
    {
        P(A[0], A[1], A[2], A[3], A[4], W[i], K[i / 20]);
        temp = A[4];
        A[4] = A[3];
        A[3] = A[2];
        A[2] = A[1];
        A[1] = A[0];
        A[0] = temp;
    }
    for (i = 16; i < 20; i++)
    {
        P(A[0], A[1], A[2], A[3], A[4], R(i), K[i / 20]);
        temp = A[4];
        A[4] = A[3];
        A[3] = A[2];
        A[2] = A[1];
        A[1] = A[0];
        A[0] = temp;
    }
#undef F

#define F(x, y, z) (x ^ y ^ z)

    for (i = 20; i < 40; i++)
    {
        P(A[0], A[1], A[2], A[3], A[4], R(i), K[i / 20]);
        temp = A[4];
        A[4] = A[3];
        A[3] = A[2];
        A[2] = A[1];
        A[1] = A[0];
        A[0] = temp;
    }

#undef F

#define F(x, y, z) ((x & y) | (z & (x | y)))

    for (i = 40; i < 60; i++)
    {
        P(A[0], A[1], A[2], A[3], A[4], R(i), K[i / 20]);
        temp = A[4];
        A[4] = A[3];
        A[3] = A[2];
        A[2] = A[1];
        A[1] = A[0];
        A[0] = temp;
    }

#undef F

#define F(x, y, z) (x ^ y ^ z)
    for (i = 60; i < 80; i++)
    {
        P(A[0], A[1], A[2], A[3], A[4], R(i), K[i / 20]);
        temp = A[4];
        A[4] = A[3];
        A[3] = A[2];
        A[2] = A[1];
        A[1] = A[0];
        A[0] = temp;
    }
#undef F
    for (i = 0; i < 5; i++)
    {
        ctx->state[i] += A[i];
    }
}

void Quos_sha1init(SHA1_ctx_t *ctx)
{
    HAL_MEMSET(ctx, 0, sizeof(SHA1_ctx_t));
    ctx->total[0] = 0;
    ctx->total[1] = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
}
void Quos_sha1update(SHA1_ctx_t *ctx, const quint8_t *input, quint32_t ilen)
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
        quos_sha1process(ctx, ctx->buffer);
        input += fill;
        ilen -= fill;
        left = 0;
    }

    while (ilen >= 64)
    {
        quos_sha1process(ctx, input);
        input += 64;
        ilen -= 64;
    }

    if (ilen > 0)
    {
        HAL_MEMCPY((void *)(ctx->buffer + left), input, ilen);
    }
}
void Quos_sha1key(SHA1_ctx_t *ctx, quint8_t *key, quint8_t key_len)
{
    
    quint8_t k_ipad[QUOS_SHA1_KEY_IOPAD_SIZE];
    key_len = key_len > QUOS_SHA1_KEY_IOPAD_SIZE ? QUOS_SHA1_KEY_IOPAD_SIZE : key_len;
    HAL_MEMSET(ctx->k_opad, 0, sizeof(ctx->k_opad));
    HAL_MEMCPY(ctx->k_opad, key, key_len);
    HAL_MEMSET(k_ipad, 0, sizeof(k_ipad));
    HAL_MEMCPY(k_ipad, key, key_len);
    quint8_t i = 0;
    for (i = 0; i < QUOS_SHA1_KEY_IOPAD_SIZE; i++)
    {
        k_ipad[i] ^= 0x36;
        ctx->k_opad[i] ^= 0x5c;
    }
    ctx->isHmac = TRUE;
    Quos_sha1update(ctx, k_ipad, QUOS_SHA1_KEY_IOPAD_SIZE);
}
void Quos_sha1finish(SHA1_ctx_t *ctx, quint8_t output[QUOS_SHA1_DIGEST_LENGTH])
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

    Quos_sha1update(ctx, sha1_padding, padn);
    Quos_sha1update(ctx, msglen, 8);

    _U32_ARRAY0123(ctx->state[0], output);
    _U32_ARRAY0123(ctx->state[1], output + 4);
    _U32_ARRAY0123(ctx->state[2], output + 8);
    _U32_ARRAY0123(ctx->state[3], output + 12);
    _U32_ARRAY0123(ctx->state[4], output + 16);
    if (ctx->isHmac)
    {
        SHA1_ctx_t context;
        Quos_sha1init(&context);                                     /* init context for 2nd pass */
        Quos_sha1update(&context, ctx->k_opad, QUOS_SHA1_KEY_IOPAD_SIZE); /* start with outer pad */
        Quos_sha1update(&context, output, QUOS_SHA1_DIGEST_LENGTH);       /* then results of 1st hash */
        Quos_sha1finish(&context, output);                           /* finish up 2nd pass */
    }
}

#endif