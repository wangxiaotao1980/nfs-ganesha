/*
 * Copyright (C) Red Hat  Inc., 2015
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
 * License along with this library.
 *
 * ---------------------------------------
 */

/**
 * @file    fsal_up.c
 *
 * @author  Soumya Koduri <skoduri@redhat.com>
 *
 * @brief   Upcall Interface for FSAL_GLUSTER
 *
 */

#include "../../include/config.h"
#include "../../include/fsal.h"
#include "../../include/fsal_up.h"
#include "../../include/fsal_convert.h"

#include <sys/types.h>
#include <unistd.h>
#include <utime.h>
#include <sys/time.h>

#include "gluster_internal.h"


int upcall_inode_invalidate(struct glusterfs_fs* gl_fs,
                            struct glfs_object* object)
{
    int rc = -1;
    glfs_t* fs = NULL;
    char vol_uuid[GLAPI_UUID_LENGTH] = { '\0' };
    unsigned char globjhdl[GLAPI_HANDLE_LENGTH];
    struct gsh_buffdesc key;
    const struct fsal_up_vector* event_func;
    fsal_status_t fsal_status = { 0, 0 };

    fs = gl_fs->fs;
    if(!fs)
    {
        LogCrit(COMPONENT_FSAL_UP,
            "Invalid fs object of the glusterfs_fs(%p)",
            gl_fs);
        goto out;
    }

    rc = glfs_h_extract_handle(object, globjhdl + GLAPI_UUID_LENGTH,
                               GFAPI_HANDLE_LENGTH);
    if(rc < 0)
    {
        LogDebug(COMPONENT_FSAL_UP,
            "glfs_h_extract_handle failed %p",
            fs);
        goto out;
    }

    rc = glfs_get_volumeid(fs, vol_uuid,
                           GLAPI_UUID_LENGTH);
    if(rc < 0)
    {
        LogDebug(COMPONENT_FSAL_UP,
            "glfs_get_volumeid failed %p",
            fs);
        goto out;
    }

    memcpy(globjhdl, vol_uuid, GLAPI_UUID_LENGTH);
    key.addr = &globjhdl;
    key.len = GLAPI_HANDLE_LENGTH;

    LogDebug(COMPONENT_FSAL_UP, "Received event to process for %p",
        fs);

    event_func = gl_fs->up_ops;

    fsal_status = event_func->invalidate_close(
                                               event_func,
                                               &key,
                                               FSAL_UP_INVALIDATE_CACHE);

    rc = fsal_status.major;
    if(FSAL_IS_ERROR(fsal_status) && fsal_status.major != ERR_FSAL_NOENT)
    {
        LogWarn(COMPONENT_FSAL_UP,
            "Inode_Invalidate event could not be processed for fd %p, rc %d",
            gl_fs->fs, rc);
    }

out:
    return rc;
}

void* GLUSTERFSAL_UP_Thread(void* Arg)
{
    struct glusterfs_fs* gl_fs = Arg;
    struct fsal_up_vector* event_func;
    char thr_name[16];
    int rc = 0;
    struct glfs_upcall* cbk = NULL;
    struct glfs_upcall_inode* in_arg = NULL;
    enum glfs_upcall_reason reason = 0;
    int retry = 0;
    int errsv = 0;
    struct glfs_object* object = NULL;
    struct glfs_object* p_object = NULL;
    struct glfs_object* oldp_object = NULL;


    snprintf(thr_name, sizeof(thr_name),
             "fsal_up_%p",
             gl_fs->fs);
    SetNameFunction(thr_name);

    /* Set the FSAL UP functions that will be used to process events. */
    event_func = (struct fsal_up_vector *)gl_fs->up_ops;

    if(event_func == NULL)
    {
        LogFatal(COMPONENT_FSAL_UP,
            "FSAL up vector does not exist. Can not continue.");
        gsh_free(Arg);
        return NULL;
    }

    LogFullDebug(COMPONENT_FSAL_UP,
        "Initializing FSAL Callback context for %p.",
        gl_fs->fs);

    if(!gl_fs->fs)
    {
        LogCrit(COMPONENT_FSAL_UP,
            "FSAL Callback interface - Null glfs context.");
        goto out;
    }

    /* wait for upcall readiness */
    up_ready_wait(event_func);

    /* Start querying for events and processing. */
    /** @todo : Do batch processing instead */
    while(!atomic_fetch_int8_t(&gl_fs->destroy_mode))
    {
        LogFullDebug(COMPONENT_FSAL_UP,
            "Requesting event from FSAL Callback interface for %p.",
            gl_fs->fs);

        reason = 0;

        rc = glfs_h_poll_upcall(gl_fs->fs, &cbk);
        errsv = errno;

        if(rc != 0)
        {
            /* if ENOMEM retry for couple of times
             * and then exit
             */
            if((errsv == ENOMEM) && (retry < 10))
            {
                sleep(1);
                retry++;
                continue;
            }
            else
            {
                switch(errsv)
                {
                    case ENOMEM :
                        LogMajor(COMPONENT_FSAL_UP,
                            "Memory allocation failed during poll_upcall for (%p).",
                            gl_fs->fs);
                        abort();

                    case ENOTSUP :
                        LogEvent(COMPONENT_FSAL_UP,
                            "Upcall feature is not supported for (%p).",
                            gl_fs->fs);
                        break;
                    default :
                        LogCrit(COMPONENT_FSAL_UP,
                            "Poll upcall failed for %p. rc %d errno %d (%s) reason %d",
                            gl_fs->fs, rc, errsv,
                            strerror(errsv), reason)
                        
                        ;
                }
                return NULL;
            }
        }

        retry = 0;

        LogFullDebug(COMPONENT_FSAL_UP,
            "Received upcall event: reason(%d)",
            reason);

        if(!cbk)
        {
            usleep(10);
            continue;
        }

        reason = glfs_upcall_get_reason(cbk);
        /* Decide what type of event this is
         * inode update / invalidate? */
        switch(reason)
        {
            case GLFS_UPCALL_EVENT_NULL :
                usleep(10);
                continue;
            case GLFS_UPCALL_INODE_INVALIDATE :
                in_arg = glfs_upcall_get_event(cbk);

                if(!in_arg)
                {
                    /* Could be ENOMEM issues. continue */
                    LogWarn(COMPONENT_FSAL_UP,
                        "Received NULL upcall event arg");
                    break;
                }

                object = glfs_upcall_inode_get_object(in_arg);
                if(object)
                    upcall_inode_invalidate(gl_fs, object);
                p_object = glfs_upcall_inode_get_pobject(in_arg);
                if(p_object)
                    upcall_inode_invalidate(gl_fs, p_object);
                oldp_object = glfs_upcall_inode_get_oldpobject(in_arg);
                if(oldp_object)
                    upcall_inode_invalidate(gl_fs, oldp_object);
                break;
            default :
                LogWarn(COMPONENT_FSAL_UP, "Unknown event: %d", reason);
                continue;
        }
        if(cbk)
        {
            glfs_free(cbk);
            cbk = NULL;
        }
    }

out:
    return NULL;
} /* GLUSTERFSFSAL_UP_Thread */
