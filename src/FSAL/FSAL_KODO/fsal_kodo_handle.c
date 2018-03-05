/*******************************************************************************************
 *  @file      fsal_kodo_handle.c 2018\1\10 17:40:02 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com> (中文编码测试)
 *******************************************************************************************/
#include "../../include/config.h"
#include "../../include/fsal_api.h"
#include "fsal_kodo_internal.h"
// ------------------------------------------------------------------------------------------

/**
 * @brief Release an object
 *
 * This function looks up an object by name in a directory.
 *
 * @param[in] obj_pub The object to release
 *
 * @return FSAL status codes.
 */
static void kodo_release(struct fsal_obj_handle *obj_pub)
{

}

/**
 * @brief Look up an object by name
 *
 * This function looks up an object by name in a directory.
 *
 * @param[in]  dir_pub The directory in which to look up the object.
 * @param[in]  path    The name to look up.
 * @param[out] obj_pub The looked up object.
 *
 * @return FSAL status codes.
 */
static fsal_status_t kodo_lookup(struct fsal_obj_handle* dir_pub,
                                 const char *path, 
                                 struct fsal_obj_handle** obj_pub,
                                 struct attrlist *attrs_out)
{
    fsal_status_t fsal_status = { ERR_FSAL_NO_ERROR, 0 };

    return fsal_status;
}

/**
 * @brief Read a directory
 *
 * This function reads the contents of a directory (excluding . and
 * .., which is ironic since the Ceph readdir call synthesizes them
 * out of nothing) and passes dirent information to the supplied
 * callback.
 *
 * @param[in]  dir_pub     The directory to read
 * @param[in]  whence      The cookie indicating resumption, NULL to start
 * @param[in]  dir_state   Opaque, passed to cb
 * @param[in]  cb          Callback that receives directory entries
 * @param[out] eof         True if there are no more entries
 *
 * @return FSAL status.
 */
static fsal_status_t kodo_readdir(struct fsal_obj_handle *dir_pub,
                                  fsal_cookie_t *whence,
                                  void *dir_state,
                                  fsal_readdir_cb cb,
                                  attrmask_t attrmask,
                                  bool *eof)
{
    fsal_status_t fsal_status = { ERR_FSAL_NO_ERROR, 0 };

    return fsal_status;
}


/**
* @brief Manage closing a file when a state is no longer needed.
*
* When the upper layers are ready to dispense with a state, this method is
* called to allow the FSAL to close any file descriptors or release any other
* resources associated with the state. A call to free_state should be assumed
* to follow soon.
*
* @param[in] obj_hdl    File on which to operate
* @param[in] state      state_t to use for this operation
*
* @return FSAL status.
*/
static fsal_status_t kodo_close2(struct fsal_obj_handle* obj_hdl, struct state_t* state)
{
    int rc;
    struct kodo_state_fd* open_state = NULL;

    struct kodo_export* export = container_of(op_ctx->fsal_export, struct kodo_export, pub);
    struct kodo_handle* handle = container_of(obj_hdl, struct kodo_handle, pub);

    LogFullDebug(COMPONENT_FSAL, "%s enter obj_hdl %p state %p", __func__, obj_hdl, state);

    if (state)
    {
        open_state = (struct kodo_state_fd *)state;

        LogFullDebug(COMPONENT_FSAL, "%s called w/open_state %p", __func__, open_state);

        if (state->state_type == STATE_TYPE_SHARE ||
            state->state_type == STATE_TYPE_NLM_SHARE ||
            state->state_type == STATE_TYPE_9P_FID)
        {
            /* This is a share state, we must update the share
            * counters.  This can block over an I/O operation.
            */
            PTHREAD_RWLOCK_wrlock(&obj_hdl->obj_lock);

            //update_share_counters(&handle->share,
            //    handle->openflags,
            //    FSAL_O_CLOSED);

            PTHREAD_RWLOCK_unlock(&obj_hdl->obj_lock);
        }
    }
    //else if (handle->openflags == FSAL_O_CLOSED)
    //{
    //    return fsalstat(ERR_FSAL_NOT_OPENED, 0);
    //}

    //rc = rgw_close(export->rgw_fs, handle->rgw_fh, RGW_CLOSE_FLAG_NONE);
    //if (rc < 0)
    //{
    //    return rgw2fsal_error(rc);
    //}
    //handle->openflags = FSAL_O_CLOSED;

    return fsalstat(ERR_FSAL_NO_ERROR, 0);
}


