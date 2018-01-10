/** @file main.c
 *  @brief GPFS FSAL module core functions
 *
 * vim:noexpandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright (C) Panasas Inc., 2011
 * Author: Jim Lieb jlieb@panasas.com
 *
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * -------------
 */

#include <sys/types.h>
#include "../../include/fsal.h"
#include "../../include/FSAL/fsal_init.h"
#include "../../include/gsh_config.h"
#include "fsal_internal.h"
#include "gpfs_methods.h"

static const char myname[] = "GPFS";

/** @struct gpfs_fsal_module
 *  @brief GPFS FSAL module private storage
 */
struct gpfs_fsal_module
{
    struct fsal_module fsal;
    struct fsal_staticfsinfo_t fs_info;
    /** gpfsfs_specific_initinfo_t specific_info;  placeholder */
};

/** @struct default_gpfs_info
 *  @brief filesystem info for GPFS
 */
static struct fsal_staticfsinfo_t default_gpfs_info = {
    .maxfilesize = UINT64_MAX,
    .maxlink = _POSIX_LINK_MAX,
    .maxnamelen = 1024,
    .maxpathlen = 1024,
    .no_trunc = true,
    .chown_restricted = false,
    .case_insensitive = false,
    .case_preserving = true,
    .link_support = true,
    .symlink_support = true,
    .lock_support = true,
    .lock_support_owner = true,
    .lock_support_async_block = true,
    .named_attr = true,
    .unique_handles = true,
    .lease_time = { 10, 0 },
    .acl_support = FSAL_ACLSUPPORT_ALLOW | FSAL_ACLSUPPORT_DENY,
    .cansettime = true,
    .homogenous = true,
    .supported_attrs = GPFS_SUPPORTED_ATTRIBUTES,
    .maxread = FSAL_MAXIOSIZE,
    .maxwrite = FSAL_MAXIOSIZE,
    .umask = 0,
    .auth_exportpath_xdev = true,
    .xattr_access_rights = 0,
    .share_support = true,
    .share_support_owner = false,
    .delegations = FSAL_OPTION_FILE_READ_DELEG, /** not working with pNFS */
    .pnfs_mds = true,
    .pnfs_ds = true,
    .fsal_trace = true,
    .fsal_grace = false,
    .link_supports_permission_checks = true,
};

/** @struct gpfs_params
 *  @brief Configuration items
 */
static struct config_item gpfs_params[] = {
    CONF_ITEM_BOOL("link_support", true,
        fsal_staticfsinfo_t, link_support),
    CONF_ITEM_BOOL("symlink_support", true,
        fsal_staticfsinfo_t, symlink_support),
    CONF_ITEM_BOOL("cansettime", true,
        fsal_staticfsinfo_t, cansettime),
    CONF_ITEM_MODE("umask", 0,
        fsal_staticfsinfo_t, umask),
    CONF_ITEM_BOOL("auth_xdev_export", false,
        fsal_staticfsinfo_t, auth_exportpath_xdev),
    CONF_ITEM_MODE("xattr_access_rights", 0400,
        fsal_staticfsinfo_t, xattr_access_rights),
    /** At the moment GPFS doesn't support WRITE delegations */
    CONF_ITEM_ENUM_BITS("Delegations",
        FSAL_OPTION_FILE_READ_DELEG,
        FSAL_OPTION_FILE_DELEGATIONS,
        deleg_types, fsal_staticfsinfo_t, delegations),
    CONF_ITEM_BOOL("PNFS_MDS", true,
        fsal_staticfsinfo_t, pnfs_mds),
    CONF_ITEM_BOOL("PNFS_DS", true,
        fsal_staticfsinfo_t, pnfs_ds),
    CONF_ITEM_BOOL("fsal_trace", true,
        fsal_staticfsinfo_t, fsal_trace),
    CONF_ITEM_BOOL("fsal_grace", false,
        fsal_staticfsinfo_t, fsal_grace),
    CONFIG_EOL
};

/** @struct gpfs_param
 *  @brief Configuration block
 */
static struct config_block gpfs_param = {
    .dbus_interface_name = "org.ganesha.nfsd.config.fsal.gpfs",
    .blk_desc.name = "GPFS",
    .blk_desc.type = CONFIG_BLOCK,
    .blk_desc.u.blk.init = noop_conf_init,
    .blk_desc.u.blk.params = gpfs_params,
    .blk_desc.u.blk.commit = noop_conf_commit
};

/** @struct GPFS
 *  @brief my module private storage
 */
static struct gpfs_fsal_module GPFS;


/** @fn struct fsal_staticfsinfo_t *gpfs_staticinfo(struct fsal_module *hdl)
 *  @brief private helper for export object
 *  @param hdl handle to fsal_module
 */
struct fsal_staticfsinfo_t* gpfs_staticinfo(struct fsal_module* hdl)
{
    struct gpfs_fsal_module* gpfs_me =
    container_of(hdl, struct gpfs_fsal_module, fsal)
    ;

    return &gpfs_me->fs_info;
}

