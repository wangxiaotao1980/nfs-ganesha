/*******************************************************************************************
 *  @file      fsal_kodo_ds.c 2018\1\11 16:28:29 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com> (中文编码测试)
 *******************************************************************************************/
#include "fsal_kodo_internal.h"
#include "../fsal_private.h"


static nfsstat4 kodo_make_ds_handle(struct fsal_pnfs_ds*const pds,
                                    const struct gsh_buffdesc*const hdl_desc,
                                    struct fsal_ds_handle**const handle,
                                    int flags)
{
    return NFS4_OK;
}



static void kodo_dsh_ops_init(struct fsal_dsh_ops *ops)
{
    /* redundant copy, but you never know about the future... */
    memcpy(ops, &def_dsh_ops, sizeof(struct fsal_dsh_ops));

/*  ops->release  = ds_release;
    ops->read       = ds_read;
    ops->read_plus  = ds_read_plus;
    ops->write      = ds_write;
    ops->write_plus = ds_write_plus;
    ops->commit     = ds_commit;*/
}





/**
 *  @param ops FSAL pNFS ds ops
 */
void kodo_pnfs_ds_ops_init(struct fsal_pnfs_ds_ops *ops)
{
    memcpy(ops, &def_pnfs_ds_ops, sizeof(struct fsal_pnfs_ds_ops));
    
    ops->make_ds_handle = kodo_make_ds_handle;
    ops->fsal_dsh_ops   = kodo_dsh_ops_init;

}


// 
// ------------------------------------------------------------------------------------------
