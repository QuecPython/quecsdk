#ifndef __QUOS_COAP_H__
#define __QUOS_COAP_H__
#include "quos_config.h"
#include "quos_twll.h"
#include "quos_socket.h"
#if (SDK_ENABLE_COAP == 1)

typedef enum
{
    COAP_HTYPE_CON, /* confirmables */
    COAP_HTYPE_NON, /* non-confirmables */
    COAP_HTYPE_ACK, /* acknowledgements */
    COAP_HTYPE_RST  /* reset */
} coapHeadType_t;
#define COAP_HEAD_TYPE_STRING(X)                                                            \
    (                                                                                       \
        (X == COAP_HTYPE_CON) ? "COAP_HTYPE_CON" : (X == COAP_HTYPE_NON) ? "COAP_HTYPE_NON" \
                                               : (X == COAP_HTYPE_ACK)   ? "COAP_HTYPE_ACK" \
                                               : (X == COAP_HTYPE_RST)   ? "COAP_HTYPE_RST" \
                                                                         : "Unknown")
typedef enum
{
#define COAP_HEAD_CODE(X, Y) (((X) << 5) + Y)
    COAP_HCODE_EMPTY = COAP_HEAD_CODE(0, 0),
    COAP_HCODE_GET = COAP_HEAD_CODE(0, 1),
    COAP_HCODE_POST = COAP_HEAD_CODE(0, 2),
    COAP_HCODE_PUT = COAP_HEAD_CODE(0, 3),
    COAP_HCODE_DELETE = COAP_HEAD_CODE(0, 4),

    COAP_HCODE_CREATED_201 = COAP_HEAD_CODE(2, 1), /* CREATED */
    COAP_HCODE_DELETED_202 = COAP_HEAD_CODE(2, 2), /* DELETED */
    COAP_HCODE_VALID_203 = COAP_HEAD_CODE(2, 3),   /* NOT_MODIFIED */
    COAP_HCODE_CHANGED_204 = COAP_HEAD_CODE(2, 4), /* CHANGED */
    COAP_HCODE_CONTENT_205 = COAP_HEAD_CODE(2, 5), /* OK */

    COAP_HCODE_BAD_REQUEST_400 = COAP_HEAD_CODE(4, 0),               /* BAD_REQUEST */
    COAP_HCODE_UNAUTHORIZED_401 = COAP_HEAD_CODE(4, 1),              /* UNAUTHORIZED */
    COAP_HCODE_BAD_OPTION_402 = COAP_HEAD_CODE(4, 2),                /* BAD_OPTION */
    COAP_HCODE_FORBIDDEN_403 = COAP_HEAD_CODE(4, 3),                 /* FORBIDDEN */
    COAP_HCODE_NOT_FOUND_404 = COAP_HEAD_CODE(4, 4),                 /* NOT_FOUND */
    COAP_HCODE_METHOD_NOT_ALLOWED_405 = COAP_HEAD_CODE(4, 5),        /* METHOD_NOT_ALLOWED */
    COAP_HCODE_NOT_ACCEPTABLE_406 = COAP_HEAD_CODE(4, 6),            /* NOT_ACCEPTABLE */
    COAP_HCODE_PRECONDITION_FAILED_412 = COAP_HEAD_CODE(4, 12),      /* BAD_REQUEST */
    COAP_HCODE_REQUEST_ENTITY_TOO_LARGE_413 = COAP_HEAD_CODE(4, 13), /* REQUEST_ENTITY_TOO_LARGE */
    COAP_HCODE_UNSUPPORTED_MEDIA_TYPE_415 = COAP_HEAD_CODE(4, 15),   /* UNSUPPORTED_MEDIA_TYPE */

    COAP_HCODE_INTERNAL_SERVER_ERROR_500 = COAP_HEAD_CODE(5, 0),  /* INTERNAL_SERVER_ERROR */
    COAP_HCODE_NOT_IMPLEMENTED_501 = COAP_HEAD_CODE(5, 1),        /* NOT_IMPLEMENTED */
    COAP_HCODE_BAD_GATEWAY_502 = COAP_HEAD_CODE(5, 2),            /* BAD_GATEWAY */
    COAP_HCODE_SERVICE_UNAVAILABLE_503 = COAP_HEAD_CODE(5, 3),    /* SERVICE_UNAVAILABLE */
    COAP_HCODE_GATEWAY_TIMEOUT_504 = COAP_HEAD_CODE(5, 4),        /* GATEWAY_TIMEOUT */
    COAP_HCODE_PROXYING_NOT_SUPPORTED_505 = COAP_HEAD_CODE(5, 5), /* PROXYING_NOT_SUPPORTED */
} coapHeadCode_t;
#define COAP_HEAD_CODE_STRING(X)                                                                                                                \
    (                                                                                                                                           \
        (X == COAP_HCODE_EMPTY) ? "COAP_HCODE_EMPTY" : (X == COAP_HCODE_GET)                        ? "COAP_HCODE_GET"                          \
                                                   : (X == COAP_HCODE_POST)                         ? "COAP_HCODE_POST"                         \
                                                   : (X == COAP_HCODE_PUT)                          ? "COAP_HCODE_PUT"                          \
                                                   : (X == COAP_HCODE_DELETE)                       ? "COAP_HCODE_DELETE"                       \
                                                   : (X == COAP_HCODE_CREATED_201)                  ? "COAP_HCODE_CREATED_201"                  \
                                                   : (X == COAP_HCODE_DELETED_202)                  ? "COAP_HCODE_DELETED_202"                  \
                                                   : (X == COAP_HCODE_VALID_203)                    ? "COAP_HCODE_VALID_203"                    \
                                                   : (X == COAP_HCODE_CHANGED_204)                  ? "COAP_HCODE_CHANGED_204"                  \
                                                   : (X == COAP_HCODE_CONTENT_205)                  ? "COAP_HCODE_CONTENT_205"                  \
                                                   : (X == COAP_HCODE_BAD_REQUEST_400)              ? "COAP_HCODE_BAD_REQUEST_400"              \
                                                   : (X == COAP_HCODE_UNAUTHORIZED_401)             ? "COAP_HCODE_UNAUTHORIZED_401"             \
                                                   : (X == COAP_HCODE_BAD_OPTION_402)               ? "COAP_HCODE_BAD_OPTION_402"               \
                                                   : (X == COAP_HCODE_FORBIDDEN_403)                ? "COAP_HCODE_FORBIDDEN_403"                \
                                                   : (X == COAP_HCODE_NOT_FOUND_404)                ? "COAP_HCODE_NOT_FOUND_404"                \
                                                   : (X == COAP_HCODE_METHOD_NOT_ALLOWED_405)       ? "COAP_HCODE_METHOD_NOT_ALLOWED_405"       \
                                                   : (X == COAP_HCODE_NOT_ACCEPTABLE_406)           ? "COAP_HCODE_NOT_ACCEPTABLE_406"           \
                                                   : (X == COAP_HCODE_PRECONDITION_FAILED_412)      ? "COAP_HCODE_PRECONDITION_FAILED_412"      \
                                                   : (X == COAP_HCODE_REQUEST_ENTITY_TOO_LARGE_413) ? "COAP_HCODE_REQUEST_ENTITY_TOO_LARGE_413" \
                                                   : (X == COAP_HCODE_UNSUPPORTED_MEDIA_TYPE_415)   ? "COAP_HCODE_UNSUPPORTED_MEDIA_TYPE_415"   \
                                                   : (X == COAP_HCODE_INTERNAL_SERVER_ERROR_500)    ? "COAP_HCODE_INTERNAL_SERVER_ERROR_500"    \
                                                   : (X == COAP_HCODE_NOT_IMPLEMENTED_501)          ? "COAP_HCODE_NOT_IMPLEMENTED_501"          \
                                                   : (X == COAP_HCODE_BAD_GATEWAY_502)              ? "COAP_HCODE_BAD_GATEWAY_502"              \
                                                   : (X == COAP_HCODE_SERVICE_UNAVAILABLE_503)      ? "COAP_HCODE_SERVICE_UNAVAILABLE_503"      \
                                                   : (X == COAP_HCODE_GATEWAY_TIMEOUT_504)          ? "COAP_HCODE_GATEWAY_TIMEOUT_504"          \
                                                   : (X == COAP_HCODE_PROXYING_NOT_SUPPORTED_505)   ? "COAP_HCODE_PROXYING_NOT_SUPPORTED_505"   \
                                                                                                    : "Unknown")
