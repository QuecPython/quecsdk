/*************************************************************************
**    @author  : 
**    @version : 
**    @date    : 
**    @brief   : 注意文件使用  UTF-8 编码
**    @hardware :
**    @other   :
***************************************************************************/
#include "Ql_iotApi.h"
#include "py/obj.h"
#include "py/objlist.h"
#include "py/objtuple.h"
#include "py/objstr.h"
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mpz.h"
#include "py/objint.h"
#include "mpconfigport.h"
//#include "helios_os.h"

extern quint32_t Quos_stringSplit(char *src, quint32_t srcLen, char **words, quint32_t maxSize, const char *delim, qbool keepEmptyParts);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdBusPassTransSend(mp_obj_t mp_mode, mp_obj_t mp_data)
{
	int mode = mp_obj_get_int(mp_mode);
	mp_buffer_info_t data = {0};
	mp_get_buffer_raise(mp_data, &data, MP_BUFFER_READ);
	if (FALSE == Ql_iotCmdBusPassTransSend(mode, (quint8_t *)data.buf, data.len))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(qpy_Ql_iotCmdBusPassTransSend_obj, qpy_Ql_iotCmdBusPassTransSend);

//mpz -> 64bit integer for 32bit builds
static qint64_t mpz_to_64bit_int(const mp_obj_int_t *arg, bool is_signed)
{
	//see mpz_as_int_checked
	const qint64_t maxCalcThreshold = is_signed ? 140737488355327 : 281474976710655;

	const mpz_t *i = &arg->mpz;
	if (!is_signed && i->neg)
	{
		mp_raise_TypeError(MP_ERROR_TEXT("Source integer must be unsigned"));
	}

	short unsigned int *d = i->dig + i->len;
	qint64_t val = 0;

	while (d-- > i->dig)
	{
		if (val > maxCalcThreshold)
		{
			mp_raise_ValueError(MP_ERROR_TEXT("Value too large for 64bit integer"));
		}
		val = (val << MPZ_DIG_SIZE) | *d;
	}

	if (i->neg)
	{
		val = -val;
	}
	return val;
}

static qint64_t mp_obj_get_lint(mp_const_obj_t arg)
{
	if (arg == mp_const_false)
	{
		return 0u;
	}
	else if (arg == mp_const_true)
	{
		return 1u;
	}
	else if (MP_OBJ_IS_SMALL_INT(arg))
	{
		return MP_OBJ_SMALL_INT_VALUE(arg);
	}
	else if (MP_OBJ_IS_TYPE(arg, &mp_type_int))
	{
		return mpz_to_64bit_int((mp_obj_int_t *)arg, 1);
	}
	else
	{
		mp_raise_TypeError(MP_ERROR_TEXT("unsigned integer"));
	}
	return 0u;
}

static qbool phy_dict_handle(mp_obj_t mp_data, void **ttlvHead);
static qbool phy_list_handle(mp_obj_t mp_data, void **ttlvHead)
{
	size_t len = 0;
	mp_obj_t *items = NULL;
	mp_obj_list_get(mp_data, &len, &items);
	if (len == 0)
	{
		return FALSE;
	}
	for (quint32_t i = 0; i < len; i++)
	{
		mp_obj_t id = mp_obj_new_int(0);
		mp_obj_t value = items[i];
		if (mp_obj_is_bool(value))
		{
			Ql_iotTtlvIdAddBool(ttlvHead, mp_obj_get_int(id), value == mp_const_true ? TRUE : FALSE);
		}
		else if (mp_obj_is_int(value))
		{
			Ql_iotTtlvIdAddInt(ttlvHead, mp_obj_get_int(id), mp_obj_get_lint(value));
		}
		else if (mp_obj_is_float(value))
		{
			Ql_iotTtlvIdAddFloat(ttlvHead, mp_obj_get_int(id), mp_obj_get_float(value));
		}
		else if (mp_obj_is_str(value) || mp_obj_is_type(value, &mp_type_bytearray) || mp_obj_is_type(value, &mp_type_bytes))
		{
			mp_buffer_info_t data = {0};
			mp_get_buffer_raise(value, &data, MP_BUFFER_READ);
			Ql_iotTtlvIdAddByte(ttlvHead, mp_obj_get_int(id), data.buf, data.len);
		}
		else if (mp_obj_is_type(value, &mp_type_dict))
		{
			void *ttlvStructHead = NULL;
			phy_dict_handle(value, &ttlvStructHead);
			Ql_iotTtlvIdAddStruct(ttlvHead, mp_obj_get_int(id), ttlvStructHead);
		}
		else if (mp_obj_is_type(value, &mp_type_list) || mp_obj_is_type(value, &mp_type_tuple))
		{
			void *ttlvStructHead = NULL;
			phy_list_handle(value, &ttlvStructHead);
			Ql_iotTtlvIdAddStruct(ttlvHead, mp_obj_get_int(id), ttlvStructHead);
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}

static qbool phy_dict_handle(mp_obj_t mp_data, void **ttlvHead)
{
	quint32_t index = 0;
	mp_map_t *map = NULL;
	mp_obj_t id = 0;
	mp_obj_t value = 0;
	if (!mp_obj_is_type(mp_data, &mp_type_dict))
	{
		return FALSE;
	}

	map = mp_obj_dict_get_map(mp_data);
	if (map == NULL)
	{
		return FALSE;
	}

	for (; index < map->alloc; index++)
	{
		id = map->table[index].key;
		value = map->table[index].value;

		if (id == MP_OBJ_NULL)
		{
			if (index + (quint32_t)1 < map->alloc)
			{
				continue;
			}
			else if (index > 0)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		if (mp_obj_is_bool(value))
		{
			Ql_iotTtlvIdAddBool(ttlvHead, mp_obj_get_int(id), value == mp_const_true ? TRUE : FALSE);
		}
		else if (mp_obj_is_int(value))
		{
			Ql_iotTtlvIdAddInt(ttlvHead, mp_obj_get_int(id), mp_obj_get_lint(value));
		}
		else if (mp_obj_is_float(value))
		{
			Ql_iotTtlvIdAddFloat(ttlvHead, mp_obj_get_int(id), mp_obj_get_float(value));
		}
		else if (mp_obj_is_str(value) || mp_obj_is_type(value, &mp_type_bytearray) || mp_obj_is_type(value, &mp_type_bytes))
		{
			mp_buffer_info_t data = {0};
			mp_get_buffer_raise(value, &data, MP_BUFFER_READ);
			Ql_iotTtlvIdAddByte(ttlvHead, mp_obj_get_int(id), data.buf, data.len);
		}
		else if (mp_obj_is_type(value, &mp_type_dict))
		{
			void *ttlvStructHead = NULL;
			phy_dict_handle(value, &ttlvStructHead);
			Ql_iotTtlvIdAddStruct(ttlvHead, mp_obj_get_int(id), ttlvStructHead);
		}
		else if (mp_obj_is_type(value, &mp_type_list) || mp_obj_is_type(value, &mp_type_tuple))
		{
			void *ttlvStructHead = NULL;
			phy_list_handle(value, &ttlvStructHead);
			Ql_iotTtlvIdAddStruct(ttlvHead, mp_obj_get_int(id), ttlvStructHead);
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdBusPhymodelReport(mp_obj_t mp_mode, mp_obj_t mp_data)
{
	int mode = mp_obj_get_int(mp_mode);
	mp_obj_t ret = mp_const_true;
	void *ttlvHead = NULL;
	if (phy_dict_handle(mp_data, &ttlvHead))
	{
		if (FALSE == Ql_iotCmdBusPhymodelReport(mode, ttlvHead))
		{
			ret = mp_const_false;
		}
	}
	else
	{
		ret = mp_const_false;
	}
	Ql_iotTtlvFree(&ttlvHead);
	return ret;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(qpy_Ql_iotCmdBusPhymodelReport_obj, qpy_Ql_iotCmdBusPhymodelReport);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdBusPhymodelAck(mp_obj_t mp_mode, mp_obj_t mp_pkgid, mp_obj_t mp_data)
{
	int mode = mp_obj_get_int(mp_mode);
	int pkgid = mp_obj_get_int(mp_pkgid);
	mp_obj_t ret = mp_const_true;
	void *ttlvHead = NULL;
	if (phy_dict_handle(mp_data, &ttlvHead))
	{
		if (FALSE == Ql_iotCmdBusPhymodelAck(mode, (quint16_t)pkgid, ttlvHead))
		{
			ret = mp_const_false;
		}
	}
	else
	{
		ret = mp_const_false;
	}
	Ql_iotTtlvFree(&ttlvHead);
	return ret;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(qpy_Ql_iotCmdBusPhymodelAck_obj, qpy_Ql_iotCmdBusPhymodelAck);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdOtaAction(mp_obj_t mp_action)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	int action = mp_obj_get_int(mp_action);
	if (FALSE == Ql_iotCmdOtaAction(action))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotCmdOtaAction_obj, qpy_Ql_iotCmdOtaAction);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdOtaRequest(mp_obj_t mp_mode)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	int mode = mp_obj_get_int(mp_mode);
	if (FALSE == Ql_iotCmdOtaRequest(mode))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotCmdOtaRequest_obj, qpy_Ql_iotCmdOtaRequest);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdOtaMcuFWDataRead(mp_obj_t mp_addr, mp_obj_t mp_len)
{
	quint32_t addr = mp_obj_get_int(mp_addr);
	quint32_t len = mp_obj_get_int(mp_len);
	// 2021-06-18 限制SOTA读取长度。因为读取长度过长，会导致模组dump。而且读取超过固件大小的长度也会先行申请内存，容易dump。
	if (Ql_iotGetWorkState() == 0 || len == 0 /* || len > Helios_GetAvailableMemorySize() / 3*/)
	{
		return mp_const_none;
	}
	void *data = NULL;
	quint32_t ret = 0;

	data = malloc(len);
	if (data == NULL)
	{
		mp_raise_OSError(MP_ENOMEM);
		return mp_const_none;
	}
	if ((ret = Ql_iotCmdOtaMcuFWDataRead(addr, (quint8_t *)data, len)) > 0)
	{
		mp_obj_t retData = mp_obj_new_bytes((const unsigned char *)data, ret);
		free(data);
		return retData;
	}
	free(data);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(qpy_Ql_iotCmdOtaMcuFWDataRead_obj, qpy_Ql_iotCmdOtaMcuFWDataRead);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdSysGetDevStatus(mp_obj_t id_list)
{
	size_t count = 0;
	mp_obj_t *items = NULL;
	mp_obj_get_array(id_list, &count, &items);
	count = count < QIOT_DPID_STATUS_MAX ? count : QIOT_DPID_STATUS_MAX;
	mp_obj_t struct_dict = mp_obj_new_dict(count);
	quint16_t ids[QIOT_DPID_STATUS_MAX];
	size_t i;
	for (i = 0; i < count; i++)
	{
		ids[i] = mp_obj_get_int(items[i]);
	}
	void *statusTtlv = Ql_iotSysGetDevStatus(ids, count);

	i = 0;
	void *node;
	quint16_t id;
	QIot_dpDataType_e type;
	while ((node = Ql_iotTtlvNodeGet(statusTtlv, i++, &id, &type)))
	{
		switch (type)
		{
		case QIOT_DPDATA_TYPE_BOOL:
		{
			qbool value;
			if (Ql_iotTtlvNodeGetBool(node, &value))
			{
				mp_obj_dict_store(struct_dict, mp_obj_new_int(id), mp_obj_new_bool(value));
			}
			break;
		}
		case QIOT_DPDATA_TYPE_INT:
		{
			qint64_t value;
			if (Ql_iotTtlvNodeGetInt(node, &value))
			{
				mp_obj_dict_store(struct_dict, mp_obj_new_int(id), mp_obj_new_int(value));
			}
			break;
		}
		case QIOT_DPDATA_TYPE_FLOAT:
		{
			double value;
			if (Ql_iotTtlvNodeGetFloat(node, &value))
			{
				mp_obj_dict_store(struct_dict, mp_obj_new_int(id), mp_obj_new_float(value));
			}
			break;
		}
		case QIOT_DPDATA_TYPE_BYTE:
		{
			quint8_t *value;
			quint32_t len = Ql_iotTtlvNodeGetByte(node, &value);
			if (len)
			{
				mp_obj_dict_store(struct_dict, mp_obj_new_int(id), mp_obj_new_str((const char *)value, len));
			}
			break;
		}
		case QIOT_DPDATA_TYPE_STRUCT:
			break;
		}
	}

	Ql_iotTtlvFree(&statusTtlv);
	return struct_dict;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotCmdSysGetDevStatus_obj, qpy_Ql_iotCmdSysGetDevStatus);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdSysGetDevInfo(mp_obj_t id_list)
{
	size_t count = 0;
	mp_obj_t *items = NULL;
	mp_obj_get_array(id_list, &count, &items);
	count = count < QIOT_DPID_INFO_MAX ? count : QIOT_DPID_INFO_MAX;
	mp_obj_t struct_dict = mp_obj_new_dict(count);

	quint16_t ids[QIOT_DPID_INFO_MAX];
	size_t i;
	for (i = 0; i < count; i++)
	{
		ids[i] = mp_obj_get_int(items[i]);
	}
	void *statusTtlv = Ql_iotSysGetDevInfo(ids, count);

	i = 0;
	void *node;
	quint16_t id;
	QIot_dpDataType_e type;
	while ((node = Ql_iotTtlvNodeGet(statusTtlv, i++, &id, &type)))
	{
		switch (type)
		{
		case QIOT_DPDATA_TYPE_BOOL:
		{
			qbool value;
			if (Ql_iotTtlvNodeGetBool(node, &value))
			{
				mp_obj_dict_store(struct_dict, mp_obj_new_int(id), mp_obj_new_bool(value));
			}
			break;
		}
		case QIOT_DPDATA_TYPE_INT:
		{
			qint64_t value;
			if (Ql_iotTtlvNodeGetInt(node, &value))
			{
				mp_obj_dict_store(struct_dict, mp_obj_new_int(id), mp_obj_new_int(value));
			}
			break;
		}
		case QIOT_DPDATA_TYPE_FLOAT:
		{
			double value;
			if (Ql_iotTtlvNodeGetFloat(node, &value))
			{
				mp_obj_dict_store(struct_dict, mp_obj_new_int(id), mp_obj_new_float(value));
			}
			break;
		}
		case QIOT_DPDATA_TYPE_BYTE:
		{
			quint8_t *value;
			quint32_t len = Ql_iotTtlvNodeGetByte(node, &value);
			if (len)
			{
				mp_obj_dict_store(struct_dict, mp_obj_new_int(id), mp_obj_new_str((const char *)value, len));
			}
			break;
		}
		case QIOT_DPDATA_TYPE_STRUCT:
			break;
		}
	}

	Ql_iotTtlvFree(&statusTtlv);
	return struct_dict;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotCmdSysGetDevInfo_obj, qpy_Ql_iotCmdSysGetDevInfo);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdSysStatusReport(mp_obj_t id_list)
{
	qbool ret = FALSE;
	size_t count = 0;
	mp_obj_t *items = NULL;
	quint16_t *ids = NULL;
	mp_obj_get_array(id_list, &count, &items);
	if (count && (ids = malloc(count * sizeof(sizeof(quint16_t)))) != NULL)
	{
		size_t i;
		for (i = 0; i < count; i++)
		{
			ids[i] = mp_obj_get_int(items[i]);
		}
		ret = Ql_iotCmdSysStatusReport(ids, count);
		free(ids);
	}
	return mp_obj_new_bool(ret ? TRUE : FALSE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotCmdSysStatusReport_obj, qpy_Ql_iotCmdSysStatusReport);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdSysDevInfoReport(mp_obj_t id_list)
{
	qbool ret = FALSE;
	size_t count = 0;
	mp_obj_t *items = NULL;
	quint16_t *ids = NULL;
	mp_obj_get_array(id_list, &count, &items);
	if (count && (ids = malloc(count * sizeof(sizeof(quint16_t)))) != NULL)
	{
		size_t i;
		for (i = 0; i < count; i++)
		{
			ids[i] = mp_obj_get_int(items[i]);
		}
		ret = Ql_iotCmdSysDevInfoReport(ids, count);
		free(ids);
	}
	return mp_obj_new_bool(ret ? TRUE : FALSE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotCmdSysDevInfoReport_obj, qpy_Ql_iotCmdSysDevInfoReport);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotInit(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		if (TRUE == Ql_iotInit())
		{
			return mp_const_true;
		}
		return mp_const_false;
	}
	else
	{
		Ql_iotConfigSetConnmode(0);
	}
	return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotInit_obj, qpy_Ql_iotInit);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetConnmode(mp_obj_t mode)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	if (FALSE == Ql_iotConfigSetConnmode(mp_obj_get_int(mode)))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotConfigSetConnmode_obj, qpy_Ql_iotConfigSetConnmode);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigGetConnmode(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_none;
	}
	QIot_connMode_e mode = Ql_iotConfigGetConnmode();
	return mp_obj_new_int(mode);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotConfigGetConnmode_obj, qpy_Ql_iotConfigGetConnmode);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetPdpContextId(mp_obj_t context_id)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_false;
	}
	if (FALSE == Ql_iotConfigSetPdpContextId((quint8_t)mp_obj_get_int(context_id)))
	{
		return mp_const_false;
	}
	return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotConfigSetPdpContextId_obj, qpy_Ql_iotConfigSetPdpContextId);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigGetPdpContextId(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_none;
	}
	quint8_t contextid = Ql_iotConfigGetPdpContextId();
	return mp_obj_new_int(contextid);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotConfigGetPdpContextId_obj, qpy_Ql_iotConfigGetPdpContextId);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetServer(mp_obj_t type, mp_obj_t server)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	mp_buffer_info_t serverinfo = {0};
	mp_get_buffer_raise(server, &serverinfo, MP_BUFFER_READ);
	if (FALSE == Ql_iotConfigSetServer(mp_obj_get_int(type), (const char *)serverinfo.buf))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(qpy_Ql_iotConfigSetServer_obj, qpy_Ql_iotConfigSetServer);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigGetServer(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_none;
	}
	int type = 0;
	char *server = NULL;

	Ql_iotConfigGetServer((QIot_protocolType_t *)&type, &server);
	mp_obj_t url_info[2] =
		{
			mp_obj_new_int(type),
			mp_obj_new_str(server, strlen(server)),
		};
	return mp_obj_new_tuple(2, url_info);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotConfigGetServer_obj, qpy_Ql_iotConfigGetServer);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetProductinfo(mp_obj_t product_key, mp_obj_t product_secret)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	mp_buffer_info_t keyinfo = {0};
	mp_buffer_info_t secretinfo = {0};
	mp_get_buffer_raise(product_key, &keyinfo, MP_BUFFER_READ);
	mp_get_buffer_raise(product_secret, &secretinfo, MP_BUFFER_READ);
	if (FALSE == Ql_iotConfigSetProductinfo((const char *)keyinfo.buf, (const char *)secretinfo.buf))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(qpy_Ql_iotConfigSetProductinfo_obj, qpy_Ql_iotConfigSetProductinfo);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigGetProductinfo(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_none;
	}
	char *product_key = NULL;
	char *product_secret = NULL;
	char *product_ver = NULL;

	Ql_iotConfigGetProductinfo(&product_key, &product_secret, &product_ver);
	mp_obj_t product_info[3] =
		{
			mp_obj_new_str(product_key, strlen(product_key)),
			mp_obj_new_str(product_secret, strlen(product_secret)),
			mp_obj_new_str(product_ver, strlen(product_ver))};
	return mp_obj_new_tuple(3, product_info);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotConfigGetProductinfo_obj, qpy_Ql_iotConfigGetProductinfo);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetLifetime(mp_obj_t life_time)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	int lifetime = mp_obj_get_int(life_time);
	if (FALSE == Ql_iotConfigSetLifetime(lifetime))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotConfigSetLifetime_obj, qpy_Ql_iotConfigSetLifetime);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigGetLifetime(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_none;
	}
	quint32_t lifetime = Ql_iotConfigGetLifetime();
	return mp_obj_new_int_from_uint(lifetime);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotConfigGetLifetime_obj, qpy_Ql_iotConfigGetLifetime);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetSessionFlag(mp_obj_t sessionFlag)
{
	if (Ql_iotGetWorkState() == 0 || FALSE == mp_obj_is_bool(sessionFlag))
	{
		return mp_obj_new_bool(FALSE);
	}
	if (FALSE == Ql_iotConfigSetSessionFlag(mp_const_true == sessionFlag ? TRUE : FALSE))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotConfigSetSessionFlag_obj, qpy_Ql_iotConfigSetSessionFlag);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigGetSessionFlag(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_none;
	}
	return mp_obj_new_bool(Ql_iotConfigGetSessionFlag());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotConfigGetSessionFlag_obj, qpy_Ql_iotConfigGetSessionFlag);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigGetSoftVersion(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_none;
	}
	char *ver = Ql_iotConfigGetSoftVersion();
	return mp_obj_new_str(ver, strlen(ver));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotConfigGetSoftVersion_obj, qpy_Ql_iotConfigGetSoftVersion);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetMcuVersion(mp_obj_t mp_compno, mp_obj_t mp_version)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	mp_buffer_info_t mcu_compno = {0};
	mp_buffer_info_t mcu_version = {0};
	mp_get_buffer_raise(mp_compno, &mcu_compno, MP_BUFFER_READ);
	mp_get_buffer_raise(mp_version, &mcu_version, MP_BUFFER_READ);
	if (FALSE == Ql_iotConfigSetMcuVersion((const char *)mcu_compno.buf, (const char *)mcu_version.buf))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(qpy_Ql_iotConfigSetMcuVersion_obj, qpy_Ql_iotConfigSetMcuVersion);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigGetMcuVersion(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_none;
	}
	char *temp = NULL;
	char *oldVer = NULL;
	mp_obj_t objList = mp_obj_new_list(0, NULL);
	Ql_iotConfigGetMcuVersion(NULL, &oldVer);
	temp = malloc(strlen(oldVer) + 1);
	if (temp)
	{
		char *words[100];
		sprintf(temp, "%s", oldVer);
		quint32_t count = Quos_stringSplit(temp, strlen(temp), words, sizeof(words) / sizeof(words[0]), ";", FALSE);
		while (count--)
		{
			char *wordsnode[2];
			qint32_t nodeLen = Quos_stringSplit(words[count], strlen(words[count]), wordsnode, sizeof(wordsnode) / sizeof(wordsnode[0]), ":", FALSE);
			if (2 == nodeLen)
			{
				mp_obj_t node[] =
					{
						mp_obj_new_str(wordsnode[0], strlen(wordsnode[0])),
						mp_obj_new_str(wordsnode[1], strlen(wordsnode[1]))};
				mp_obj_list_append(objList, mp_obj_new_tuple(sizeof(node) / sizeof(node[0]), node));
			}
		}
		free(temp);
	}
	return objList;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotConfigGetMcuVersion_obj, qpy_Ql_iotConfigGetMcuVersion);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetDkDs(mp_obj_t dk, mp_obj_t ds)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	mp_buffer_info_t dkData = {0}, dsData = {0};
	mp_get_buffer_raise(dk, &dkData, MP_BUFFER_READ);
	mp_get_buffer_raise(ds, &dsData, MP_BUFFER_READ);
	if (FALSE == Ql_iotConfigSetDkDs((const char *)dkData.buf, (const char *)dsData.buf))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(qpy_Ql_iotConfigSetDkDs_obj, qpy_Ql_iotConfigSetDkDs);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigGetDkDs(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_none;
	}
	char *dk, *ds;
	if (FALSE == Ql_iotConfigGetDkDs(&dk, &ds))
	{
		return mp_const_none;
	}
	mp_obj_t dkds[2] =
		{
			mp_obj_new_str(dk, strlen(dk)),
			mp_obj_new_str(ds, strlen(ds))};
	return mp_obj_new_tuple(2, dkds);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotConfigGetDkDs_obj, qpy_Ql_iotConfigGetDkDs);
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static qbool ttlv_dict_handle(const void *ttlv_head, quint32_t count, mp_obj_t node_dict);
static qbool ttlv_array_handle(const void *ttlv_head, quint32_t count, mp_obj_t node_list)
{
	for (quint32_t i = 0; i < count; i++)
	{
		uint16_t id = 0;
		int type = 0;
		void *node = Ql_iotTtlvNodeGet(ttlv_head, i, &id, (QIot_dpDataType_e *)&type);
		if (node)
		{
			switch (type)
			{
			case QIOT_DPDATA_TYPE_BOOL:
			{
				qbool bool_value;
				Ql_iotTtlvNodeGetBool(node, &bool_value);
				mp_obj_list_append(node_list, mp_obj_new_bool(bool_value));
				break;
			}
			case QIOT_DPDATA_TYPE_INT:
			{
				qint64_t num_value;
				Ql_iotTtlvNodeGetInt(node, &num_value);
				mp_obj_list_append(node_list, mp_obj_new_int_from_ll((long long)num_value));
				break;
			}
			case QIOT_DPDATA_TYPE_FLOAT:
			{
				double num_value;
				Ql_iotTtlvNodeGetFloat(node, &num_value);
				mp_obj_list_append(node_list, mp_obj_new_float((mp_float_t)num_value));
				break;
			}
			case QIOT_DPDATA_TYPE_BYTE:
			{
				quint8_t *value;
				quint32_t len = Ql_iotTtlvNodeGetByte(node, &value);
				mp_obj_list_append(node_list, mp_obj_new_bytes(value, len));
				break;
			}
			case QIOT_DPDATA_TYPE_STRUCT:
			{
				quint32_t struct_count = Ql_iotTtlvCountGet(node);
				uint16_t temp_id = 0;
				int temp_type = 0;
				Ql_iotTtlvNodeGet(Ql_iotTtlvNodeGetStruct(node), 0, &temp_id, (QIot_dpDataType_e *)&temp_type);
				if (temp_id == 0)
				{
					mp_obj_t struct_list = mp_obj_new_list(0, NULL);
					ttlv_array_handle(Ql_iotTtlvNodeGetStruct(node), struct_count, struct_list);
					mp_obj_list_append(node_list, struct_list);
				}
				else
				{
					mp_obj_t struct_dict = mp_obj_new_dict(struct_count);
					ttlv_dict_handle(Ql_iotTtlvNodeGetStruct(node), struct_count, struct_dict);
					mp_obj_list_append(node_list, struct_dict);
				}
				break;
			}
			default:
				return FALSE;
				break;
			}
		}
	}
	return TRUE;
}
static qbool ttlv_dict_handle(const void *ttlv_head, quint32_t count, mp_obj_t node_dict)
{
	for (quint32_t i = 0; i < count; i++)
	{
		uint16_t id = 0;
		int type = 0;
		void *node = Ql_iotTtlvNodeGet(ttlv_head, i, &id, (QIot_dpDataType_e *)&type);
		if (node)
		{
			switch (type)
			{
			case QIOT_DPDATA_TYPE_BOOL:
			{
				qbool bool_value;
				Ql_iotTtlvNodeGetBool(node, &bool_value);
				mp_obj_dict_store(node_dict, mp_obj_new_int(id), mp_obj_new_bool(bool_value));
				break;
			}
			case QIOT_DPDATA_TYPE_INT:
			{
				qint64_t num_value;
				Ql_iotTtlvNodeGetInt(node, &num_value);
				mp_obj_dict_store(node_dict, mp_obj_new_int(id), mp_obj_new_int_from_ll((long long)num_value));
				break;
			}
			case QIOT_DPDATA_TYPE_FLOAT:
			{
				double num_value;
				Ql_iotTtlvNodeGetFloat(node, &num_value);
				mp_obj_dict_store(node_dict, mp_obj_new_int(id), mp_obj_new_float((mp_float_t)num_value));
				break;
			}
			case QIOT_DPDATA_TYPE_BYTE:
			{
				quint8_t *value;
				quint32_t len = Ql_iotTtlvNodeGetByte(node, &value);
				mp_obj_dict_store(node_dict, mp_obj_new_int(id), mp_obj_new_bytes(value, len));
				break;
			}
			case QIOT_DPDATA_TYPE_STRUCT:
			{
				quint32_t struct_count = Ql_iotTtlvCountGet(Ql_iotTtlvNodeGetStruct(node));
				uint16_t temp_id = 0;
				int temp_type = 0;
				Ql_iotTtlvNodeGet(Ql_iotTtlvNodeGetStruct(node), 0, &temp_id, (QIot_dpDataType_e *)&temp_type);
				if (temp_id == 0)
				{
					mp_obj_t struct_list = mp_obj_new_list(0, NULL);
					ttlv_array_handle(Ql_iotTtlvNodeGetStruct(node), struct_count, struct_list);
					mp_obj_dict_store(node_dict, mp_obj_new_int(id), struct_list);
				}
				else
				{
					mp_obj_t struct_dict = mp_obj_new_dict(struct_count);
					ttlv_dict_handle(Ql_iotTtlvNodeGetStruct(node), struct_count, struct_dict);
					mp_obj_dict_store(node_dict, mp_obj_new_int(id), struct_dict);
				}
				break;
			}
			default:
				return FALSE;
				break;
			}
		}
	}
	return TRUE;
}

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
static c_callback_t *Ql_iotUrcEventCB = NULL;
static void ql_iotEventCB(quint32_t event, qint32_t errcode, const void *valueT, quint32_t valLen)
{
	if (NULL == Ql_iotUrcEventCB || (event == 7 && errcode == 10702))
	{
		return;
	}
	if (NULL == valueT)
	{
		mp_obj_t tuple[] =
			{
				mp_obj_new_int_from_uint(event),
				mp_obj_new_int_from_uint(errcode)};
		mp_sched_schedule_ex(Ql_iotUrcEventCB, mp_obj_new_tuple(2, tuple));
	}
	else if (QIOT_ATEVENT_TYPE_RECV == event && QIOT_RECV_SUCC_PHYMODEL_RECV == errcode)
	{
		quint32_t count = Ql_iotTtlvCountGet(valueT);
		mp_obj_t node_dict = mp_obj_new_dict(count);
		if (ttlv_dict_handle(valueT, count, node_dict))
		{
			mp_obj_t tuple[] =
				{
					mp_obj_new_int_from_uint(event),
					mp_obj_new_int_from_uint(errcode),
					node_dict};
			mp_sched_schedule_ex(Ql_iotUrcEventCB, mp_obj_new_tuple(3, tuple));
		}
	}
	else if (QIOT_ATEVENT_TYPE_RECV == event && QIOT_RECV_SUCC_PHYMODEL_REQ == errcode)
	{
		quint16_t pkgId = *(quint16_t *)valueT;
		quint16_t *ids = (quint16_t *)((quint8_t *)valueT + sizeof(quint16_t));
		mp_obj_t req_list = mp_obj_new_list(0, NULL);
		mp_obj_t req_list_temp = mp_obj_new_list(0, NULL);
		mp_obj_list_append(req_list, mp_obj_new_int((mp_int_t)pkgId));
		for (quint32_t i = 0; i < valLen; i++)
		{
			quint16_t modelId = ids[i];
			mp_obj_list_append(req_list_temp, mp_obj_new_int((mp_int_t)modelId));
		}
		mp_obj_list_append(req_list, req_list_temp);
		mp_obj_t tuple[] =
			{
				mp_obj_new_int_from_uint(event),
				mp_obj_new_int_from_uint(errcode),
				req_list};
		mp_sched_schedule_ex(Ql_iotUrcEventCB, mp_obj_new_tuple(3, tuple));
	}
	else
	{
		mp_obj_t tuple[] =
			{
				mp_obj_new_int_from_uint(event),
				mp_obj_new_int_from_uint(errcode),
				mp_obj_new_bytes(valueT, valLen)};
		mp_sched_schedule_ex(Ql_iotUrcEventCB, mp_obj_new_tuple(3, tuple));
	}
}
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetEventCB(mp_obj_t event_urc_cb)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	static c_callback_t cb = {0};
	Ql_iotUrcEventCB = &cb;
	mp_sched_schedule_callback_register(Ql_iotUrcEventCB, event_urc_cb);
	Ql_iotConfigSetEventCB(ql_iotEventCB);
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotConfigSetEventCB_obj, qpy_Ql_iotConfigSetEventCB);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotGetWorkState(void)
{
	quint32_t state = Ql_iotGetWorkState();
	return mp_obj_new_int_from_uint(state);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotGetWorkState_obj, qpy_Ql_iotGetWorkState);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdGetLocSupList(void)
{
	mp_obj_t supList = mp_obj_new_list(0, NULL);
	void *titleTtlv = Ql_iotLocGetSupList();
	quint32_t count = Ql_iotTtlvCountGet(titleTtlv);
	quint32_t i;
	for (i = 0; i < count; i++)
	{
		char *str = Ql_iotTtlvNodeGetString(Ql_iotTtlvNodeGet(titleTtlv, i, NULL, NULL));
		if (str)
		{
			mp_obj_list_append(supList, mp_obj_new_str(str, strlen(str)));
		}
	}
	Ql_iotTtlvFree(&titleTtlv);

	return supList;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotCmdGetLocSupList_obj, qpy_Ql_iotCmdGetLocSupList);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdGetLocData(mp_obj_t mp_title)
{
	mp_obj_t locDatas = mp_obj_new_list(0, NULL);
	if (mp_obj_is_type(mp_title, &mp_type_list) || mp_obj_is_type(mp_title, &mp_type_tuple))
	{
		quint32_t i;
		size_t len = 0;
		void *titleTtlv = NULL;
		mp_obj_t *items = NULL;
		mp_buffer_info_t itemstr = {0};
		mp_obj_list_get(mp_title, &len, &items);
		for (i = 0; i < len; i++)
		{
			mp_get_buffer_raise(items[i], &itemstr, MP_BUFFER_READ);
			Ql_iotTtlvIdAddString(&titleTtlv, 0, itemstr.buf);
		}
		void *nmeaTtlv = Ql_iotLocGetData(titleTtlv);
		Ql_iotTtlvFree(&titleTtlv);

		quint32_t count = Ql_iotTtlvCountGet(nmeaTtlv);
		for (i = 0; i < count; i++)
		{
			char *str = Ql_iotTtlvNodeGetString(Ql_iotTtlvNodeGet(nmeaTtlv, i, NULL, NULL));
			if (str)
			{
				mp_obj_list_append(locDatas, mp_obj_new_str(str, strlen(str)));
			}
		}
		Ql_iotTtlvFree(&nmeaTtlv);
	}
	return locDatas;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotCmdGetLocData_obj, qpy_Ql_iotCmdGetLocData);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdBusLocReportInside(mp_obj_t mp_title)
{
	mp_obj_t ret = mp_const_false;
	if (mp_obj_is_type(mp_title, &mp_type_list) || mp_obj_is_type(mp_title, &mp_type_tuple))
	{
		quint8_t i;
		size_t len = 0;
		void *titleTtlv = NULL;
		mp_obj_t *items = NULL;
		mp_buffer_info_t itemstr = {0};
		mp_obj_list_get(mp_title, &len, &items);
		for (i = 0; i < len; i++)
		{
			mp_get_buffer_raise(items[i], &itemstr, MP_BUFFER_READ);
			Ql_iotTtlvIdAddString(&titleTtlv, 0, itemstr.buf);
		}
		if (Ql_iotCmdBusLocReportInside(titleTtlv))
		{
			ret = mp_const_true;
		}
		Ql_iotTtlvFree(&titleTtlv);
	}
	else
	{
		ret = mp_const_false;
	}
	return ret;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotCmdBusLocReportInside_obj, qpy_Ql_iotCmdBusLocReportInside);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotCmdBusLocReportOutside(mp_obj_t mp_nmea)
{
	mp_obj_t ret = mp_const_false;
	if (mp_obj_is_type(mp_nmea, &mp_type_list) || mp_obj_is_type(mp_nmea, &mp_type_tuple))
	{
		quint8_t i;
		size_t len = 0;
		void *titleTtlv = NULL;
		mp_obj_t *items = NULL;
		mp_buffer_info_t itemstr = {0};
		mp_obj_list_get(mp_nmea, &len, &items);
		for (i = 0; i < len; i++)
		{
			mp_get_buffer_raise(items[i], &itemstr, MP_BUFFER_READ);
			Ql_iotTtlvIdAddString(&titleTtlv, 0, itemstr.buf);
		}
		if (Ql_iotCmdBusLocReportOutside(titleTtlv))
		{
			ret = mp_const_true;
		}
		Ql_iotTtlvFree(&titleTtlv);
	}
	else
	{
		ret = mp_const_false;
	}
	return ret;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotCmdBusLocReportOutside_obj, qpy_Ql_iotCmdBusLocReportOutside);
