#include "qhal_patch.h"
#include "Qhal_types.h"
#include "Ql_iotApi.h"

#include "helios_os.h"


void qhal_mutex_create(Helios_Mutex_t *mutex)
{
    *mutex = Helios_Mutex_Create();  
}






