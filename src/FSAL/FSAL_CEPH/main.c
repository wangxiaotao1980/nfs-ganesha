/*
 * Copyright © 2012-2014, CohortFS, LLC.
 * Author: Adam C. Emerson <aemerson@linuxbox.com>
 *
 * contributeur : William Allen Simpson <bill@cohortfs.com>
 *          Marcus Watts <mdw@cohortfs.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * -------------
 */

/**
 * @file FSAL_CEPH/main.c
 * @author Adam C. Emerson <aemerson@linuxbox.com>
 * @author William Allen Simpson <bill@cohortfs.com>
 * @date Wed Oct 22 13:24:33 2014
 *
 * @brief Implementation of FSAL module founctions for Ceph
 *
 * This file implements the module functions for the Ceph FSAL, for
 * initialization, teardown, configuration, and creation of exports.
 */


#include "../../include/fsal.h"
#include "../../include/fsal_types.h"
#include "../../include/FSAL/fsal_init.h"
#include "../../include/FSAL/fsal_commonlib.h"
#include "../../include/fsal_api.h"
#include "../../include/abstract_mem.h"
#include "../../include/nfs_exports.h"
#include "../../include/export_mgr.h"
#include <stdlib.h>
#include <assert.h>
#include "internal.h"
#include "statx_compat.h"

/**
 * Ceph global module object.
 */
struct ceph_fsal_module CephFSM;

/**
 * The name of this module.
 */
static const char* module_name = "Ceph";

static fsal_staticfsinfo_t default_ceph_info = {
    /* settable */
#if 0
    .umask = 0,
    .xattr_access_rights = 0,
#endif
    /* fixed */
    .symlink_support          = true,
    .link_support             = true,
    .cansettime               = true,
    .no_trunc                 = true,
    .chown_restricted         = true,
    .case_preserving          = true,
#ifdef USE_FSAL_CEPH_SETLK
    .lock_support             = true,
    .lock_support_owner       = true,
    .lock_support_async_block = false,
#endif
    .unique_handles           = true,
    .homogenous               = true,
};

static struct config_item ceph_items[] = 
{
    CONF_ITEM_PATH("ceph_conf", 1, MAXPATHLEN, NULL,
        ceph_fsal_module, conf_path),
    CONF_ITEM_MODE("umask", 0,
        ceph_fsal_module, fs_info.umask),
    CONF_ITEM_MODE("xattr_access_rights", 0,
        ceph_fsal_module, fs_info.xattr_access_rights),
    CONFIG_EOL
};

static struct config_block ceph_block = 
{
    .dbus_interface_name   = "org.ganesha.nfsd.config.fsal.ceph",
    .blk_desc.name         = "Ceph",
    .blk_desc.type         = CONFIG_BLOCK,
    .blk_desc.u.blk.init   = noop_conf_init,
    .blk_desc.u.blk.params = ceph_items,
    .blk_desc.u.blk.commit = noop_conf_commit
};

/* Module methods
 */

/* init_config
 * must be called with a reference taken (via lookup_fsal)
 */

static fsal_status_t init_config(struct fsal_module* module_in,
                                 config_file_t config_struct,
                                 struct config_error_type* err_type)
{
    struct ceph_fsal_module* myself =
    container_of(module_in, struct ceph_fsal_module, fsal);

    LogDebug(COMPONENT_FSAL, "Ceph module setup.");

    myself->fs_info = default_ceph_info;
    (void)load_config_from_parse(config_struct,
                                 &ceph_block,
                                 myself,
                                 true,
                                 err_type);
    if(!config_error_is_harmless(err_type))
        return fsalstat(ERR_FSAL_INVAL, 0);

