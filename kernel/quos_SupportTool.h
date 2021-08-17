#ifndef __QUOS_SUPPORTTOOL_H__
#define __QUOS_SUPPORTTOOL_H__
#include "quos_config.h"
#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        char *path; /* pathָ��url�ĵ�ַ */
        char hostname[QUOS_DNS_HOSTNANE_MAX_LENGHT];
        qbool isSecure;
        quint16_t port;
    } urlAnalyze_t;

#define __ENDIANCHANGE(x) ((sizeof(x) == 2) ? (quint16_t)((x >> 8) | (x << 8)) : ((sizeof(x) == 4) ? (quint32_t)((((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00) | (((x) << 8) & 0x00FF0000) | (((x) << 24) & 0xFF000000)) : (x)))

/* �ֽڶ��� */
#define __BYTE_TO_ALIGN(X, Y) ((X) % (Y) ? ((X) + (Y) - (X) % (Y)) : (X))

/*��Сдת�� */
#define __TO_UPPER(X) ((X) & (~0x20))
#define __TO_LOWER(X) ((X) | 0x20)

#define __IS_DIGIT(X) ('0' <= (X) && (X) <= '9')
#define __IS_XDIGIT(X) (('0' <= (X) && (X) <= '9') || ('a' <= (X) && (X) <= 'f') || ('A' <= (X) && (X) <= 'F'))

/*�����ֵ����Сֵ */
#define __GET_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define __GET_MIN(x, y) (((x) < (y)) ? (x) : (y))

/*�õ�һ��field�ڽṹ��(struct)�е�ƫ���� */
#define __GET_POS_ELEMENT(type, field) ((pointer_t) & (((type *)0)->field))
/*�õ�һ���ṹ����field��ռ�õ��ֽ��� */
#define __GET_SIZE_ELEMENT(type, field) sizeof(((type *)0)->field)
/*����Ԫ�ص�ַ�õ��ṹ�� */
#define __GET_STRUCT_BY_ELEMENT(ptr, type, field) ((type *)((char *)ptr - __GET_POS_ELEMENT(type, field)))
/*����һ����XС����ӽ���n�ı��� */
#define __GET_SMALL_N(x, n) ((x) / (n) * (n))

/*����һ����X�����ӽ���n�ı��� */
#define __GET_BIG_N(x, n) (((x) + (n)-1) / (n) * (n))

/* ת����Ϊ�ַ��� */
#define _MACRO2STR_1(s) #s
#define _MACRO2STR_2(s) _MACRO2STR_1(s)

/* �ַ���ƴ�� */
#define _STRCAT_STR_1(A, B) A##B
#define _STRCAT_STR_2(A, B) _STRCAT_STR_1(A, B)

#define _BOOL2STR(X) ((X) ? "TRUE" : "FALSE") /* qbool ת�ַ��� */
#define _STR2BOOL(X) (0 == HAL_STRCASECMP(X, "TRUE") ? TRUE : FALSE)

#define _DATA2BOOL(X, Y) (((X >> Y) & 1) ? TRUE : FALSE)

#define _ARRAY01_U16(ARRAY) (((quint16_t)(((quint8_t *)(ARRAY))[0]) << 8) | \
                             ((quint16_t)(((quint8_t *)(ARRAY))[1]) << 0))

#define _ARRAY10_U16(ARRAY) (((quint16_t)(((quint8_t *)(ARRAY))[1]) << 8) | \
                             ((quint16_t)(((quint8_t *)(ARRAY))[0]) << 0))

#define _ARRAY0123_U32(ARRAY) (((quint32_t)(((quint8_t *)(ARRAY))[0]) << 24) | \
                               ((quint32_t)(((quint8_t *)(ARRAY))[1]) << 16) | \
                               ((quint32_t)(((quint8_t *)(ARRAY))[2]) << 8) |  \
                               ((quint32_t)(((quint8_t *)(ARRAY))[3]) << 0))

#define _ARRAY1032_U32(ARRAY) (((quint32_t)(((quint8_t *)(ARRAY))[1]) << 24) | \
                               ((quint32_t)(((quint8_t *)(ARRAY))[0]) << 16) | \
                               ((quint32_t)(((quint8_t *)(ARRAY))[3]) << 8) |  \
                               ((quint32_t)(((quint8_t *)(ARRAY))[2]) << 0))

#define _ARRAY3210_U32(ARRAY) (((quint32_t)(((quint8_t *)(ARRAY))[3]) << 24) | \
                               ((quint32_t)(((quint8_t *)(ARRAY))[2]) << 16) | \
                               ((quint32_t)(((quint8_t *)(ARRAY))[1]) << 8) |  \
                               ((quint32_t)(((quint8_t *)(ARRAY))[0]) << 0))

#define _ARRAY2301_U32(ARRAY) (((quint32_t)(((quint8_t *)(ARRAY))[2]) << 24) | \
                               ((quint32_t)(((quint8_t *)(ARRAY))[3]) << 16) | \
                               ((quint32_t)(((quint8_t *)(ARRAY))[0]) << 8) |  \
                               ((quint32_t)(((quint8_t *)(ARRAY))[1]) << 0))

#define _ARRAY76543210_U64(ARRAY)   (((quint64_t)(((quint8_t *)(ARRAY))[7]) << 56) | \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[6]) << 48) | \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[5]) << 40) | \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[4]) << 32) | \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[3]) << 24) | \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[2]) << 16) | \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[1]) << 8) |  \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[0]) << 0))