#ifdef QUEC_ENABLE_HTTP_OTA
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetHttpOtaProductInfo(mp_obj_t product_key, mp_obj_t product_secret)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	mp_buffer_info_t keyinfo = {0};
	mp_buffer_info_t secretinfo = {0};
	mp_get_buffer_raise(product_key, &keyinfo, MP_BUFFER_READ);
	mp_get_buffer_raise(product_secret, &secretinfo, MP_BUFFER_READ);
	if (FALSE == Ql_iotConfigSetHttpOtaProductInfo((const char *)keyinfo.buf, (const char *)secretinfo.buf))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(qpy_Ql_iotConfigSetHttpOtaProductInfo_obj, qpy_Ql_iotConfigSetHttpOtaProductInfo);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigGetHttpOtaProductInfo(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_none;
	}
	char *product_key = NULL;
	char *product_secret = NULL;

	Ql_iotConfigGetHttpOtaProductInfo(&product_key, &product_secret);
	mp_obj_t product_info[2] =
		{
			mp_obj_new_str(product_key, strlen(product_key)),
			mp_obj_new_str(product_secret, strlen(product_secret)),
		};
	return mp_obj_new_tuple(2, product_info);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotConfigGetHttpOtaProductInfo_obj, qpy_Ql_iotConfigGetHttpOtaProductInfo);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetHttpOtaTls(mp_obj_t mp_tls)
{
	if (Ql_iotGetWorkState() == 0 || FALSE == mp_obj_is_bool(mp_tls))
	{
		return mp_obj_new_bool(FALSE);
	}
	if (FALSE == Ql_iotConfigSetHttpOtaTls(mp_const_true == mp_tls ? TRUE : FALSE))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotConfigSetHttpOtaTls_obj, qpy_Ql_iotConfigSetHttpOtaTls);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigGetHttpOtaTls(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_none;
	}
	return mp_obj_new_bool(Ql_iotConfigGetHttpOtaTls());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotConfigGetHttpOtaTls_obj, qpy_Ql_iotConfigGetHttpOtaTls);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetHttpOtaServer(mp_obj_t mp_server)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	mp_buffer_info_t serverinfo = {0};
	mp_get_buffer_raise(mp_server, &serverinfo, MP_BUFFER_READ);
	if (FALSE == Ql_iotConfigSetHttpOtaServer((const char *)serverinfo.buf))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotConfigSetHttpOtaServer_obj, qpy_Ql_iotConfigSetHttpOtaServer);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigGetHttpOtaServer(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_none;
	}
	char *server = NULL;

	Ql_iotConfigGetHttpOtaServer(&server);
	return mp_obj_new_str(server, strlen(server));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotConfigGetHttpOtaServer_obj, qpy_Ql_iotConfigGetHttpOtaServer);