/**
* @brief Close a file
*
* This function closes a file.  It is protected by the Cache inode
* content lock.  This should return ERR_FSAL_NOT_OPENED if the global FD for
* this obj was not open.
*
* @param[in] obj_hdl File to close
*
* @return FSAL status.
*/
static fsal_status_t kodo_close(struct fsal_obj_handle* obj_hdl)
{
    return kodo_close2(obj_hdl, NULL);
}



/**
* @brief Override functions in ops vector
*
* This function overrides implemented functions in the ops vector
* with versions for this FSAL.
*
* @param[in] ops Handle operations vector
*/

void kodo_handle_ops_init(struct fsal_obj_ops *ops)
{
                                                                            /**Lifecycle management*/
                                                                            /* ops->get_ref                  //01 */
                                                                            /* ops->put_ref                  //02 */
    ops->release = kodo_release;    //01                                    /* ops->release                  //03 */
                                                                            /* ops->merge                    //04 */
                                                                            /** Directory operations*/
    ops->lookup = kodo_lookup;      //02                                    /* ops->lookup                   //05 */
    ops->readdir = kodo_readdir;    //03                                    /* ops->readdir                  //06 */
                                                                            /* ops->compute_readdir_cookie   //07 */
                                                                            /* ops->dirent_cmp               //08 */
                                                                            /** Creation operations*/
                                                                            /* ops->create                   //09 */
                                                                            /* ops->mkdir                    //10 */
                                                                            /* ops->mknode                   //11 */
                                                                            /* ops->symlink                  //12 */
                                                                            /**File object operations*/
                                                                            /* ops->readlink                 //13 */
                                                                            /* ops->test_access              //14 */
                                                                            /* ops->getattrs                 //15 */
                                                                            /* ops->setattrs                 //16 */
                                                                            /* ops->link                     //17 */
                                                                            /* ops->fs_locations             //18 */
                                                                            /* ops->rename                   //19 */
                                                                            /* ops->unlink                   //20 */
                                                                            /**I/O management*/
                                                                            /* ops->open                     //21 */
                                                                            /* ops->reopen                   //22 */
                                                                            /* ops->status                   //23 */
                                                                            /* ops->read                     //24 */
                                                                            /* ops->read_plus                //25 */
                                                                            /* ops->write                    //26 */
                                                                            /* ops->write_plus               //27 */
                                                                            /* ops->seek                     //28 */
                                                                            /* ops->io_advise                //29 */
                                                                            /* ops->commit                   //30 */
                                                                            /* ops->lock_op                  //31 */
                                                                            /* ops->share_op                 //32 */
                                                                            /* ops->close                    //33 */
                                                                            /**Extended attribute management*/
                                                                            /* ops->list_ext_attrs           //34 */
                                                                            /* ops->getextattr_id_by_name    //35 */
                                                                            /* ops->getextattr_value_by_name //36 */
                                                                            /* ops->getextattr_value_by_id   //37 */
                                                                            /* ops->setextattr_value         //38 */
                                                                            /* ops->setextattr_value_by_id   //39 */
                                                                            /* ops->remove_extattr_by_id     //40 */
                                                                            /* ops->remove_extattr_by_name   //41 */
                                                                            /**Handle operations*/
                                                                            /* ops->handle_is                //42 */
                                                                            /* ops->handle_to_wire           //43 */
                                                                            /* ops->handle_to_key            //44 */
                                                                            /* ops->handle_cmp               //45 */
                                                                            /** pNFS functions*/
                                                                            /* ops->layoutget                //46 */
                                                                            /* ops->layoutreturn             //47 */
                                                                            /* ops->layoutcommit             //48 */
                                                                            /* ops->getxattrs                //49 */
                                                                            /* ops->setxattrs                //50 */
                                                                            /* ops->removexattrs             //51 */
                                                                            /* ops->listxattrs               //52 */
                                                                            /**Extended API functions.*/
                                                                            /* ops->open2                    //53 */
                                                                            /* ops->check_verifier           //54 */
                                                                            /* ops->status2                  //55 */
                                                                            /* ops->reopen2                  //56 */
                                                                            /* ops->read2                    //57 */
                                                                            /* ops->write2                   //58 */
                                                                            /* ops->seek2                    //59 */
                                                                            /* ops->io_advise2               //60 */
                                                                            /* ops->commit2                  //61 */
                                                                            /* ops->lock_op2                 //62 */
                                                                            /* ops->setattr2                 //63 */
                                                                            /* ops->close2                   //64 */
}
// 
// ------------------------------------------------------------------------------------------
