/*
 * vim:noexpandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright (C) Red Hat  Inc., 2013
 * Author: Anand Subramanian anands@redhat.com
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * -------------
 */

/**
 * @file main.c
 * @author Anand Subramanian <anands@redhat.com>
 *
 * @author Shyamsundar R     <srangana@redhat.com>
 *
 * @brief Module core functions for FSAL_GLUSTER functionality, init etc.
 *
 */

#include "../../include/fsal.h"
#include "../../include/FSAL/fsal_init.h"
//#include "../../include/FSAL/fsal_commonlib.h"
#include "gluster_internal.h"

/* GLUSTERFS FSAL module private storage
 */

const char glfsal_name[] = "GLUSTER";

/* filesystem info for GLUSTERFS */
static struct fsal_staticfsinfo_t default_gluster_info = {
    .maxfilesize                     = UINT64_MAX,
    .maxlink                         = _POSIX_LINK_MAX,
    .maxnamelen                      = 1024,
    .maxpathlen                      = 1024,
    .no_trunc                        = true,
    .chown_restricted                = true,
    .case_insensitive                = false,
    .case_preserving                 = true,
    .link_support                    = true,
    .symlink_support                 = true,
    .lock_support                    = true,
    .lock_support_owner              = true,
    .lock_support_async_block        = false,
    .named_attr                      = true,
    .unique_handles                  = true,
    .lease_time                      = { 10, 0 },
    .acl_support                     = FSAL_ACLSUPPORT_ALLOW | FSAL_ACLSUPPORT_DENY,
    .cansettime                      = true,
    .homogenous                      = true,
    .supported_attrs                 = GLUSTERFS_SUPPORTED_ATTRIBUTES,
    .maxread                         = 0,
    .maxwrite                        = 0,
    .umask                           = 0,
    .auth_exportpath_xdev            = false,
    .xattr_access_rights             = 0400, /* root=RW, owner=R */
    .pnfs_mds                        = false,
    .pnfs_ds                         = true,
    .link_supports_permission_checks = true,
};

static struct config_item glfs_params[] = 
{
    CONF_ITEM_BOOL("pnfs_mds", false,
                   fsal_staticfsinfo_t, pnfs_mds),
    CONF_ITEM_BOOL("pnfs_ds", true,
                   fsal_staticfsinfo_t, pnfs_ds),
    CONFIG_EOL
};

struct config_block glfs_param = 
{
    .dbus_interface_name   = "org.ganesha.nfsd.config.fsal.gluster",
    .blk_desc.name         = "GLUSTER",
    .blk_desc.type         = CONFIG_BLOCK,
    .blk_desc.u.blk.init   = noop_conf_init,
    .blk_desc.u.blk.params = glfs_params,
    .blk_desc.u.blk.commit = noop_conf_commit
};

static fsal_status_t init_config(struct fsal_module* fsal_hdl,
                                 config_file_t config_struct,
                                 struct config_error_type* err_type)
{
    struct glusterfs_fsal_module* glfsal_module =
            container_of(fsal_hdl, struct glusterfs_fsal_module, fsal);


    glfsal_module->fs_info = default_gluster_info;
    (void)load_config_from_parse(config_struct,
                                 &glfs_param,
                                 &glfsal_module->fs_info,
                                 true,
                                 err_type);

    /*
     * Global block is not mandatory, so evenif
     * it is not parsed correctly, don't consider
     * that as an error
     */
    if(!config_error_is_harmless(err_type))
        LogDebug(COMPONENT_FSAL, "Parsing Export Block failed");

    display_fsinfo(&glfsal_module->fs_info);

    return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

/**
 * @brief Indicate support for extended operations.
 *
 * @retval true if extended operations are supported.
 */

bool glusterfs_support_ex(struct fsal_obj_handle* obj)
{
    return true;
}

/* Module methods
 */

MODULE_INIT

void glusterfs_init(void)
{
    struct fsal_module* myself = &GlusterFS.fsal;

    /* register_fsal seems to expect zeroed memory. */
    memset(myself, 0, sizeof(*myself));

    if(register_fsal(myself, glfsal_name, FSAL_MAJOR_VERSION,
                     FSAL_MINOR_VERSION, FSAL_ID_GLUSTER) != 0)
    {
        LogCrit(COMPONENT_FSAL,
                "Gluster FSAL module failed to register.");
        return;
    }

    /* set up module operations */
    myself->m_ops.create_export = glusterfs_create_export;

    /* setup global handle internals */
    myself->m_ops.init_config = init_config;

    /* Enable extended fop support */
    myself->m_ops.support_ex = glusterfs_support_ex;
    /*
     * Following inits needed for pNFS support
     * get device info will used by pnfs meta data server
     */
    myself->m_ops.getdeviceinfo = getdeviceinfo;
    myself->m_ops.fsal_pnfs_ds_ops = pnfs_ds_ops_init;

    PTHREAD_MUTEX_init(&GlusterFS.lock, NULL);
    glist_init(&GlusterFS.fs_obj);

    LogDebug(COMPONENT_FSAL, "FSAL Gluster initialized");
}

MODULE_FINI

void glusterfs_unload(void)
{
    if(unregister_fsal(&GlusterFS.fsal) != 0)
    {
        LogCrit(COMPONENT_FSAL,
                "FSAL Gluster unable to unload.  Dying ...");
        return;
    }

    /* All the shares should have been unexported */
    if(!glist_empty(&GlusterFS.fs_obj))
    {
        LogWarn(COMPONENT_FSAL,
                "FSAL Gluster still contains active shares.");
    }
    PTHREAD_MUTEX_destroy(&GlusterFS.lock);
    LogDebug(COMPONENT_FSAL, "FSAL Gluster unloaded");
}