/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetHttpOtaUp(mp_obj_t mp_battery, mp_obj_t mp_upmode, mp_obj_t mp_url)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	int battery = mp_obj_get_int(mp_battery);
	int upmode = mp_obj_get_int(mp_upmode);
	mp_buffer_info_t url = {0};
	mp_get_buffer_raise(mp_url, &url, MP_BUFFER_READ);
	if (FALSE == Ql_iotConfigSetHttpOtaUp(battery, upmode, (const char *)url.buf))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(qpy_Ql_iotConfigSetHttpOtaUp_obj, qpy_Ql_iotConfigSetHttpOtaUp);
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigGetHttpOtaUp(void)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_const_none;
	}
	quint8_t battery, upmode;
	char *url = NULL;

	Ql_iotConfigGetHttpOtaUp(&battery, &upmode, &url);
	mp_obj_t info[3] =
		{
			mp_obj_new_int(battery),
			mp_obj_new_int(upmode),
			mp_obj_new_str(url, strlen(url)),
		};
	return mp_obj_new_tuple(3, info);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_Ql_iotConfigGetHttpOtaUp_obj, qpy_Ql_iotConfigGetHttpOtaUp);

/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static mp_obj_t Ql_iotUrcHttpOtaEventCB = NULL;
static void ql_iotHttpOTAEventCB(quint32_t event, qint32_t errcode, const void *valueT, quint32_t valLen)
{
	if (NULL == Ql_iotUrcHttpOtaEventCB)
	{
		return;
	}
	if (NULL == valueT)
	{
		mp_obj_t tuple[] =
			{
				mp_obj_new_int_from_uint(event),
				mp_obj_new_int_from_uint(errcode)};
		mp_sched_schedule(Ql_iotUrcHttpOtaEventCB, mp_obj_new_tuple(2, tuple));
	}
	else
	{
		mp_obj_t tuple[] =
			{
				mp_obj_new_int_from_uint(event),
				mp_obj_new_int_from_uint(errcode),
				mp_obj_new_bytes(valueT, valLen)};
		mp_sched_schedule(Ql_iotUrcHttpOtaEventCB, mp_obj_new_tuple(3, tuple));
	}
}
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotConfigSetHttpOtaEventCb(mp_obj_t event_urc_cb)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	Ql_iotUrcHttpOtaEventCB = event_urc_cb;
	Ql_iotConfigSetHttpOtaEventCb(ql_iotHttpOTAEventCB);
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotConfigSetHttpOtaEventCb_obj, qpy_Ql_iotConfigSetHttpOtaEventCb);
#endif
#ifdef QUEC_ENABLE_GATEWAY
/**************************************************************************
** 	@brief : 使用回调函数将数据组包并把数据发送到Python应用层
** 	@param : 
** 	@retval: 
***************************************************************************/
static mp_obj_t Ql_iotSubDevUrcEventCB = NULL;
static void ql_iotSubDevEventCB(quint32_t event, qint32_t errcode, const char *subPk, const char *subDk, const void *valueT, quint32_t valLen)
{
	if (NULL == Ql_iotSubDevUrcEventCB)
	{
		return;
	}
	if (NULL == valueT)
	{
		mp_obj_t tuple[] =
			{
				mp_obj_new_bytes((const quint8_t*)subPk, strlen(subDk)),
				mp_obj_new_bytes((const quint8_t*)subDk, strlen(subDk)),
				mp_obj_new_int_from_uint(event),
				mp_obj_new_int_from_uint(errcode)};
		mp_sched_schedule(Ql_iotSubDevUrcEventCB, mp_obj_new_tuple(4, tuple));
	}
	else if (QIOT_ATEVENT_TYPE_RECV == event && QIOT_RECV_SUCC_PHYMODEL_RECV == errcode)
	{
		quint32_t count = Ql_iotTtlvCountGet(valueT);
		mp_obj_t node_dict = mp_obj_new_dict(count);
		if (ttlv_dict_handle(valueT, count, node_dict))
		{
			mp_obj_t tuple[] =
				{
					mp_obj_new_bytes((const quint8_t*)subPk, strlen(subPk)),
					mp_obj_new_bytes((const quint8_t*)subDk, strlen(subDk)),
					mp_obj_new_int_from_uint(event),
					mp_obj_new_int_from_uint(errcode),
					node_dict};
			mp_sched_schedule(Ql_iotSubDevUrcEventCB, mp_obj_new_tuple(5, tuple));
		}
	}
	else if (QIOT_ATEVENT_TYPE_RECV == event && QIOT_RECV_SUCC_PHYMODEL_REQ == errcode)
	{
		quint16_t pkgId = *(quint16_t *)valueT;
		quint16_t *ids = (quint16_t *)((quint8_t *)valueT + sizeof(quint16_t));
		mp_obj_t req_list = mp_obj_new_list(0, NULL);
		mp_obj_t req_list_temp = mp_obj_new_list(0, NULL);
		mp_obj_list_append(req_list, mp_obj_new_int((mp_int_t)pkgId));
		for (quint32_t i = 0; i < valLen; i++)
		{
			quint16_t modelId = ids[i];
			mp_obj_list_append(req_list_temp, mp_obj_new_int((mp_int_t)modelId));
		}
		mp_obj_list_append(req_list, req_list_temp);
		mp_obj_t tuple[] =
			{
				mp_obj_new_bytes((const quint8_t*)subPk, strlen(subPk)),
				mp_obj_new_bytes((const quint8_t*)subDk, strlen(subDk)),
				mp_obj_new_int_from_uint(event),
				mp_obj_new_int_from_uint(errcode),
				req_list};
		mp_sched_schedule(Ql_iotSubDevUrcEventCB, mp_obj_new_tuple(5, tuple));
	}
	else
	{
		mp_obj_t tuple[] =
			{
				mp_obj_new_bytes((const quint8_t*)subPk, strlen(subPk)),
				mp_obj_new_bytes((const quint8_t*)subDk, strlen(subDk)),
				mp_obj_new_int_from_uint(event),
				mp_obj_new_int_from_uint(errcode),
				mp_obj_new_bytes(valueT, valLen)};
		mp_sched_schedule(Ql_iotSubDevUrcEventCB, mp_obj_new_tuple(5, tuple));
	}
}
/**************************************************************************
** 	@brief : 注册子设备事件回调函数
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotSubDevSetEventCB(mp_obj_t event_sub_dev_urc_cb)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	Ql_iotSubDevUrcEventCB = event_sub_dev_urc_cb;
	Ql_iotConfigSetSubDevEventCB(ql_iotSubDevEventCB);
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_Ql_iotSubDevSetEventCB_obj, qpy_Ql_iotSubDevSetEventCB);
/**************************************************************************
** 	@brief : 发起子设备连接
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotSubDevConn(size_t n, const mp_obj_t *mp_data)
{
	if (Ql_iotGetWorkState() == 0
		|| !mp_obj_is_str(mp_data[0])|| !mp_obj_is_str(mp_data[1])|| !mp_obj_is_str(mp_data[2]))
	{
		printf("false...\n");
		return mp_obj_new_bool(FALSE);
	}
	mp_buffer_info_t pro_keyinfo = {0};
	mp_buffer_info_t pro_secretinfo = {0};
	mp_buffer_info_t dev_keyinfo = {0};
	mp_buffer_info_t dev_secretinfo = {0};
	int session_type_val;
	int keepalive_val;
	mp_get_buffer_raise(mp_data[0], &pro_keyinfo, MP_BUFFER_READ);
	mp_get_buffer_raise(mp_data[1], &pro_secretinfo, MP_BUFFER_READ);
	mp_get_buffer_raise(mp_data[2], &dev_keyinfo, MP_BUFFER_READ);
	if(5 == n)
	{
		session_type_val = mp_obj_get_int(mp_data[3]);
		keepalive_val = mp_obj_get_int(mp_data[4]);
	}
	else
	{
		mp_get_buffer_raise(mp_data[3], &dev_secretinfo, MP_BUFFER_READ);
		session_type_val = mp_obj_get_int(mp_data[4]);
		keepalive_val = mp_obj_get_int(mp_data[5]);
	}
	
	if (FALSE == Ql_iotSubDevConn((const char *)pro_keyinfo.buf, (const char *)pro_secretinfo.buf, (const char *)dev_keyinfo.buf, (const char *)dev_secretinfo.buf, session_type_val, keepalive_val))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(qpy_Ql_iotSubDevConn_obj, 5,6, qpy_Ql_iotSubDevConn);
/**************************************************************************
** 	@brief : 发起子设备登出
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotSubDevDisconn(mp_obj_t product_key, mp_obj_t device_key)
{
	if (Ql_iotGetWorkState() == 0)
	{
		return mp_obj_new_bool(FALSE);
	}
	mp_buffer_info_t pro_key = {0};
	mp_buffer_info_t dev_key = {0};
	mp_get_buffer_raise(product_key, &pro_key, MP_BUFFER_READ);
	mp_get_buffer_raise(device_key, &dev_key, MP_BUFFER_READ);
	if (FALSE == Ql_iotSubDevDisconn((const char *)pro_key.buf, (const char *)dev_key.buf))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(qpy_Ql_iotSubDevDisconn_obj, qpy_Ql_iotSubDevDisconn);
/**************************************************************************
** 	@brief : 发起子设备注销
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotSubDevDeauth(size_t n, const mp_obj_t *mp_data)
{
	if (Ql_iotGetWorkState() == 0 ||
		!mp_obj_is_str(mp_data[0])|| !mp_obj_is_str(mp_data[1])|| !mp_obj_is_str(mp_data[2])|| !mp_obj_is_str(mp_data[3]))
	{
		return mp_obj_new_bool(FALSE);
	}
	mp_buffer_info_t pro_key = {0};
	mp_buffer_info_t pro_secret = {0};
	mp_buffer_info_t dev_key = {0};
	mp_buffer_info_t dev_secret = {0};
	mp_get_buffer_raise(mp_data[0], &pro_key, MP_BUFFER_READ);
	mp_get_buffer_raise(mp_data[1], &pro_secret, MP_BUFFER_READ);
	mp_get_buffer_raise(mp_data[2], &dev_key, MP_BUFFER_READ);	
	mp_get_buffer_raise(mp_data[3], &dev_secret, MP_BUFFER_READ);
	if (FALSE == Ql_iotSubDevDeauth((const char *)pro_key.buf, (const char *)pro_secret.buf, (const char *)dev_key.buf, (const char *)dev_secret.buf))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(qpy_Ql_iotSubDevDeauth_obj, 4, 4, qpy_Ql_iotSubDevDeauth);
/**************************************************************************
** 	@brief : 上行子设备透传数据
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotSubDevPassTransSend(mp_obj_t product_key, mp_obj_t device_key, mp_obj_t mp_data)
{
	mp_buffer_info_t data = {0};
	mp_buffer_info_t pro_key = {0};
	mp_buffer_info_t dev_key = {0};
	mp_get_buffer_raise(product_key, &pro_key, MP_BUFFER_READ);
	mp_get_buffer_raise(device_key, &dev_key, MP_BUFFER_READ);
	mp_get_buffer_raise(mp_data, &data, MP_BUFFER_READ);
	if (FALSE == Ql_iotSubDevPassTransSend((const char *)pro_key.buf, (const char *)dev_key.buf, (quint8_t *)data.buf, data.len))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(qpy_Ql_iotSubDevPassTransSend_obj, qpy_Ql_iotSubDevPassTransSend);
/**************************************************************************
** 	@brief : 上行子设备物模型数据
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotSubDevTslReport(mp_obj_t product_key, mp_obj_t device_key, mp_obj_t mp_data)
{
	mp_obj_t ret = mp_const_true;
	void *ttlvHead = NULL;
	if (phy_dict_handle(mp_data, &ttlvHead))
	{
		mp_buffer_info_t pro_key = {0};
		mp_buffer_info_t dev_key = {0};
		mp_get_buffer_raise(product_key, &pro_key, MP_BUFFER_READ);
		mp_get_buffer_raise(device_key, &dev_key, MP_BUFFER_READ);
		if (FALSE == Ql_iotSubDevTslReport((const char *)pro_key.buf, (const char *)dev_key.buf, ttlvHead))
		{
			ret = mp_const_false;
		}
	}
	else
	{
		ret = mp_const_false;
	}
	Ql_iotTtlvFree(&ttlvHead);
	return ret;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(qpy_Ql_iotSubDevTslReport_obj, qpy_Ql_iotSubDevTslReport);
/**************************************************************************
** 	@brief : 子设备回复物模型查询
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotSubDevTslAck(size_t n, const mp_obj_t *mp_data)
{
	if (!mp_obj_is_str(mp_data[0])|| !mp_obj_is_str(mp_data[1])|| !mp_obj_is_int(mp_data[2]))
	{
		return mp_obj_new_bool(FALSE);
	}
	int pkgid = mp_obj_get_int(mp_data[2]);
	mp_obj_t ret = mp_const_true;
	void *ttlvHead = NULL;
	if (phy_dict_handle(mp_data[3], &ttlvHead))
	{
		mp_buffer_info_t pro_key = {0};
		mp_buffer_info_t dev_key = {0};
		mp_get_buffer_raise(mp_data[0], &pro_key, MP_BUFFER_READ);
		mp_get_buffer_raise(mp_data[1], &dev_key, MP_BUFFER_READ);
		if (FALSE == Ql_iotSubDevTslAck((const char *)pro_key.buf, (const char *)dev_key.buf, (quint16_t)pkgid, ttlvHead))
		{
			ret = mp_const_false;
		}
	}
	else
	{
		ret = mp_const_false;
	}
	Ql_iotTtlvFree(&ttlvHead);
	return ret;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(qpy_Ql_iotSubDevTslAck_obj, 4, 4, qpy_Ql_iotSubDevTslAck);
/**************************************************************************
** 	@brief : 子设备发送心跳包
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC mp_obj_t qpy_Ql_iotSubDevHTB(mp_obj_t product_key, mp_obj_t device_key)
{
	mp_buffer_info_t pro_key = {0};
	mp_buffer_info_t dev_key = {0};
	mp_get_buffer_raise(product_key, &pro_key, MP_BUFFER_READ);
	mp_get_buffer_raise(device_key, &dev_key, MP_BUFFER_READ);

	if (FALSE == Ql_iotSubDevHTB((const char *)pro_key.buf, (const char *)dev_key.buf))
	{
		return mp_obj_new_bool(FALSE);
	}
	return mp_obj_new_bool(TRUE);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(qpy_Ql_iotSubDevHTB_obj, qpy_Ql_iotSubDevHTB);
#endif
/**************************************************************************
** 	@brief : 
** 	@param : 
** 	@retval: 
***************************************************************************/
STATIC const mp_rom_map_elem_t mp_module_quecIot_globals_table[] = {
	{MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_quecIot)},
	{MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&qpy_Ql_iotInit_obj)},
	{MP_ROM_QSTR(MP_QSTR_setConnmode), MP_ROM_PTR(&qpy_Ql_iotConfigSetConnmode_obj)},
	{MP_ROM_QSTR(MP_QSTR_getConnmode), MP_ROM_PTR(&qpy_Ql_iotConfigGetConnmode_obj)},
	{MP_ROM_QSTR(MP_QSTR_setEventCB), MP_ROM_PTR(&qpy_Ql_iotConfigSetEventCB_obj)},
	{MP_ROM_QSTR(MP_QSTR_getWorkState), MP_ROM_PTR(&qpy_Ql_iotGetWorkState_obj)},
	{MP_ROM_QSTR(MP_QSTR_setProductinfo), MP_ROM_PTR(&qpy_Ql_iotConfigSetProductinfo_obj)},
	{MP_ROM_QSTR(MP_QSTR_getProductinfo), MP_ROM_PTR(&qpy_Ql_iotConfigGetProductinfo_obj)},
	{MP_ROM_QSTR(MP_QSTR_setServer), MP_ROM_PTR(&qpy_Ql_iotConfigSetServer_obj)},
	{MP_ROM_QSTR(MP_QSTR_getServer), MP_ROM_PTR(&qpy_Ql_iotConfigGetServer_obj)},
	{MP_ROM_QSTR(MP_QSTR_setLifetime), MP_ROM_PTR(&qpy_Ql_iotConfigSetLifetime_obj)},
	{MP_ROM_QSTR(MP_QSTR_getLifetime), MP_ROM_PTR(&qpy_Ql_iotConfigGetLifetime_obj)},
	{MP_ROM_QSTR(MP_QSTR_setSessionFlag), MP_ROM_PTR(&qpy_Ql_iotConfigSetSessionFlag_obj)},
	{MP_ROM_QSTR(MP_QSTR_getSessionFlag), MP_ROM_PTR(&qpy_Ql_iotConfigGetSessionFlag_obj)},
	{MP_ROM_QSTR(MP_QSTR_setPdpContextId), MP_ROM_PTR(&qpy_Ql_iotConfigSetPdpContextId_obj)},
	{MP_ROM_QSTR(MP_QSTR_getPdpContextId), MP_ROM_PTR(&qpy_Ql_iotConfigGetPdpContextId_obj)},
	{MP_ROM_QSTR(MP_QSTR_getSoftVersion), MP_ROM_PTR(&qpy_Ql_iotConfigGetSoftVersion_obj)},
	{MP_ROM_QSTR(MP_QSTR_setMcuVersion), MP_ROM_PTR(&qpy_Ql_iotConfigSetMcuVersion_obj)},
	{MP_ROM_QSTR(MP_QSTR_getMcuVersion), MP_ROM_PTR(&qpy_Ql_iotConfigGetMcuVersion_obj)},
	{MP_ROM_QSTR(MP_QSTR_setDkDs), MP_ROM_PTR(&qpy_Ql_iotConfigSetDkDs_obj)},
	{MP_ROM_QSTR(MP_QSTR_getDkDs), MP_ROM_PTR(&qpy_Ql_iotConfigGetDkDs_obj)},

	{MP_ROM_QSTR(MP_QSTR_passTransSend), MP_ROM_PTR(&qpy_Ql_iotCmdBusPassTransSend_obj)},
	{MP_ROM_QSTR(MP_QSTR_phymodelReport), MP_ROM_PTR(&qpy_Ql_iotCmdBusPhymodelReport_obj)},
	{MP_ROM_QSTR(MP_QSTR_phymodelAck), MP_ROM_PTR(&qpy_Ql_iotCmdBusPhymodelAck_obj)},
	{MP_ROM_QSTR(MP_QSTR_getDevStatus), MP_ROM_PTR(&qpy_Ql_iotCmdSysGetDevStatus_obj)},
	{MP_ROM_QSTR(MP_QSTR_getDevInfo), MP_ROM_PTR(&qpy_Ql_iotCmdSysGetDevInfo_obj)},
	{MP_ROM_QSTR(MP_QSTR_statusReport), MP_ROM_PTR(&qpy_Ql_iotCmdSysStatusReport_obj)},
	{MP_ROM_QSTR(MP_QSTR_devInfoReport), MP_ROM_PTR(&qpy_Ql_iotCmdSysDevInfoReport_obj)},

	{MP_ROM_QSTR(MP_QSTR_getLocSupList), MP_ROM_PTR(&qpy_Ql_iotCmdGetLocSupList_obj)},
	{MP_ROM_QSTR(MP_QSTR_getLocData), MP_ROM_PTR(&qpy_Ql_iotCmdGetLocData_obj)},
	{MP_ROM_QSTR(MP_QSTR_locReportInside), MP_ROM_PTR(&qpy_Ql_iotCmdBusLocReportInside_obj)},
	{MP_ROM_QSTR(MP_QSTR_locReportOutside), MP_ROM_PTR(&qpy_Ql_iotCmdBusLocReportOutside_obj)},

	{MP_ROM_QSTR(MP_QSTR_otaRequest), MP_ROM_PTR(&qpy_Ql_iotCmdOtaRequest_obj)},
	{MP_ROM_QSTR(MP_QSTR_otaAction), MP_ROM_PTR(&qpy_Ql_iotCmdOtaAction_obj)},
	{MP_ROM_QSTR(MP_QSTR_mcuFWDataRead), MP_ROM_PTR(&qpy_Ql_iotCmdOtaMcuFWDataRead_obj)},