#define _ARRAY012345678_U64(ARRAY)   (((quint64_t)(((quint8_t *)(ARRAY))[0]) << 56) | \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[1]) << 48) | \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[2]) << 40) | \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[3]) << 32) | \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[4]) << 24) | \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[4]) << 16) | \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[6]) << 8) |  \
                                    ((quint64_t)(((quint8_t *)(ARRAY))[7]) << 0))

#define _U16_ARRAY01(INT, ARRAY) (((quint8_t *)ARRAY)[0] = ((INT) >> 8) & 0xFF, ((quint8_t *)ARRAY)[1] = (INT)&0xFF)
#define _U16_ARRAY10(INT, ARRAY) (((quint8_t *)ARRAY)[0] = (INT)&0xFF, ((quint8_t *)ARRAY)[1] = ((INT) >> 8) & 0xFF)
#define _U32_ARRAY0123(INT, ARRAY) (((quint8_t *)ARRAY)[0] = ((INT) >> 24) & 0xFF, ((quint8_t *)ARRAY)[1] = ((INT) >> 16) & 0xFF, ((quint8_t *)ARRAY)[2] = ((INT) >> 8) & 0xFF, ((quint8_t *)ARRAY)[3] = (INT)&0xFF)
#define _U32_ARRAY3210(INT, ARRAY) (((quint8_t *)ARRAY)[0] = (INT)&0xFF, ((quint8_t *)ARRAY)[1] = ((INT) >> 8) & 0xFF, ((quint8_t *)ARRAY)[2] = ((INT) >> 16) & 0xFF, ((quint8_t *)ARRAY)[3] = ((INT) >> 24) & 0xFF)
#define _U64_ARRAY01234567(INT, ARRAY) (((quint8_t *)ARRAY)[0] = ((INT) >> 56) & 0xFF, ((quint8_t *)ARRAY)[1] = ((INT) >> 48) & 0xFF, ((quint8_t *)ARRAY)[2] = ((INT) >> 40) & 0xFF, ((quint8_t *)ARRAY)[3] = ((INT) >> 32) & 0xFF, \
                                        ((quint8_t *)ARRAY)[4] = ((INT) >> 24) & 0xFF, ((quint8_t *)ARRAY)[5] = ((INT) >> 16) & 0xFF, ((quint8_t *)ARRAY)[6] = ((INT) >> 8) & 0xFF, ((quint8_t *)ARRAY)[7] = ((INT) >> 0) & 0xFF)
#define _U64_ARRAY76543210(INT, ARRAY) (((quint8_t*)ARRAY)[0] = (INT)&0xFF,((quint8_t*)ARRAY)[1] = ((INT)>>8)&0xFF,((quint8_t*)ARRAY)[2] = ((INT)>>16)&0xFF,((quint8_t*)ARRAY)[3] = ((INT)>>24)&0xFF, \
                                (((quint8_t*)ARRAY)[4] = ((INT)>>32)&0xFF,((quint8_t*)ARRAY)[5] = ((INT)>>40)&0xFF,((quint8_t*)ARRAY)[6] = ((INT)>>48)&0xFF,((quint8_t*)ARRAY)[7] = ((INT)>>56)&0xFF)

#define IP2STR "%hhu.%hhu.%hhu.%hhu"
#define IP2STR_(IP) (quint8_t)((IP) >> 24), (quint8_t)((IP) >> 16), (quint8_t)((IP) >> 8), (quint8_t)((IP) >> 0)

#define TIME_SEC2UTC "%08d.%02d.%02d"
#define TIME_SEC2UTC_(SEC) (quint32_t)(SEC / 3600), (quint8_t)(SEC % 3600 / 60), (quint8_t)(SEC % 60)

    quint32_t Quos_hex2Str(quint8_t hex[], quint32_t hexLen, char *retStr, qbool isUpper);
    quint32_t Quos_str2Hex(void *srcStr, quint8_t RetHex[]);
    quint8_t Quos_convertToExp(quint32_t value, quint32_t exp);
    quint32_t Quos_crcCalculate(quint32_t crc, void *buf, quint32_t len);
    qint32_t Quos_keyValueExtract(char *srcStr, const char *keyword, const char *separator, char **dstStr, const char *endStr);
    qbool Quos_keyValueInsert(char *srcStr, quint32_t maxLen, const char *keyword, const char *separator, const char *value, char *endStr);
    quint32_t Quos_stringSplit(char *src, char **words, quint32_t maxSize, char *delim, qbool keepEmptyParts);
    qbool Quos_strIsUInt(char *src, quint32_t len, quint32_t *value);
    qbool Quos_urlAnalyze(const char *url, urlAnalyze_t *result);
    quint32_t Quos_ip2Int(const char *ipStr);
    quint64_t Quos_strtoul(const char *cp, char **endp, quint32_t base);
    qint64_t Quos_strtol(const char *cp, char **endp, quint32_t base);
    qbool Quos_fileMd5(const char *filename, quint32_t fileLen, char md5[]);
    quint32_t Quos_intPushArray(quint64_t intValue,quint8_t *array);
    char *Quos_stringRemoveMarks(char *strVal);

#ifdef __cplusplus
}
#endif

#endif
