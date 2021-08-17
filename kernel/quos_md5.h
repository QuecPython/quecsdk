#ifndef __QUOS_MD5_H__
#define __QUOS_MD5_H__
#include "quos_config.h"
#if (SDK_ENABLE_MD5 == 1)
#ifdef __cplusplus
extern "C"
{
#endif

    /* Define the state of the MD5 Algorithm. */
    typedef struct
    {
        quint32_t count[2]; /* message length in bits, lsw first */
        quint32_t abcd[4];  /* digest buffer */
        quint8_t buf[64];   /* accumulate block */
    } md5_state_t;

    /* Initialize the algorithm. */
    void Quos_md5Init(md5_state_t *pms);
    void Quos_md5Append(md5_state_t *pms, const quint8_t *data, quint32_t nbytes);
    void Quos_md5Finish(md5_state_t *pms, quint8_t digest[16]);

#ifdef __cplusplus
}
#endif
#endif
#endif
