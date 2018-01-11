/*******************************************************************************************
 *  @file      fsal_kodo_ds.c 2018\1\11 16:28:29 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com> (中文编码测试)
 *******************************************************************************************/
#include "fsal_kodo_internal.h"
#include "../fsal_private.h"
/**
 *  @param ops FSAL pNFS ds ops
 */
void pnfs_ds_ops_init(struct fsal_pnfs_ds_ops *ops)
{
    memcpy(ops, &def_pnfs_ds_ops, sizeof(struct fsal_pnfs_ds_ops));
}


// 
// ------------------------------------------------------------------------------------------