/* CoAP header options */
typedef enum
{
    COAP_OTYPE_IF_MATCH = 1,        /* 0-8 B */
    COAP_OTYPE_URI_HOST = 3,        /* 1-255 B */
    COAP_OTYPE_ETAG = 4,            /* 1-8 B */
    COAP_OTYPE_IF_NONE_MATCH = 5,   /* 0 B */
    COAP_OTYPE_OBSERVE = 6,         /* 0-3 B */
    COAP_OTYPE_URI_PORT = 7,        /* 0-2 B */
    COAP_OTYPE_LOCATION_PATH = 8,   /* 0-255 B */
    COAP_OTYPE_URI_PATH = 11,       /* 0-255 B */
    COAP_OTYPE_CONTENT_TYPE = 12,   /* 0-2 B */
    COAP_OTYPE_MAX_AGE = 14,        /* 0-4 B */
    COAP_OTYPE_URI_QUERY = 15,      /* 0-270 B */
    COAP_OTYPE_ACCEPT = 17,         /* 0-2 B */
    COAP_OTYPE_TOKEN = 19,          /* 1-8 B */
    COAP_OTYPE_LOCATION_QUERY = 20, /* 1-270 B */
    COAP_OTYPE_BLOCK2 = 23,         /* 1-3 B */
    COAP_OTYPE_BLOCK1 = 27,         /* 1-3 B */
    COAP_OTYPE_SIZE = 28,           /* 0-4 B */
    COAP_OTYPE_PROXY_URI = 35,      /* 1-270 B */
} coapOptionType_t;
#define COAP_OPTION_TYPE_STRING(X)                                                                              \
    (                                                                                                           \
        (X == COAP_OTYPE_IF_MATCH) ? "OTYPE_IF_MATCH" : (X == COAP_OTYPE_URI_HOST)     ? "OTYPE_URI_HOST"       \
                                                    : (X == COAP_OTYPE_ETAG)           ? "OTYPE_ETAG"           \
                                                    : (X == COAP_OTYPE_IF_NONE_MATCH)  ? "OTYPE_IF_NONE_MATCH"  \
                                                    : (X == COAP_OTYPE_OBSERVE)        ? "OTYPE_OBSERVE"        \
                                                    : (X == COAP_OTYPE_URI_PORT)       ? "OTYPE_URI_PORT"       \
                                                    : (X == COAP_OTYPE_LOCATION_PATH)  ? "OTYPE_LOCATION_PATH"  \
                                                    : (X == COAP_OTYPE_URI_PATH)       ? "OTYPE_URI_PATH"       \
                                                    : (X == COAP_OTYPE_CONTENT_TYPE)   ? "OTYPE_CONTENT_TYPE"   \
                                                    : (X == COAP_OTYPE_MAX_AGE)        ? "OTYPE_MAX_AGE"        \
                                                    : (X == COAP_OTYPE_URI_QUERY)      ? "OTYPE_URI_QUERY"      \
                                                    : (X == COAP_OTYPE_ACCEPT)         ? "OTYPE_ACCEPT"         \
                                                    : (X == COAP_OTYPE_TOKEN)          ? "OTYPE_TOKEN"          \
                                                    : (X == COAP_OTYPE_LOCATION_QUERY) ? "OTYPE_LOCATION_QUERY" \
                                                    : (X == COAP_OTYPE_BLOCK2)         ? "OTYPE_BLOCK2"         \
                                                    : (X == COAP_OTYPE_BLOCK1)         ? "OTYPE_BLOCK1"         \
                                                    : (X == COAP_OTYPE_SIZE)           ? "OTYPE_SIZE"           \
                                                    : (X == COAP_OTYPE_PROXY_URI)      ? "OTYPE_PROXY_URI"      \
                                                                                       : "Unknown")