/** @fn static int
 *      log_to_gpfs(log_header_t headers, void *private, log_levels_t level,
 *	struct display_buffer *buffer, char *compstr, char *message)
 *  @brief Log to gpfs
 */
static int
log_to_gpfs(log_header_t headers, void* private, log_levels_t level,
            struct display_buffer* buffer, char* compstr, char* message)
{
    struct trace_arg targ = { 0 };

    if(level <= 0)
        return 0;

    targ.level = level;
    targ.len = strlen(compstr);
    targ.str = compstr;

    return gpfs_ganesha(OPENHANDLE_TRACE_ME, &targ);
}

/** @fn static fsal_status_t init_config(struct fsal_module *fsal_hdl,
 *	config_file_t config_struct, struct config_error_type *err_type)
 *  @brief must be called with a reference taken (via lookup_fsal)
 */
static fsal_status_t init_config(struct fsal_module* fsal_hdl,
                                 config_file_t config_struct,
                                 struct config_error_type* err_type)
{
    struct gpfs_fsal_module* gpfs_me =
    container_of(fsal_hdl, struct gpfs_fsal_module, fsal)
    ;
    int rc;

    (void)prepare_for_stats(fsal_hdl);
    gpfs_me->fs_info = default_gpfs_info; /** get a copy of the defaults */

    (void)load_config_from_parse(config_struct,
                                 &gpfs_param,
                                 &gpfs_me->fs_info,
                                 true,
                                 err_type);

    if(!config_error_is_harmless(err_type))
        return fsalstat(ERR_FSAL_INVAL, 0);

    display_fsinfo(&gpfs_me->fs_info);

    LogFullDebug(COMPONENT_FSAL,
        "Supported attributes constant = 0x%" PRIx64,
        GPFS_SUPPORTED_ATTRIBUTES);
    LogFullDebug(COMPONENT_FSAL,
        "Supported attributes default = 0x%" PRIx64,
        default_gpfs_info.supported_attrs);
    LogDebug(COMPONENT_FSAL,
        "FSAL INIT: Supported attributes mask = 0x%" PRIx64,
        gpfs_me->fs_info.supported_attrs);

    rc = create_log_facility(myname,
                             log_to_gpfs,
                             NIV_FULL_DEBUG,
                             LH_COMPONENT,
                             NULL);
    if(rc != 0)
    {
        LogCrit(COMPONENT_FSAL,
            "Could not create GPFS logger (%s)", strerror(-rc));
        return fsalstat(ERR_FSAL_INVAL, 0);
    }

    if(gpfs_me->fs_info.fsal_trace)
    {
        rc = enable_log_facility(myname);
        if(rc == 0)
            return fsalstat(ERR_FSAL_NO_ERROR, 0);

        LogCrit(COMPONENT_FSAL, "Could not enable GPFS logger (%s)",
            strerror(-rc));
        return fsalstat(ERR_FSAL_INVAL, 0);
    }

    rc = disable_log_facility(myname);
    if(rc == 0)
        return fsalstat(ERR_FSAL_NO_ERROR, 0);

    LogCrit(COMPONENT_FSAL, "Could not disable GPFS logger (%s)",
        strerror(-rc));

    return fsalstat(ERR_FSAL_INVAL, 0);
}

/**
 * @brief Indicate support for extended operations.
 *
 * @retval true if extended operations are supported.
 */

bool gpfs_support_ex(struct fsal_obj_handle* obj)
{
    return true;
}

/** @fn MODULE_INIT void gpfs_init(void)
 *  @brief  Module initialization.
 *
 *  Called by dlopen() to register the module
 *  keep a private pointer to me in myself
 */
MODULE_INIT void gpfs_init(void)
{
    struct fsal_module* myself = &GPFS.fsal;

    if(register_fsal(myself,
                     myname,
                     FSAL_MAJOR_VERSION,
                     FSAL_MINOR_VERSION,
                     FSAL_ID_GPFS) != 0)
    {
        fprintf(stderr, "GPFS module failed to register");
        return;
    }

    /** Set up module operations */
    myself->m_ops.fsal_pnfs_ds_ops = pnfs_ds_ops_init;
    myself->m_ops.create_export = gpfs_create_export;
    myself->m_ops.init_config = init_config;
    myself->m_ops.support_ex = gpfs_support_ex;
    myself->m_ops.getdeviceinfo = getdeviceinfo;
    myself->m_ops.fs_da_addr_size = fs_da_addr_size;
#ifdef USE_DBUS
    myself->m_ops.fsal_extract_stats = fsal_gpfs_extract_stats;
#endif
    myself->m_ops.fsal_reset_stats = fsal_gpfs_reset_stats;
}

/** @fn MODULE_FINI void gpfs_unload(void)
 *  @brief unload module
 */
MODULE_FINI void gpfs_unload(void)
{
    release_log_facility(myname);

    if(unregister_fsal(&GPFS.fsal) != 0)
        fprintf(stderr, "GPFS module failed to unregister");
}