#ifdef QUEC_ENABLE_HTTP_OTA
	{MP_ROM_QSTR(MP_QSTR_setHttpOtaProductInfo), MP_ROM_PTR(&qpy_Ql_iotConfigSetHttpOtaProductInfo_obj)},
	{MP_ROM_QSTR(MP_QSTR_getHttpOtaProductInfo), MP_ROM_PTR(&qpy_Ql_iotConfigGetHttpOtaProductInfo_obj)},
	{MP_ROM_QSTR(MP_QSTR_setHttpOtaTls), MP_ROM_PTR(&qpy_Ql_iotConfigSetHttpOtaTls_obj)},
	{MP_ROM_QSTR(MP_QSTR_getHttpOtaTls), MP_ROM_PTR(&qpy_Ql_iotConfigGetHttpOtaTls_obj)},
	{MP_ROM_QSTR(MP_QSTR_setHttpOtaServer), MP_ROM_PTR(&qpy_Ql_iotConfigSetHttpOtaServer_obj)},
	{MP_ROM_QSTR(MP_QSTR_getHttpOtaServer), MP_ROM_PTR(&qpy_Ql_iotConfigGetHttpOtaServer_obj)},
	{MP_ROM_QSTR(MP_QSTR_setHttpOtaUp), MP_ROM_PTR(&qpy_Ql_iotConfigSetHttpOtaUp_obj)},
	{MP_ROM_QSTR(MP_QSTR_getHttpOtaUp), MP_ROM_PTR(&qpy_Ql_iotConfigGetHttpOtaUp_obj)},
	{MP_ROM_QSTR(MP_QSTR_setHttpOtaEventCb), MP_ROM_PTR(&qpy_Ql_iotConfigSetHttpOtaEventCb_obj)},