/* CoAP Content-Types */
typedef enum
{
    COAP_OCTYPE_TEXT_PLAIN = 0,
    COAP_OCTYPE_TEXT_XML = 1,
    COAP_OCTYPE_TEXT_CSV = 2,
    COAP_OCTYPE_TEXT_HTML = 3,
    COAP_OCTYPE_IMAGE_GIF = 21,
    COAP_OCTYPE_IMAGE_JPEG = 22,
    COAP_OCTYPE_IMAGE_PNG = 23,
    COAP_OCTYPE_IMAGE_TIFF = 24,
    COAP_OCTYPE_AUDIO_RAW = 25,
    COAP_OCTYPE_VIDEO_RAW = 26,
    COAP_OCTYPE_APP_LINK_FORMAT = 40,
    COAP_OCTYPE_APP_XML = 41,
    COAP_OCTYPE_APP_OCTET_STREAM = 42,
    COAP_OCTYPE_APP_RDF_XML = 43,
    COAP_OCTYPE_APP_SOAP_XML = 44,
    COAP_OCTYPE_APP_ATOM_XML = 45,
    COAP_OCTYPE_APP_XMPP_XML = 46,
    COAP_OCTYPE_APP_EXI = 47,
    COAP_OCTYPE_APP_FASTINFOSET = 48,
    COAP_OCTYPE_APP_SOAP_FASTINFOSET = 49,
    COAP_OCTYPE_APP_JSON = 50,
    COAP_OCTYPE_APP_X_OBIX_BINARY = 51,
} coapOptionContentType_t;
#define COAP_OPTION_CONTENT_TYPE_STRING(X)                                                                                          \
    (                                                                                                                               \
        (X == COAP_OCTYPE_TEXT_PLAIN) ? "OCTYPE_TEXT_PLAIN" : (X == COAP_OCTYPE_TEXT_XML)           ? "OCTYPE_TEXT_XML"             \
                                                          : (X == COAP_OCTYPE_TEXT_CSV)             ? "OCTYPE_TEXT_CSV"             \
                                                          : (X == COAP_OCTYPE_TEXT_HTML)            ? "OCTYPE_TEXT_HTML"            \
                                                          : (X == COAP_OCTYPE_IMAGE_GIF)            ? "OCTYPE_IMAGE_GIF"            \
                                                          : (X == COAP_OCTYPE_IMAGE_JPEG)           ? "OCTYPE_IMAGE_JPEG"           \
                                                          : (X == COAP_OCTYPE_IMAGE_PNG)            ? "OCTYPE_IMAGE_PNG"            \
                                                          : (X == COAP_OCTYPE_IMAGE_TIFF)           ? "OCTYPE_IMAGE_TIFF"           \
                                                          : (X == COAP_OCTYPE_AUDIO_RAW)            ? "OCTYPE_AUDIO_RAW"            \
                                                          : (X == COAP_OCTYPE_VIDEO_RAW)            ? "OCTYPE_VIDEO_RAW"            \
                                                          : (X == COAP_OCTYPE_APP_LINK_FORMAT)      ? "OCTYPE_APP_LINK_FORMAT"      \
                                                          : (X == COAP_OCTYPE_APP_XML)              ? "OCTYPE_APP_XML"              \
                                                          : (X == COAP_OCTYPE_APP_OCTET_STREAM)     ? "OCTYPE_APP_OCTET_STREAM"     \
                                                          : (X == COAP_OCTYPE_APP_RDF_XML)          ? "OCTYPE_APP_RDF_XML"          \
                                                          : (X == COAP_OCTYPE_APP_SOAP_XML)         ? "OCTYPE_APP_SOAP_XML"         \
                                                          : (X == COAP_OCTYPE_APP_ATOM_XML)         ? "OCTYPE_APP_ATOM_XML"         \
                                                          : (X == COAP_OCTYPE_APP_XMPP_XML)         ? "OCTYPE_APP_XMPP_XML"         \
                                                          : (X == COAP_OCTYPE_APP_EXI)              ? "OCTYPE_APP_EXI"              \
                                                          : (X == COAP_OCTYPE_APP_FASTINFOSET)      ? "OCTYPE_APP_FASTINFOSET"      \
                                                          : (X == COAP_OCTYPE_APP_SOAP_FASTINFOSET) ? "OCTYPE_APP_SOAP_FASTINFOSET" \
                                                          : (X == COAP_OCTYPE_APP_JSON)             ? "OCTYPE_APP_JSON"             \
                                                          : (X == COAP_OCTYPE_APP_X_OBIX_BINARY)    ? "OCTYPE_APP_X_OBIX_BINARY"    \
                                                                                                    : "Unknown")

