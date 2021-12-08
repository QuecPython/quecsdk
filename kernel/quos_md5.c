/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : MD5算法
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_md5.h"
#if (SDK_ENABLE_MD5 == 1)
#define T1 0xd76aa478
#define T2 0xe8c7b756
#define T3 0x242070db
#define T4 0xc1bdceee
#define T5 0xf57c0faf
#define T6 0x4787c62a
#define T7 0xa8304613
#define T8 0xfd469501
#define T9 0x698098d8
#define T10 0x8b44f7af
#define T11 0xffff5bb1
#define T12 0x895cd7be
#define T13 0x6b901122
#define T14 0xfd987193
#define T15 0xa679438e
#define T16 0x49b40821
#define T17 0xf61e2562
#define T18 0xc040b340
#define T19 0x265e5a51
#define T20 0xe9b6c7aa
#define T21 0xd62f105d
#define T22 0x02441453
#define T23 0xd8a1e681
#define T24 0xe7d3fbc8
#define T25 0x21e1cde6
#define T26 0xc33707d6
#define T27 0xf4d50d87
#define T28 0x455a14ed
#define T29 0xa9e3e905
#define T30 0xfcefa3f8
#define T31 0x676f02d9
#define T32 0x8d2a4c8a
#define T33 0xfffa3942
#define T34 0x8771f681
#define T35 0x6d9d6122
#define T36 0xfde5380c
#define T37 0xa4beea44
#define T38 0x4bdecfa9
#define T39 0xf6bb4b60
#define T40 0xbebfbc70
#define T41 0x289b7ec6
#define T42 0xeaa127fa
#define T43 0xd4ef3085
#define T44 0x04881d05
#define T45 0xd9d4d039
#define T46 0xe6db99e5
#define T47 0x1fa27cf8
#define T48 0xc4ac5665
#define T49 0xf4292244
#define T50 0x432aff97
#define T51 0xab9423a7
#define T52 0xfc93a039
#define T53 0x655b59c3
#define T54 0x8f0ccc92
#define T55 0xffeff47d
#define T56 0x85845dd1
#define T57 0x6fa87e4f
#define T58 0xfe2ce6e0
#define T59 0xa3014314
#define T60 0x4e0811a1
#define T61 0xf7537e82
#define T62 0xbd3af235
#define T63 0x2ad7d2bb
#define T64 0xeb86d391

