/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    : 2020-12-25
** 功能   @brief   : dmp安全算法
** 硬件   @hardware：
** 其他   @other   ：
***************************************************************************/
#include "ql_iotSecure.h"
#include "ql_iotConfig.h"
#include "Qhal_driver.h"

#define QIOT_MQTT_AUTH_VERSION "3"
#define QIOT_COAP_AUTH_VERSION "2"

/**************************************************************************
** 功能	@brief : 获取认证协议版本号
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
char FUNCTION_ATTR_ROM *Ql_iotSecureVerGet(void)
{
    return QIOT_MQTT_AUTH_VERSION;
}
/**************************************************************************
** 功能	@brief : 生成接入的签名
** 输入	@param : 
** 输出	@retval: 返回值需要外部FREE内存
***************************************************************************/
char FUNCTION_ATTR_ROM *ql_iotSecureGenConnSign(const char *ps, const char *ds, const char *random)
{
    /* 分配最大buffer */
    quint32_t maxLen = HAL_STRLEN(ds) + 1 + HAL_STRLEN(ps) + 1 + HAL_STRLEN(random) + 1;
    maxLen = __GET_MAX(maxLen, QUOS_BASE64_DSTDATA_LEN(QUOS_SHA256_DIGEST_LENGTH));
    Quos_logPrintf(QUEC_SECURE, LL_DBG, "keystr maxLen:%d", maxLen);
    char *bodyStr = HAL_MALLOC(maxLen);
    if (NULL == bodyStr)
    {
        Quos_logPrintf(QUEC_SECURE, LL_ERR, "mcf bodyStr fail");
        return NULL;
    }
    /* 第一步：设备签名 SIGN = BASE64(SHA256(DS;PS;RAND)) */
    /* DS;PS;RAND */
    HAL_SPRINTF(bodyStr, "%s;%s;%s", ds, ps, random);
    Quos_logPrintf(QUEC_SECURE, LL_DBG, "bodyStr:[%ld]%s", HAL_STRLEN(bodyStr), bodyStr);

    /* SHA256(DS;PS;RAND) */
    SHA256_ctx_t sha256_ctx;
    quint8_t sha256Data[QUOS_SHA256_DIGEST_LENGTH];
    Quos_sha256init(&sha256_ctx);
    Quos_sha256update(&sha256_ctx, (const quint8_t *)bodyStr, HAL_STRLEN(bodyStr));
    Quos_sha256finish(&sha256_ctx, sha256Data);
    Quos_logHexDump(QUEC_SECURE, LL_DBG, "sha256Data", sha256Data, QUOS_SHA256_DIGEST_LENGTH);

    /* BASE64(SHA256(DS;PS;RAND)) */
    Quos_base64Encrypt(sha256Data, QUOS_SHA256_DIGEST_LENGTH, (quint8_t *)bodyStr);
    Quos_logPrintf(QUEC_SECURE, LL_DBG, "Sign:[%ld]%s", HAL_STRLEN(bodyStr), bodyStr);
    return bodyStr;
}
/**************************************************************************
** 功能	@brief : 生成设备认证密钥
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
char FUNCTION_ATTR_ROM *ql_iotSecureGenAuthKey(const char *pk, const char *ps, const char *dk, const char *random)
{
    UNUSED(pk);
    quint32_t maxLen = HAL_STRLEN(dk) + 1 + __GET_MAX(HAL_STRLEN(ps), QUOS_BASE64_DSTDATA_LEN(QUOS_SHA256_DIGEST_LENGTH)) + 1 + HAL_STRLEN(random) + 1;
    maxLen = __BYTE_TO_ALIGN(maxLen + 1, QUOS_AES_BLOCKLEN);
    maxLen = QUOS_BASE64_DSTDATA_LEN(maxLen);
    Quos_logPrintf(QUEC_SECURE, LL_DBG, "keystr maxLen:%d", maxLen);
    char *bodyStr = HAL_MALLOC(maxLen);
    if (NULL == bodyStr)
    {
        Quos_logPrintf(QUEC_SECURE, LL_ERR, "mcf bodyStr fail");
        return NULL;
    }

    /* 第一步：设备签名 SIGN = BASE64(SHA256(DK;PS;RAND)) */
    /* DK;PS;RAND */
    HAL_SPRINTF(bodyStr, "%s;%s;%s", dk, ps, random);
    Quos_logPrintf(QUEC_SECURE, LL_DBG, "bodyStr:[%ld]%s", HAL_STRLEN(bodyStr), bodyStr);
    /* SHA256(DK;PS;RAND) */
    SHA256_ctx_t sha256_ctx;
    quint8_t sha256Data[QUOS_SHA256_DIGEST_LENGTH];
    Quos_sha256init(&sha256_ctx);
    Quos_sha256update(&sha256_ctx, (const quint8_t *)bodyStr, HAL_STRLEN(bodyStr));
    Quos_sha256finish(&sha256_ctx, sha256Data);
    Quos_logHexDump(QUEC_SECURE, LL_DBG, "sha256Data", sha256Data, QUOS_SHA256_DIGEST_LENGTH);

    HAL_SPRINTF(bodyStr, "%s;%s;", dk, random);
    /* BASE64(SHA256(DK;PS;RAND)) */
    Quos_base64Encrypt(sha256Data, QUOS_SHA256_DIGEST_LENGTH, (quint8_t *)bodyStr + HAL_STRLEN(bodyStr));
    Quos_logPrintf(QUEC_SECURE, LL_DBG, "aes srcData:[%ld]%s", HAL_STRLEN(bodyStr), bodyStr);

    /* 第二步，对DATA请求体进行AES算法加密然后进行BASE64编码 BASE64（AES(PS,DATA请求体)） */
    /* PS使用SHA256加密，生成32字节的数据,iv偏移量为sha256(ps)的左边16Byte */
    Quos_sha256init(&sha256_ctx);
    Quos_sha256update(&sha256_ctx, (const quint8_t *)ps, HAL_STRLEN(ps));
    Quos_sha256finish(&sha256_ctx, sha256Data);

    maxLen = HAL_STRLEN(bodyStr);
    AES_ctx_t aes_ctx;
    Quos_logHexDump(QUEC_SECURE, LL_DBG, "IV", sha256Data, QUOS_SHA256_DIGEST_LENGTH);
    Quos_aesInitCtxIv(&aes_ctx, (const char *)ps, (const char *)sha256Data);
    Quos_aesPadding((quint8_t *)bodyStr, (quint8_t *)bodyStr, maxLen);
    maxLen = Quos_aesCbcEncrypt(&aes_ctx, bodyStr, __BYTE_TO_ALIGN(maxLen + 1, QUOS_AES_BLOCKLEN));
    Quos_logHexDump(QUEC_SECURE, LL_DBG, "aes dstdata", bodyStr, maxLen);

    /* BASE64（AES(PS,DATA请求体)） */
    quint32_t offset = QUOS_BASE64_DSTDATA_LEN(maxLen) - maxLen;
    HAL_MEMMOVE(bodyStr + offset, bodyStr, maxLen);
    Quos_logPrintf(QUEC_SECURE, LL_DBG, "offset:%d", offset);
    Quos_base64Encrypt((quint8_t *)bodyStr + offset, maxLen, (quint8_t *)bodyStr);
    Quos_logPrintf(QUEC_SECURE, LL_DBG, "bodyStr:[%ld]%s", HAL_STRLEN(bodyStr), bodyStr);
    return bodyStr;
}
/**************************************************************************
** 功能	@brief : 生成MQTT设备连接password
** 输入	@param : 
** 输出	@retval: 返回值需要外部FREE内存
***************************************************************************/
char FUNCTION_ATTR_ROM *Ql_iotSecureGenMqttConnData(const char *ps, const char *ds)
{
    char random[16 + 1];
    HAL_MEMSET(random, 0, sizeof(random));
    Quos_RandomGen((quint8_t *)random, sizeof(random) - 1);
    char *sign = ql_iotSecureGenConnSign(ps, ds, random);
    if (sign)
    {
        char *passWord = HAL_MALLOC(1 + 1 + 4 + 1 + HAL_STRLEN(random) + 1 + HAL_STRLEN(sign) + 1);
        if (passWord)
        {
            /* 拼接passWord */
            HAL_SPRINTF((char *)passWord, "1;%04d;%s;%s", Ql_iotConfigGetSessionFlag(), random, sign);
        }
        HAL_FREE(sign);
        Quos_logPrintf(QUEC_SECURE, LL_DBG, "passWord:%s", passWord);
        return passWord;
    }
    return NULL;
}
/**************************************************************************
** 功能	@brief : 生成MQTT设备认证password
** 输入	@param : 
** 输出	@retval: 返回值需要外部FREE内存
***************************************************************************/
char FUNCTION_ATTR_ROM *Ql_iotSecureGenMqttAuthData(const char *pk, const char *ps, const char *dk)
{
    char random[16 + 1];
    HAL_MEMSET(random, 0, sizeof(random));
    Quos_RandomGen((quint8_t *)random, sizeof(random) - 1);
    char *bodyStr = ql_iotSecureGenAuthKey(pk, ps, dk, random);
    if (bodyStr)
    {
        char *passWord = HAL_MALLOC(1 + 1 + 4 + 1 + HAL_STRLEN(pk) + 1 + HAL_STRLEN(bodyStr) + 1);
        if (passWord)
        {
            /* 拼接passWord */
            HAL_SPRINTF((char *)passWord, "0;%04d;%s;%s", Ql_iotConfigGetSessionFlag(), pk, bodyStr);
        }
        HAL_FREE(bodyStr);
        Quos_logPrintf(QUEC_SECURE, LL_DBG, "passWord:[%ld]%s",HAL_STRLEN(passWord), passWord);
        return passWord;
    }
    return NULL;
}
/**************************************************************************
** 功能	@brief : 生成COAP设备连接password
** 输入	@param : 
** 输出	@retval: 返回值需要外部FREE内存
***************************************************************************/
char FUNCTION_ATTR_ROM *Ql_iotSecureGenCoapConnEndpoint(const char *pk, const char *ps, const char *dk, const char *ds)
{
    char random[16 + 1];
    HAL_MEMSET(random, 0, sizeof(random));
    Quos_RandomGen((quint8_t *)random, sizeof(random) - 1);
    char *sign = ql_iotSecureGenConnSign(ps, ds, random);
    if (sign)
    {
        char *passWord = HAL_MALLOC(HAL_STRLEN(pk) + 1 + HAL_STRLEN(dk) + 1 + HAL_STRLEN(random) + 1 + HAL_STRLEN(sign) + 1 + HAL_STRLEN(QIOT_COAP_AUTH_VERSION) + 1);
        if (passWord)
        {
            /* 拼接passWord */
            HAL_SPRINTF((char *)passWord, "%s;%s;%s;%s;%s", pk, dk, random, sign, QIOT_COAP_AUTH_VERSION);
        }
        HAL_FREE(sign);
        Quos_logPrintf(QUEC_SECURE, LL_DBG, "passWord:%s", passWord);
        return passWord;
    }
    return NULL;
}
/**************************************************************************
** 功能	@brief : 生成coap设备认证password
** 输入	@param : 
** 输出	@retval: 返回值需要外部FREE内存
***************************************************************************/
char FUNCTION_ATTR_ROM *Ql_iotSecureGenCoapAuthPayload(const char *pk, const char *ps, const char *dk)
{
    char random[16 + 1];
    HAL_MEMSET(random, 0, sizeof(random));
    Quos_RandomGen((quint8_t *)random, sizeof(random) - 1);
    char *bodyStr = ql_iotSecureGenAuthKey(pk, ps, dk, random);
    if (bodyStr)
    {
        char *passWord = HAL_MALLOC(HAL_STRLEN(pk) + 1 + HAL_STRLEN(bodyStr) + 1 + HAL_STRLEN(QIOT_COAP_AUTH_VERSION) + 1);
        if (passWord)
        {
            /* 拼接passWord */
            HAL_SPRINTF((char *)passWord, "%s;%s;%s", pk, bodyStr, QIOT_COAP_AUTH_VERSION);
        }
        HAL_FREE(bodyStr);
        Quos_logPrintf(QUEC_SECURE, LL_DBG, "passWord:%s", passWord);
        return passWord;
    }
    return NULL;
}
/**************************************************************************
** 功能	@brief : ds解密提取
** 输入	@param : 
** 输出	@retval: 返回值需要外部FREE内存
***************************************************************************/
char FUNCTION_ATTR_ROM *Ql_iotSecureDecodeDs(const char *ps, const quint8_t *encryData, quint32_t len)
{
    quint8_t *ds = HAL_MALLOC(len + 1);
    if (NULL == ds)
    {
        return NULL;
    }
    len = Quos_base64Decrypt(encryData, len, ds);
    if (len % QUOS_AES_BLOCKLEN != 0)
    {
        HAL_FREE(ds);
        return NULL;
    }
    quint8_t sha256Data[QUOS_SHA256_DIGEST_LENGTH];
    SHA256_ctx_t sha256_ctx;
    Quos_sha256init(&sha256_ctx);
    Quos_sha256update(&sha256_ctx, (const quint8_t *)ps, HAL_STRLEN(ps));
    Quos_sha256finish(&sha256_ctx, sha256Data);

    AES_ctx_t aes_ctx;
    Quos_aesInitCtxIv(&aes_ctx, (const char *)ps, (const char *)sha256Data);
    Quos_aesCbcDecrypt(&aes_ctx, ds, len);
    len = Quos_aesPaddingBack(ds, len);
    ds[len] = 0;
    return (char *)ds;
}
char FUNCTION_ATTR_ROM *Ql_iotSecureDecodeSessionKey(const char *ds, const quint8_t *encryData, quint32_t len)
{
    if (len % QUOS_AES_BLOCKLEN != 0)
    {
        return NULL;
    }
    quint8_t *sessionKey = HAL_MALLOC(len + 1);
    if (NULL == sessionKey)
    {
        return NULL;
    }

    HAL_MEMCPY(sessionKey, encryData, len);

    quint8_t sha256Data[QUOS_SHA256_DIGEST_LENGTH];
    SHA256_ctx_t sha256_ctx;
    Quos_sha256init(&sha256_ctx);
    Quos_sha256update(&sha256_ctx, (const quint8_t *)ds, HAL_STRLEN(ds));
    Quos_sha256finish(&sha256_ctx, sha256Data);
    AES_ctx_t aes_ctx;
    Quos_aesInitCtxIv(&aes_ctx, (const char *)ds, (const char *)sha256Data);
    Quos_aesCbcDecrypt(&aes_ctx, sessionKey, len);
    len = Quos_aesPaddingBack(sessionKey, len);
    sessionKey[len] = 0;
    return (char *)sessionKey;
}
/**************************************************************************
** 功能	@brief : 上行数据加密
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Ql_iotSecureEncryptPayload(quint8_t *data, quint32_t len, uint8_t **outData, const char *key, const char *iv)
{
    /* 上下行消息的时候，如果开启sessionkey的话，需要用sessionkey作为key做aes加密，那么sessionkey的sha256左边16位就是iv偏移量 */
    AES_ctx_t aes_ctx;
    Quos_aesInitCtxIv(&aes_ctx, key, iv);
    quint32_t encryptLen = __BYTE_TO_ALIGN(len + 1, QUOS_AES_BLOCKLEN);
    *outData = HAL_MALLOC(encryptLen);
    if (*outData)
    {
        Quos_aesPadding((quint8_t *)*outData, (quint8_t *)data, len);
        Quos_aesCbcEncrypt(&aes_ctx, (void *)*outData, encryptLen);
    }
    return encryptLen;
}

/**************************************************************************
** 功能	@brief : 下行数据解密
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Ql_iotSecureDecryptPayload(quint8_t *data, quint32_t len, const char *key, const char *iv)
{
    if (len % QUOS_AES_BLOCKLEN != 0)
    {
        return 0;
    }
    /* 上下行消息的时候，如果开启sessionkey的话，需要用sessionkey作为key做aes加密，那么sessionkey的sha256左边16位就是iv偏移量 */
    AES_ctx_t aes_ctx;
    Quos_aesInitCtxIv(&aes_ctx, key, iv);
    Quos_aesCbcDecrypt(&aes_ctx, (void *)data, len);
    len = Quos_aesPaddingBack(data, len);
    return len;
}