typedef struct
{
    quint32_t ver : 2;
    coapHeadType_t type : 2;
    quint32_t tokenLen : 4;
    coapHeadCode_t code : 8;
    quint32_t mid : 16;
} coap_MessageHead_t;

typedef struct
{
    coap_MessageHead_t head;
    quint8_t token[8];
    void *optionsHead;
    struct
    {
        quint16_t len;
        void *val;
    } payload;
} Coap_Message_t;

typedef qbool (*coapRecvNotify_f)(void *chlFd, const Coap_Message_t *coapMsg, Coap_Message_t *retCoapMsg);

void Quos_coapHeadSet(Coap_Message_t *coapMsg, coapHeadType_t type, coapHeadCode_t code, quint16_t mid, quint32_t tokenLen, const quint8_t *token);
char *Quos_coapOptionGetPath(const Coap_Message_t *coapMsg);
qbool Quos_coapOptionSetPath(Coap_Message_t *coapMsg, const char *path);
qbool Quos_coapOptionSetOpaque(Coap_Message_t *coapMsg, coapOptionType_t type, const void *val, quint16_t valLen);
qbool Quos_coapOptionGetOpaque(Coap_Message_t *coapMsg, coapOptionType_t type, const void **val, quint16_t *valLen);
qbool Quos_coapOptionSetNumber(Coap_Message_t *coapMsg, coapOptionType_t type, quint32_t number);
qbool Quos_coapOptionGetNumber(Coap_Message_t *coapMsg, coapOptionType_t type, quint32_t *number);
void Quos_coapPayloadSet(Coap_Message_t *coapMsg, const void *val, quint16_t valLen);
void Quos_coapMessageFree(Coap_Message_t *coapMsg);
qbool Quos_coapInit(void **chlFdPoint, const char *url, coapRecvNotify_f notifyCb);
qbool Quos_coapMsgSend(void *chlFd, const char *path, Coap_Message_t *coapMsg, socketRecvNodeCb_f recvCB, qbool isAck);

#endif
#endif
