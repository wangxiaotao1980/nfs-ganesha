/*******************************************************************************************
 *  @file      main.c 2017\12\20 12:37:36 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com> (中文编码测试)
 *******************************************************************************************/

#include "fsal_kodo_internal.h"

#include "../../include/FSAL/fsal_init.h"
#include "../../include/fsal_types.h"

#include "fsal_kodo_internal.h"

 /**
  * KODO global module object.
  */
struct kodo_fsal_module g_kodoFileSystemModule;

/**
 * The name of this module.
 */
static const char module_name[] = "KODO";


/* filesystem info for KODO */
static struct fsal_staticfsinfo_t default_kodo_info =
{
};

static int kodo_fsal_ops_unload(struct fsal_module* fsal_hdl)
{}

static fsal_status_t kodo_fsal_ops_init_config(struct fsal_module* fsal_hdl,
                                               config_file_t config_struct,
                                               struct config_error_type* err_type)
{}

static void kodo_fsal_ops_dump_config(struct fsal_module* fsal_hdl, int log_fd)
{}

static fsal_status_t kodo_fsal_ops_create_export(struct fsal_module* fsal_hdl,
                                                 void* parse_node,
                                                 struct config_error_type* err_type,
                                                 const struct fsal_up_vector* up_ops)
{}

static void kodo_fsal_ops_emergency_cleanup(void)
{}

static nfsstat4 kodo_fsal_ops_getdeviceinfo(struct fsal_module* fsal_hdl,
                                            XDR* da_addr_body,
                                            const layouttype4 type,
                                            const struct pnfs_deviceid* deviceid)
{}

static size_t kodo_fsal_ops_fs_da_addr_size(struct fsal_module* fsal_hdl)
{}

static fsal_status_t kodo_fsal_ops_fsal_pnfs_ds(struct fsal_module*const fsal_hdl,
                                                void* parse_node,
                                                struct fsal_pnfs_ds**const handle)
{}
static void kodo_fsal_ops_fsal_pnfs_ds_ops(struct fsal_pnfs_ds_ops* ops)
{}

static bool kodo_fsal_ops_support_ex(struct fsal_obj_handle* obj)
{}

static void kodo_fsal_ops_fsal_extract_stats(struct fsal_module*const fsal_hdl,
                                             void* iter)
{}

static void kodo_fsal_ops_fsal_reset_stats(struct fsal_module*const fsal_hdl)
{}



MODULE_INIT void kodo_load(void)
{
    struct fsal_module* pFsalModule = &(g_kodoFileSystemModule.fsal);
    LogDebug(COMPONENT_FSAL, "KODO module registering.");

    /* register_fsal seems to expect zeroed memory. */
    memset(pFsalModule, 0, sizeof(*pFsalModule));

}


MODULE_FINI void kodo_unload(void)
{
    LogDebug(COMPONENT_FSAL, "KODO module finishing.");

    if (unregister_fsal(&(g_kodoFileSystemModule.fsal)) != 0)
    {
        LogCrit(COMPONENT_FSAL, "Unable to unload KODO FSAL.  Dying with extreme prejudice.");
        abort();
    }
}
// 
// ------------------------------------------------------------------------------------------
