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
    .maxfilesize              = UINT64_MAX,
    .maxlink                  = _POSIX_LINK_MAX,
    .maxnamelen               = 1024,
    .maxpathlen               = 1024,
    .no_trunc                 = true,
    .chown_restricted         = false,
    .case_insensitive         = false,
    .case_preserving          = true,
    .link_support             = false,
    .symlink_support          = false,
    .lock_support             = false,
    .lock_support_owner       = false,
    .lock_support_async_block = false,
    .named_attr               = true, /* XXX */
    .unique_handles           = true,
    .lease_time               = {10, 0},
    .acl_support              = false,
    .cansettime               = true,
    .homogenous               = true,
    .supported_attrs          = KODO_SUPPORTED_ATTRIBUTES,
    .maxread                  = FSAL_MAXIOSIZE,
    .maxwrite                 = FSAL_MAXIOSIZE,
    .umask                    = 0,
    .rename_changes_key       = true,
#if HAVE_DIRENT_OFFSETOF
    .compute_readdir_cookie   = true,
#endif
    .whence_is_name           = true,
};


static struct config_item kodo_items[] = 
{
    CONFIG_EOL,
};


struct config_block kodo_block = 
{
    .dbus_interface_name   = "org.ganesha.nfsd.config.fsal.kodo",
    .blk_desc.name         = "KODO",
    .blk_desc.type         = CONFIG_BLOCK,
    .blk_desc.u.blk.init   = noop_conf_init,
    .blk_desc.u.blk.params = kodo_items,
    .blk_desc.u.blk.commit = noop_conf_commit,
};



//static int kodo_fsal_ops_unload(struct fsal_module* fsal_hdl)
//{}


/**
 * @brief Initialize the configuration
 *
 * Given the root of the Ganesha configuration structure, initialize
 * the FSAL parameters.
 *
 * @param[in] fsal_hdl      The FSAL module
 * @param[in] config_struct Parsed ganesha configuration file
 * @param[out]err_type      config error processing state
 *
 * @return FSAL status.
 */
static fsal_status_t kodo_fsal_ops_init_config(struct fsal_module* fsal_hdl,
                                               config_file_t config_struct,
                                               struct config_error_type* err_type)
{

    //转换成
    struct kodo_fsal_module* kodoFsalModule 
        = container_of(fsal_hdl, struct kodo_fsal_module, fsal);


    kodoFsalModule->fs_info = default_kodo_info;


    if (kodoFsalModule->fs_info.lock_support)
    {
        LogInfo(COMPONENT_FSAL, "FSAL_KODO enabling OFD Locks");
    }
    else
    {
        LogInfo(COMPONENT_FSAL, "FSAL_KODO disabling lock support");
    }

    (void)load_config_from_parse(config_struct, 
                                 &kodo_block,
                                 &(kodoFsalModule->fs_info),
                                 true,
                                 err_type);

    if (!config_error_is_harmless(err_type))
    {
        return fsalstat(ERR_FSAL_INVAL, 0);
    }

    return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

//static void kodo_fsal_ops_dump_config(struct fsal_module* fsal_hdl, int log_fd)
//{}

/**
 * @brief Create a new export
 *
 * This function creates a new export in the FSAL using the supplied
 * path and options.  The function is expected to allocate its own
 * export (the full, private structure).  It must then initialize the
 * public portion like so:
 *
 * @code{.c}
 *         fsal_export_init(&private_export_handle->pub);
 * @endcode
 *
 * After doing other private initialization, it must attach the export
 * to the module, like so:
 *
 *
 * @code{.c}
 *         fsal_attach_export(fsal_hdl,
 *                            &private_export->pub.exports);
 *
 * @endcode
 *
 * And create the parent link with:
 *
 * @code{.c}
 * private_export->pub.fsal = fsal_hdl;
 * @endcode
 *
 * @note This seems like something that fsal_attach_export should
 * do. -- ACE.
 *
 * @param[in]     fsal_hdl    FSAL module
 * @param[in]     parse_node  opaque pointer to parse tree node for
 *                            export options to be passed to
 *                            load_config_from_node
 * @param[out]    err_type    config proocessing error reporting
 * @param[in]     up_ops      Upcall ops
 *
 * @return FSAL status.
 */
static fsal_status_t kodo_fsal_ops_create_export(struct fsal_module* fsal_hdl,
                                                 void* parse_node,
                                                 struct config_error_type* err_type,
                                                 const struct fsal_up_vector* up_ops)
{
    fsal_status_t status = { ERR_FSAL_NO_ERROR, 0 };

    struct kodo_export* pKODOExport = NULL;

    pKODOExport = gsh_calloc(1, sizeof(struct kodo_export));
    if (pKODOExport == NULL)
    {
        status.major = ERR_FSAL_NOMEM;
        LogCrit(COMPONENT_FSAL, "Unable to allocate export object for %s.", op_ctx->ctx_export->fullpath);
        goto error;
    }






error:
    gsh_free(pKODOExport);
    return status;

}

//static void kodo_fsal_ops_emergency_cleanup(void)
//{}

//static nfsstat4 kodo_fsal_ops_getdeviceinfo(struct fsal_module* fsal_hdl,
//                                            XDR* da_addr_body,
//                                            const layouttype4 type,
//                                            const struct pnfs_deviceid* deviceid)
//{}
//
//static size_t kodo_fsal_ops_fs_da_addr_size(struct fsal_module* fsal_hdl)
//{}
//
//static fsal_status_t kodo_fsal_ops_fsal_pnfs_ds(struct fsal_module*const fsal_hdl,
//                                                void* parse_node,
//                                                struct fsal_pnfs_ds**const handle)
//{}
//static void kodo_fsal_ops_fsal_pnfs_ds_ops(struct fsal_pnfs_ds_ops* ops)
//{}

static bool kodo_fsal_ops_support_ex(struct fsal_obj_handle* obj)
{
    return true;
}

//static void kodo_fsal_ops_fsal_extract_stats(struct fsal_module*const fsal_hdl,
//                                             void* iter)
//{}
//
//static void kodo_fsal_ops_fsal_reset_stats(struct fsal_module*const fsal_hdl)
//{}



MODULE_INIT void kodo_load(void)
{
    int retval = 0;
    int registerFsalResult = 0;

    struct fsal_module* pFsalModule = &(g_kodoFileSystemModule.fsal);
    LogDebug(COMPONENT_FSAL, "KODO module registering.");

    /* register_fsal seems to expect zeroed memory. */
    memset(pFsalModule, 0, sizeof(*pFsalModule));


    registerFsalResult = register_fsal(pFsalModule, module_name, FSAL_MAJOR_VERSION, FSAL_MINOR_VERSION, FSAL_ID_NO_PNFS);

    if (registerFsalResult != 0)
    {
        LogCrit(COMPONENT_FSAL, "KODO FSAL module failed to register.");
        return;
    }

    pFsalModule->m_ops.create_export = kodo_fsal_ops_create_export;
    pFsalModule->m_ops.init_config   = kodo_fsal_ops_init_config;
    pFsalModule->m_ops.support_ex    = kodo_fsal_ops_support_ex;

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