    return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

#ifdef USE_FSAL_CEPH_LL_LOOKUP_ROOT
static fsal_status_t find_cephfs_root(struct ceph_mount_info *cmount,
                    Inode **pi)
{
    return ceph2fsal_error(ceph_ll_lookup_root(cmount, pi));
}
#else /* USE_FSAL_CEPH_LL_LOOKUP_ROOT */
static fsal_status_t find_cephfs_root(struct ceph_mount_info* cmount,
                                      Inode** pi)
{
    struct stat st;

    return ceph2fsal_error(ceph_ll_walk(cmount, "/", pi, &st));
}
#endif /* USE_FSAL_CEPH_LL_LOOKUP_ROOT */

static struct config_item export_params[] = {
    CONF_ITEM_NOOP("name"),
    CONF_ITEM_STR("user_id", 0, MAXUIDLEN, NULL, export, user_id),
    CONF_ITEM_STR("secret_access_key", 0, MAXSECRETLEN, NULL, export,
        secret_key),
    CONFIG_EOL
};

static struct config_block export_param_block = 
{
    .dbus_interface_name   = "org.ganesha.nfsd.config.fsal.ceph-export%d",
    .blk_desc.name         = "FSAL",
    .blk_desc.type         = CONFIG_BLOCK,
    .blk_desc.u.blk.init   = noop_conf_init,
    .blk_desc.u.blk.params = export_params,
    .blk_desc.u.blk.commit = noop_conf_commit
};

/**
 * @brief Create a new export under this FSAL
 *
 * This function creates a new export object for the Ceph FSAL.
 *
 * @todo ACE: We do not handle re-exports of the same cluster in a
 * sane way.  Currently we create multiple handles and cache objects
 * pointing to the same one.  This is not necessarily wrong, but it is
 * inefficient.  It may also not be something we expect to use enough
 * to care about.
 *
 * @param[in]     module_in  The supplied module handle
 * @param[in]     path       The path to export
 * @param[in]     options    Export specific options for the FSAL
 * @param[in,out] list_entry Our entry in the export list
 * @param[in]     next_fsal  Next stacked FSAL
 * @param[out]    pub_export Newly created FSAL export object
 *
 * @return FSAL status.
 */

static fsal_status_t create_export(struct fsal_module* module_in,
                                   void* parse_node,
                                   struct config_error_type* err_type,
                                   const struct fsal_up_vector* up_ops)
{
    /* The status code to return */
    fsal_status_t status  = { ERR_FSAL_NO_ERROR, 0 };
    /* The internal export object */
    struct export* export = gsh_calloc(1, sizeof(struct export));
    /* The 'private' root handle */
    struct handle* handle = NULL;
    /* Root inode */
    struct Inode* i       = NULL;
    /* Stat for root */
    struct ceph_statx stx;
    /* Return code */
    int rc;
    /* Return code from Ceph calls */
    int ceph_status;
    /* True if we have called fsal_export_init */
    bool initialized = false;

    fsal_export_init(&export->export);
    export_ops_init(&export->export.exp_ops);

    /* get params for this export, if any */
    if(parse_node)
    {
        rc = load_config_from_node(parse_node,
                                   &export_param_block,
                                   export,
                                   true,
                                   err_type);
        if(rc != 0)
        {
            gsh_free(export);
            return fsalstat(ERR_FSAL_INVAL, 0);
        }
    }

    initialized = true;

    /* allocates ceph_mount_info */
    ceph_status = ceph_create(&export->cmount, export->user_id);
    if(ceph_status != 0)
    {
        status.major = ERR_FSAL_SERVERFAULT;
        LogCrit(COMPONENT_FSAL,
            "Unable to create Ceph handle for %s.",
            op_ctx->ctx_export->fullpath);
        goto error;
    }

    ceph_status = ceph_conf_read_file(export->cmount, CephFSM.conf_path);
    if(ceph_status != 0)
    {
        status.major = ERR_FSAL_SERVERFAULT;
        LogCrit(COMPONENT_FSAL,
            "Unable to read Ceph configuration for %s.",
            op_ctx->ctx_export->fullpath);
        goto error;
    }

    if(export->secret_key)
    {
        ceph_status = ceph_conf_set(export->cmount,
                                    "key",
                                    export->secret_key);
        if(ceph_status)
        {
            status.major = ERR_FSAL_INVAL;
            LogCrit(COMPONENT_FSAL,
                "Unable to set Ceph secret key for %s: %d",
                op_ctx->ctx_export->fullpath, ceph_status);
            goto error;
        }
    }

    /*
     * Workaround for broken libcephfs that doesn't handle the path
     * given in ceph_mount properly. Should be harmless for fixed
     * libcephfs as well (see http://tracker.ceph.com/issues/18254).
     */
    ceph_status = ceph_conf_set(export->cmount,
                                "client_mountpoint",
                                op_ctx->ctx_export->fullpath);
    if(ceph_status)
    {
        status.major = ERR_FSAL_INVAL;
        LogCrit(COMPONENT_FSAL,
            "Unable to set Ceph client_mountpoint for %s: %d",
            op_ctx->ctx_export->fullpath, ceph_status);
        goto error;
    }

    ceph_status = ceph_mount(export->cmount, op_ctx->ctx_export->fullpath);
    if(ceph_status != 0)
    {
        status.major = ERR_FSAL_SERVERFAULT;
        LogCrit(COMPONENT_FSAL,
            "Unable to mount Ceph cluster for %s.",
            op_ctx->ctx_export->fullpath);
        goto error;
    }

    if(fsal_attach_export(module_in, &export->export.exports) != 0)
    {
        status.major = ERR_FSAL_SERVERFAULT;
        LogCrit(COMPONENT_FSAL,
            "Unable to attach export for %s.",
            op_ctx->ctx_export->fullpath);
        goto error;
    }

    export->export.fsal = module_in;
    export->export.up_ops = up_ops;

    LogDebug(COMPONENT_FSAL, "Ceph module export %s.",
        op_ctx->ctx_export->fullpath);

    status = find_cephfs_root(export->cmount, &i);
    if(FSAL_IS_ERROR(status))
        goto error;

    rc = fsal_ceph_ll_getattr(export->cmount,
                              i,
                              &stx,
                              CEPH_STATX_HANDLE_MASK,
                              op_ctx->creds);
    if(rc < 0)
    {
        status = ceph2fsal_error(rc);
        goto error;
    }

    construct_handle(&stx, i, export, &handle);

    export->root = handle;
    op_ctx->fsal_export = &export->export;

    return status;

error:
    if(i)
        ceph_ll_put(export->cmount, i);

    if(export)
    {
        if(export->cmount)
            ceph_shutdown(export->cmount);
        gsh_free(export);
    }

    if(initialized)
        initialized = false;

    return status;
}

/**
 * @brief Indicate support for extended operations.
 *
 * @retval true if extended operations are supported.
 */

bool ceph_support_ex(struct fsal_obj_handle* obj)
{
    return true;
}

/**
 * @brief Initialize and register the FSAL
 *
 * This function initializes the FSAL module handle, being called
 * before any configuration or even mounting of a Ceph cluster.  It
 * exists solely to produce a properly constructed FSAL module
 * handle.
 */

MODULE_INIT void init(void)
{
    struct fsal_module* myself = &CephFSM.fsal;

    LogDebug(COMPONENT_FSAL,
        "Ceph module registering.");

    /* register_fsal seems to expect zeroed memory. */
    memset(myself, 0, sizeof(*myself));

    if(register_fsal(myself,
                     module_name,
                     FSAL_MAJOR_VERSION,
                     FSAL_MINOR_VERSION,
                     FSAL_ID_CEPH) != 0)
    {
        /* The register_fsal function prints its own log
           message if it fails */
        LogCrit(COMPONENT_FSAL,
            "Ceph module failed to register.");
    }

    /* Set up module operations */
#ifdef CEPH_PNFS
    myself->m_ops.fsal_pnfs_ds_ops = pnfs_ds_ops_init;
#endif                /* CEPH_PNFS */
    myself->m_ops.create_export = create_export;
    myself->m_ops.init_config   = init_config;
    myself->m_ops.support_ex    = ceph_support_ex;
}

/**
 * @brief Release FSAL resources
 *
 * This function unregisters the FSAL and frees its module handle.
 * The Ceph FSAL has no other resources to release on the per-FSAL
 * level.
 */

MODULE_FINI void finish(void)
{
    LogDebug(COMPONENT_FSAL, "Ceph module finishing.");

    if(unregister_fsal(&CephFSM.fsal) != 0)
    {
        LogCrit(COMPONENT_FSAL, "Unable to unload Ceph FSAL.  Dying with extreme prejudice.");
        abort();
    }
}
