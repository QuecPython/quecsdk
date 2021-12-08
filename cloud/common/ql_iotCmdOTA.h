#ifndef __QIOT_CMDOTA_H__
#define __QIOT_CMDOTA_H__
#include "Ql_iotApi.h"
#include "Qhal_driver.h"

#define QIOT_COMPNO_MAXSIZE (32)                        /* ���������󳤶� */
#define QIOT_COMPVER_MAXSIZE (64)                       /* ����汾��󳤶� */
#define QIOT_OTA_WAIT_URL_TIMEOUT (30 * SWT_ONE_SECOND) /* ȷ�������ȴ���ȡURL��ʱʱ�� */
#define QIOT_OTA_WAIT_READ (60 * SWT_ONE_SECOND)        /* SOTA����ģ��֪ͨMCU������ɣ�MCU��ʱ����ȡ���ݽ���Ϊ����ʧ�� */
#define QIOT_OTA_UPDATE_TIMEOUT (20 * SWT_ONE_MINUTE)
#define QIOT_OTA_DOWNLOADING_NOTIFY (5 * SWT_ONE_SECOND)
#define QIOT_OTA_DOWNLOAD_FAIL_COUNT_MAX 5         /* �����������ʧ�ܴ��� */
#define QIOT_OTA_UPDATE_DELAY (5 * SWT_ONE_SECOND) /* ģ��������ɣ��ӻ����������������������״̬�ϱ� */
/* OTA����TTLV ID */
enum
{
    QIOT_DPID_OTA_DOWN_URL = 1,        /* ota������Դ�����ص�ַ */
    QIOT_DPID_OTA_DOWN_SIZE = 2,       /* Ota������Դ����С */
    QIOT_DPID_OTA_DOWN_MD5 = 3,        /* Ota������Դ��md5ֵ */
    QIOT_DPID_OTA_COMPONENT_NO = 4,    /* �����ʶ */
    QIOT_DPID_OTA_SOURCE_VER = 5,      /* Դ�汾 */
    QIOT_DPID_OTA_TARGET_VER = 6,      /* Ŀ��汾 */
    QIOT_DPID_OTA_COMPONENT_TYPE = 7,  /* ������� */
    QIOT_DPID_OTA_BATTERY_LIMIT = 8,   /* Ota������С���� */
    QIOT_DPID_OTA_USE_SPACE = 9,       /* ota������Ҫ���̿ռ� */
    QIOT_DPID_OTA_MIN_SIGNAL = 10,     /* Ota������С�ź�ǿ�� */
    QIOT_DPID_OTA_SUBMIT = 11,         /* �Ƿ����ota���� */
    QIOT_DPID_OTA_DELAY_TIME = 12,     /* Ota�����´�Э��ʱ�� */
    QIOT_DPID_OTA_STATUS = 13,         /* Ota����״̬ */
    QIOT_DPID_OTA_MESSAGE = 14,        /* Ota����״̬��Ϣ */
    QIOT_DPID_OTA_DOWN_CRC = 15,       /* Ota������Դ��CRCֵ */
    QIOT_DPID_OTA_DOWN_SHA256 = 16,    /* Ota������Դ��SHA256ֵ */
    QIOT_DPID_OTA_DOWN_SIGN = 19,      /* ������Ҫ���ļ�ǩ������ */
    QIOT_DPID_OTA_DOWN_INFO = 20,      /* Ota��̼���Դ��Ϣ */
    QIOT_DPID_OTA_MODULE_VER = 25,     /* ģ��汾 */
    QIOT_DPID_OTA_MCU_VER = 26,        /* MCU�汾 */
    QIOT_DPID_OTA_HANDLE_TYPE = 27,    /* ������������ */
    QIOT_DPID_OTA_TASK_INFO = 28,      /* Ota���������������Ϣ */
    QIOT_DPID_OTA_DOWN_INFO_IDEX = 30, /* Ota��̼���Դ��Ϣidex */
};
typedef enum
{
    QIOT_OTA_STATUS_NOPLAN = 0,          /* ���� */
    QIOT_OTA_STATUS_REVICEPLAN = 1,      /* �յ����� */
    QIOT_OTA_STATUS_SUBMITOTA = 2,       /* �յ�ȷ�� */
    QIOT_OTA_STATUS_REFUSEDOTA = 3,      /* �յ�ȡ�� */
    QIOT_OTA_STATUS_DOWNLOADSTART = 4,   /* ���ؿ�ʼ */
    QIOT_OTA_STATUS_DOWNLOADING = 5,     /* ������ */
    QIOT_OTA_STATUS_DOWNLOADERROR = 6,   /* ����ʧ�� */
    QIOT_OTA_STATUS_DOWNLOADSUCCESS = 8, /* ���سɹ� */
    QIOT_OTA_STATUS_UPDATING = 9,        /* ������ */
    QIOT_OTA_STATUS_UPDATESUCCESS = 11,  /* ���³ɹ� */
    QIOT_OTA_STATUS_UPDATEERROR = 12,    /* ����ʧ�� */
} QIot_otaStatus_e;