#endif
#ifdef QUEC_ENABLE_GATEWAY
	{MP_ROM_QSTR(MP_QSTR_subDevSetEventCB), MP_ROM_PTR(&qpy_Ql_iotSubDevSetEventCB_obj)},
	{MP_ROM_QSTR(MP_QSTR_subDevConn), MP_ROM_PTR(&qpy_Ql_iotSubDevConn_obj)},
	{MP_ROM_QSTR(MP_QSTR_subDevDisconn), MP_ROM_PTR(&qpy_Ql_iotSubDevDisconn_obj)},
	{MP_ROM_QSTR(MP_QSTR_subDevDeauth), MP_ROM_PTR(&qpy_Ql_iotSubDevDeauth_obj)},
	{MP_ROM_QSTR(MP_QSTR_subDevPassTransSend), MP_ROM_PTR(&qpy_Ql_iotSubDevPassTransSend_obj)},
	{MP_ROM_QSTR(MP_QSTR_subDevTslReport), MP_ROM_PTR(&qpy_Ql_iotSubDevTslReport_obj)},
	{MP_ROM_QSTR(MP_QSTR_subDevTslAck), MP_ROM_PTR(&qpy_Ql_iotSubDevTslAck_obj)},
	{MP_ROM_QSTR(MP_QSTR_subDevHTB), MP_ROM_PTR(&qpy_Ql_iotSubDevHTB_obj)},
#endif
};
STATIC MP_DEFINE_CONST_DICT(mp_module_quecIot_globals, mp_module_quecIot_globals_table);

const mp_obj_module_t mp_module_quecIot = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&mp_module_quecIot_globals,
};