static void md5_process(md5_state_t *pms, const quint8_t data[64])
{
    quint32_t a = pms->abcd[0], b = pms->abcd[1], c = pms->abcd[2], d = pms->abcd[3];
    quint32_t t;

    quint32_t xbuf[16];
    const quint32_t *X;
    static const int w = 1;

    if (*((const quint8_t *)&w))
    {
        if (!((data - (const quint8_t *)0) & 3))
        {
            X = (const quint32_t *)data;
        }
        else
        {
            HAL_MEMCPY(xbuf, data, 64);
            X = xbuf;
        }
    }
    else
    {
        const quint8_t *xp = data;
        int i;
        X = xbuf;
        for (i = 0; i < 16; ++i, xp += 4)
        {
            xbuf[i] = xp[0] + (xp[1] << 8) + (xp[2] << 16) + (xp[3] << 24);
        }
    }

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define FF(a, b, c, d, k, s, Ti)              \
    t = a + ((b & c) | (~b & d)) + X[k] + Ti; \
    a = ROTATE_LEFT(t, s) + b
#define GG(a, b, c, d, k, s, Ti)              \
    t = a + ((b & d) | (c & ~d)) + X[k] + Ti; \
    a = ROTATE_LEFT(t, s) + b
#define HH(a, b, c, d, k, s, Ti)           \
    t = a + ((b) ^ (c) ^ (d)) + X[k] + Ti; \
    a = ROTATE_LEFT(t, s) + b
#define II(a, b, c, d, k, s, Ti)              \
    t = a + ((c) ^ ((b) | ~(d))) + X[k] + Ti; \
    a = ROTATE_LEFT(t, s) + b

    FF(a, b, c, d, 0, 7, T1);
    FF(d, a, b, c, 1, 12, T2);
    FF(c, d, a, b, 2, 17, T3);
    FF(b, c, d, a, 3, 22, T4);
    FF(a, b, c, d, 4, 7, T5);
    FF(d, a, b, c, 5, 12, T6);
    FF(c, d, a, b, 6, 17, T7);
    FF(b, c, d, a, 7, 22, T8);
    FF(a, b, c, d, 8, 7, T9);
    FF(d, a, b, c, 9, 12, T10);
    FF(c, d, a, b, 10, 17, T11);
    FF(b, c, d, a, 11, 22, T12);
    FF(a, b, c, d, 12, 7, T13);
    FF(d, a, b, c, 13, 12, T14);
    FF(c, d, a, b, 14, 17, T15);
    FF(b, c, d, a, 15, 22, T16);

    GG(a, b, c, d, 1, 5, T17);
    GG(d, a, b, c, 6, 9, T18);
    GG(c, d, a, b, 11, 14, T19);
    GG(b, c, d, a, 0, 20, T20);
    GG(a, b, c, d, 5, 5, T21);
    GG(d, a, b, c, 10, 9, T22);
    GG(c, d, a, b, 15, 14, T23);
    GG(b, c, d, a, 4, 20, T24);
    GG(a, b, c, d, 9, 5, T25);
    GG(d, a, b, c, 14, 9, T26);
    GG(c, d, a, b, 3, 14, T27);
    GG(b, c, d, a, 8, 20, T28);
    GG(a, b, c, d, 13, 5, T29);
    GG(d, a, b, c, 2, 9, T30);
    GG(c, d, a, b, 7, 14, T31);
    GG(b, c, d, a, 12, 20, T32);

    HH(a, b, c, d, 5, 4, T33);
    HH(d, a, b, c, 8, 11, T34);
    HH(c, d, a, b, 11, 16, T35);
    HH(b, c, d, a, 14, 23, T36);
    HH(a, b, c, d, 1, 4, T37);
    HH(d, a, b, c, 4, 11, T38);
    HH(c, d, a, b, 7, 16, T39);
    HH(b, c, d, a, 10, 23, T40);
    HH(a, b, c, d, 13, 4, T41);
    HH(d, a, b, c, 0, 11, T42);
    HH(c, d, a, b, 3, 16, T43);
    HH(b, c, d, a, 6, 23, T44);
    HH(a, b, c, d, 9, 4, T45);
    HH(d, a, b, c, 12, 11, T46);
    HH(c, d, a, b, 15, 16, T47);
    HH(b, c, d, a, 2, 23, T48);

    II(a, b, c, d, 0, 6, T49);
    II(d, a, b, c, 7, 10, T50);
    II(c, d, a, b, 14, 15, T51);
    II(b, c, d, a, 5, 21, T52);
    II(a, b, c, d, 12, 6, T53);
    II(d, a, b, c, 3, 10, T54);
    II(c, d, a, b, 10, 15, T55);
    II(b, c, d, a, 1, 21, T56);
    II(a, b, c, d, 8, 6, T57);
    II(d, a, b, c, 15, 10, T58);
    II(c, d, a, b, 6, 15, T59);
    II(b, c, d, a, 13, 21, T60);
    II(a, b, c, d, 4, 6, T61);
    II(d, a, b, c, 11, 10, T62);
    II(c, d, a, b, 2, 15, T63);
    II(b, c, d, a, 9, 21, T64);

    /* Then perform the following additions. (That is increment each
        of the four registers by the value it had before this block
        was started.) */
    pms->abcd[0] += a;
    pms->abcd[1] += b;
    pms->abcd[2] += c;
    pms->abcd[3] += d;
}

void Quos_md5Init(md5_state_t *pms)
{
    pms->count[0] = pms->count[1] = 0;
    pms->abcd[0] = 0x67452301;
    pms->abcd[1] = 0xefcdab89;
    pms->abcd[2] = 0x98badcfe;
    pms->abcd[3] = 0x10325476;
}

void Quos_md5Append(md5_state_t *pms, const quint8_t *data, quint32_t nbytes)
{
    const quint8_t *p = data;
    int left = nbytes;
    int offset = (pms->count[0] >> 3) & 63;
    quint32_t nbits = (quint32_t)(nbytes << 3);

    if (nbytes <= 0)
        return;

    /* Update the message length. */
    pms->count[1] += nbytes >> 29;
    pms->count[0] += nbits;
    if (pms->count[0] < nbits)
        pms->count[1]++;

    /* Process an initial partial block. */
    if (offset)
    {
        int copy;
        if (offset + nbytes > 64)
        {
            copy = 64 - offset;
        }
        else
        {
            copy = nbytes;
        }

        HAL_MEMCPY(pms->buf + offset, p, copy);
        if (offset + copy < 64)
            return;
        p += copy;
        left -= copy;
        md5_process(pms, pms->buf);
    }

    /* Process full blocks. */
    for (; left >= 64; p += 64, left -= 64)
        md5_process(pms, p);

    /* Process a final partial block. */
    if (left)
        HAL_MEMCPY(pms->buf, p, left);
}

void Quos_md5Finish(md5_state_t *pms, quint8_t digest[16])
{
    static const quint8_t pad[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    quint8_t data[8];
    int i;

    /* Save the length before padding. */
    for (i = 0; i < 8; ++i)
        data[i] = (quint8_t)(pms->count[i >> 2] >> ((i & 3) << 3));
    /* Pad to 56 bytes mod 64. */
    Quos_md5Append(pms, pad, ((55 - (pms->count[0] >> 3)) & 63) + 1);
    /* Append the length. */
    Quos_md5Append(pms, data, 8);
    for (i = 0; i < 16; ++i)
        digest[i] = (quint8_t)(pms->abcd[i >> 2] >> ((i & 3) << 3));
}
#endif