/*******************************************************************************************
 *  @file      fsal_kodo_internal.c 2017\12\20 16:14:09 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com> (中文编码测试)
 *******************************************************************************************/
#include "fsal_kodo_internal.h"
#include <error.h>

struct kodo_fsal_module g_kodoFileSystemModule;


// ------------------------------------------------------------------------------------------
// 
int construct_kodo_handle(struct kodo_export* pKodoExport, struct kodo_handle** ppObj)
{
    struct kodo_handle* pKodoHandle = NULL;
    *ppObj = NULL;

    pKodoHandle = gsh_calloc(1, sizeof(struct kodo_handle));

    if (pKodoHandle == NULL)
    {
        return -ENOMEM;
    }

    //pKodoHandle->的成员初始化

    *ppObj = pKodoHandle;

    return 0;
}

// ------------------------------------------------------------------------------------------
// 
void deconstruct_kodo_handle(struct kodo_handle* pKodoHandle)
{
    //pKodoHandle->的成员初始化
    gsh_free(pKodoHandle);

}

// 
// ------------------------------------------------------------------------------------------