#define QIOT_OTA_STATUS_STRING(X)                                                                           \
    (                                                                                                       \
        (X == QIOT_OTA_STATUS_NOPLAN) ? "NOPLAN" : (X == QIOT_OTA_STATUS_REVICEPLAN)    ? "REVICEPLAN"      \
                                               : (X == QIOT_OTA_STATUS_SUBMITOTA)       ? "SUBMITOTA"       \
                                               : (X == QIOT_OTA_STATUS_REFUSEDOTA)      ? "REFUSEDOTA"      \
                                               : (X == QIOT_OTA_STATUS_DOWNLOADSTART)   ? "DOWNLOADSTART"   \
                                               : (X == QIOT_OTA_STATUS_DOWNLOADING)     ? "DOWNLOADING"     \
                                               : (X == QIOT_OTA_STATUS_DOWNLOADERROR)   ? "DOWNLOADERROR"   \
                                               : (X == QIOT_OTA_STATUS_DOWNLOADSUCCESS) ? "DOWNLOADSUCCESS" \
                                               : (X == QIOT_OTA_STATUS_UPDATING)        ? "UPDATING"        \
                                               : (X == QIOT_OTA_STATUS_UPDATESUCCESS)   ? "UPDATESUCCESS"   \
                                               : (X == QIOT_OTA_STATUS_UPDATEERROR)     ? "UPDATEERROR"     \
                                                                                        : "Unknown")
#define QIOT_OTA_ERR_STRING(X)                                                                               \
    (                                                                                                   \
        (X == QIOT_OTA_TASK_NOTIFY) ? "OTA_TASK_NOTIFY" : (X == QIOT_OTA_START)     ? "OTA_START"       \
                                                    : (X == QIOT_OTA_DOWNLOADING)   ? "OTA_DOWNLOADING" \
                                                    : (X == QIOT_OTA_DOWNLOADED)    ? "OTA_DOWNLOADED"  \
                                                    : (X == QIOT_OTA_UPDATING)      ? "OTA_UPDATING"    \
                                                    : (X == QIOT_OTA_UPDATE_OK)     ? "OTA_UPDATE_OK"   \
                                                    : (X == QIOT_OTA_UPDATE_FAIL)   ? "OTA_UPDATE_FAIL" \
                                                    : (X == QIOT_OTA_UPDATE_FLAG)   ? "QIOT_OTA_UPDATE_FLAG" \
                                                                                    ��"Unknown")
enum
{
    QIOT_OTA_EXTERN_NONE = 0,
    QIOT_OTA_EXTERN_SHA256 = 1,
    QIOT_OTA_EXTERN_MAX,
};

typedef enum
{
    QIOT_COMPTYPE_MODULE = 0,
    QIOT_COMPTYPE_MCU,
} QIot_otaComptype_e;

typedef struct
{
    QIot_otaComptype_e componentType;             /* ������� */
    char componentNo[QIOT_COMPNO_MAXSIZE + 1];    /* ��������32bytes*/
    char targetVersion[QIOT_COMPVER_MAXSIZE + 1]; /* �汾���50bytes */
    qbool mutilPlansMode;                         /* �Ƿ�Ϊ����� */
    QIot_otaStatus_e currStatus;
    quint32_t extraMess;
    quint32_t crc;
    char sha256[QUOS_SHA256_DIGEST_LENGTH * 2 + 1];
    struct
    {
        quint32_t startAddr;
        quint32_t size;
    } currentPiece;
    QIot_otaFilePublicInfo_t otaFileInfo;
} Ql_iotCmdOtaInfo_t;
extern Ql_iotCmdOtaInfo_t QIot_otaInfo;
void Ql_iotCmdOtaInit(void);
void Ql_iotCmdOtaStatusRecovery(void);
void Ql_iotCmdOtaNotify(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen);
void Ql_iotCmdOtaFwInfo(QIot_dpAppType_e app, const char *endPoint, quint16_t pkgId, quint8_t *payload, quint16_t payloadLen);
void Ql_iotCmdOtaMcuVersionModify(const char *compNo, const char *version);
#endif
