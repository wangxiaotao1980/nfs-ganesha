/*******************************************************************************************
 *  @file      fsal_kodo_internal.h 2017\12\20 15:23:19 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com> (中文编码测试)
 *  @note      summary
 *******************************************************************************************/
#ifndef  FSAL_KODO_INTERNAL_CD7F7735_C8E9_43E2_9635_0ADBD791521F_H__
#define  FSAL_KODO_INTERNAL_CD7F7735_C8E9_43E2_9635_0ADBD791521F_H__

/*******************************************************************************************/

#include "../../include/fsal.h"
#include "../../include/fsal_types.h"
#include "../../include/fsal_api.h"
#include "../../include/fsal_convert.h"
#include "../../include/sal_data.h"

/**
 * KODO Main (global) module object
 */

struct kodo_fsal_module
{
    struct fsal_module  fsal;
    fsal_staticfsinfo_t fs_info;
};

extern struct kodo_fsal_module g_kodoFileSystemModule;


/**
 * KODO Private export object
 */
struct kodo_export
{
    struct fsal_export pub;  /*< The public export object  注意第一个成员必须是 fsal_export */
    char *kodo_access_key;
    char *kodo_secret_access_key;

};

struct kodo_fs_fd
{
    
};


struct kodo_state_fd
{
    struct state_t    state;
    struct kodo_fs_fd kodo_fs_fd;
};

/**
 * The 'private' KODO FASL handle
 */
struct kodo_handle
{
    struct fsal_obj_handle pub;  /*注意第一个成员必须是 fsal_obj_handle*/
};



/**
 * The attributes this FSAL can interpret or supply.
 * Currently FSAL_KODO uses posix2fsal_attributes, so we should indicate support
 * for at least those attributes.
 */
#define KODO_SUPPORTED_ATTRIBUTES ((const attrmask_t) (ATTRS_POSIX))

void kodo_export_ops_init(struct export_ops* ops);
void kodo_handle_ops_init(struct fsal_obj_ops* ops);



/* Prototypes */
int construct_kodo_handle
    (
        struct kodo_export*   pKodoExport,
        struct kodo_handle**  ppObj
    );

void deconstruct_kodo_handle
    (
        struct kodo_handle* pKodoHandle
    );


//for pnfs init function
//void kodo_pnfs_ds_ops_init(struct fsal_pnfs_ds_ops* ops);
//void kodo_export_ops_pnfs(struct export_ops* ops);
//void kodo_handle_ops_pnfs(struct fsal_obj_ops* ops);
/*******************************************************************************************/
#endif// FSAL_KODO_INTERNAL_CD7F7735_C8E9_43E2_9635_0ADBD791521F_H__