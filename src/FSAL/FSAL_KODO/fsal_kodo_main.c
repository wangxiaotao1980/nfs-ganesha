/*******************************************************************************************
 *  @file      main.c 2017\12\20 12:37:36 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com> (中文编码测试)
 *******************************************************************************************/

#include "fsal_kodo_internal.h"

#include "../../include/FSAL/fsal_init.h"
#include "../../include/fsal_types.h"


static const char module_name[] = "KODO";


/* filesystem info for KODO */
static struct fsal_staticfsinfo_t default_kodo_info =
{
};




MODULE_INIT void kodo_load(void)
{
}



MODULE_FINI void kodo_unload(void)
{
}
// 
// ------------------------------------------------------------------------------------------
