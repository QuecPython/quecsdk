#ifndef __QIOT_CMDOTA_H__
#define __QIOT_CMDOTA_H__
#include "Ql_iotApi.h"
#include "Qhal_driver.h"

#define QIOT_COMPNO_MAXSIZE (32)                        /* 组件名称最大长度 */
#define QIOT_COMPVER_MAXSIZE (64)                       /* 组件版本最大长度 */
#define QIOT_OTA_WAIT_URL_TIMEOUT (30 * SWT_ONE_SECOND) /* 确认升级等待获取URL超时时间 */
#define QIOT_OTA_WAIT_READ (60 * SWT_ONE_SECOND)        /* SOTA升级模组通知MCU下载完成，MCU超时不读取数据将判为升级失败 */
#define QIOT_OTA_UPDATE_TIMEOUT (20 * SWT_ONE_MINUTE)
#define QIOT_OTA_DOWNLOADING_NOTIFY (5 * SWT_ONE_SECOND)
#define QIOT_OTA_DOWNLOAD_FAIL_COUNT_MAX 5         /* 最大允许下载失败次数 */
#define QIOT_OTA_UPDATE_DELAY (5 * SWT_ONE_SECOND) /* 模组下载完成，延缓进入升级，保留几秒完成状态上报 */
/* OTA升级TTLV ID */
enum
{
    QIOT_DPID_OTA_DOWN_URL = 1,        /* ota升级资源包下载地址 */
    QIOT_DPID_OTA_DOWN_SIZE = 2,       /* Ota升级资源包大小 */
    QIOT_DPID_OTA_DOWN_MD5 = 3,        /* Ota升级资源包md5值 */
    QIOT_DPID_OTA_COMPONENT_NO = 4,    /* 组件标识 */
    QIOT_DPID_OTA_SOURCE_VER = 5,      /* 源版本 */
    QIOT_DPID_OTA_TARGET_VER = 6,      /* 目标版本 */
    QIOT_DPID_OTA_COMPONENT_TYPE = 7,  /* 组件类型 */
    QIOT_DPID_OTA_BATTERY_LIMIT = 8,   /* Ota升级最小电量 */
    QIOT_DPID_OTA_USE_SPACE = 9,       /* ota升级需要磁盘空间 */
    QIOT_DPID_OTA_MIN_SIGNAL = 10,     /* Ota升级最小信号强度 */
    QIOT_DPID_OTA_SUBMIT = 11,         /* 是否进行ota升级 */
    QIOT_DPID_OTA_DELAY_TIME = 12,     /* Ota升级下次协商时间 */
    QIOT_DPID_OTA_STATUS = 13,         /* Ota升级状态 */
    QIOT_DPID_OTA_MESSAGE = 14,        /* Ota升级状态信息 */
    QIOT_DPID_OTA_DOWN_CRC = 15,       /* Ota升级资源包CRC值 */
    QIOT_DPID_OTA_DOWN_SHA256 = 16,    /* Ota升级资源包SHA256值 */
    QIOT_DPID_OTA_DOWN_SIGN = 19,      /* 额外需要的文件签名类型 */
    QIOT_DPID_OTA_DOWN_INFO = 20,      /* Ota多固件资源信息 */
    QIOT_DPID_OTA_MODULE_VER = 25,     /* 模组版本 */
    QIOT_DPID_OTA_MCU_VER = 26,        /* MCU版本 */
    QIOT_DPID_OTA_HANDLE_TYPE = 27,    /* 升级控制类型 */
    QIOT_DPID_OTA_TASK_INFO = 28,      /* Ota多组件升级任务信息 */
    QIOT_DPID_OTA_DOWN_INFO_IDEX = 30, /* Ota多固件资源信息idex */
};
typedef enum
{
    QIOT_OTA_STATUS_NOPLAN = 0,          /* 空闲 */
    QIOT_OTA_STATUS_REVICEPLAN = 1,      /* 收到任务 */
    QIOT_OTA_STATUS_SUBMITOTA = 2,       /* 收到确认 */
    QIOT_OTA_STATUS_REFUSEDOTA = 3,      /* 收到取消 */
    QIOT_OTA_STATUS_DOWNLOADSTART = 4,   /* 下载开始 */
    QIOT_OTA_STATUS_DOWNLOADING = 5,     /* 下载中 */
    QIOT_OTA_STATUS_DOWNLOADERROR = 6,   /* 下载失败 */
    QIOT_OTA_STATUS_DOWNLOADSUCCESS = 8, /* 下载成功 */
    QIOT_OTA_STATUS_UPDATING = 9,        /* 更新中 */
    QIOT_OTA_STATUS_UPDATESUCCESS = 11,  /* 更新成功 */
    QIOT_OTA_STATUS_UPDATEERROR = 12,    /* 更新失败 */
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
                                                                                    ："Unknown")
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
    QIot_otaComptype_e componentType;             /* 组件类型 */
    char componentNo[QIOT_COMPNO_MAXSIZE + 1];    /* 组件名称最长32bytes*/
    char targetVersion[QIOT_COMPVER_MAXSIZE + 1]; /* 版本号最长50bytes */
    qbool mutilPlansMode;                         /* 是否为多组件 */
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
