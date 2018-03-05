/*******************************************************************************************
 *  @file      fsal_kodo_export.c 2018\1\11 13:46:18 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com> (中文编码测试)
 *******************************************************************************************/
#include "fsal_kodo_internal.h"
#include "../../include/FSAL/fsal_commonlib.h"


/**
 * @brief Finalize an export
 *
 * This function is called as part of cleanup when the last reference to
 * an export is released and it is no longer part of the list.  It
 * should clean up all private resources and destroy the object.
 *
 * @param[in] exp_hdl The export to release.
 */
static void kodo_release(struct fsal_export *exp_hdl)
{
    struct kodo_export* pKodoExport = container_of(exp_hdl, struct kodo_export, pub);

    //todo: 内部数据的操作，释放自己的数据
    gsh_free(pKodoExport);
    pKodoExport = NULL;
}


/**
 * @brief Convert a wire handle to a host handle
 *
 * This function extracts a host handle from a wire handle.  That
 * is, when given a handle as passed to a client, this method will
 * extract the handle to create objects.
 *
 * @param[in]     exp_hdl Export handle
 * @param[in]     in_type Protocol through which buffer was received.
 * @param[in]     flags   Flags to describe the wire handle. Example, if
 *			  the handle is a big endian handle.
 * @param[in,out] fh_desc Buffer descriptor.  The address of the
 *                        buffer is given in @c fh_desc->buf and must
 *                        not be changed.  @c fh_desc->len is the
 *                        length of the data contained in the buffer,
 *                        @c fh_desc->len must be updated to the correct
 *                        host handle size.
 *
 * @return FSAL type.
 */
static fsal_status_t kodo_wire_to_host
    (
        struct fsal_export*   exp_hdl,
        fsal_digesttype_t     in_type,
        struct gsh_buffdesc*  fh_desc,
        int                   flags
    )
{
    switch (in_type)
    {
        /* Digested Handles */
        case FSAL_DIGEST_NFSV3:
        case FSAL_DIGEST_NFSV4:
            /* wire handles */
            fh_desc->len = sizeof(struct rgw_fh_hk);
            break;
        default:
            return fsalstat(ERR_FSAL_SERVERFAULT, 0);
    }

    return fsalstat(ERR_FSAL_NO_ERROR, 0);
}

/**
 * @brief Create a FSAL object handle from a host handle
 *
 * This function creates a FSAL object handle from a host handle
 * (when an object is no longer in cache but the client still remembers
 * the handle).
 *
 * The caller will set the request_mask in attrs_out to indicate the attributes
 * of interest. ATTR_ACL SHOULD NOT be requested and need not be provided. If
 * not all the requested attributes can be provided, this method MUST return
 * an error unless the ATTR_RDATTR_ERR bit was set in the request_mask.
 *
 * Since this method instantiates a new fsal_obj_handle, it will be forced
 * to fetch at least some attributes in order to even know what the object
 * type is (as well as it's fileid and fsid). For this reason, the operation
 * as a whole can be expected to fail if the attributes were not able to be
 * fetched.
 *
 * @param[in]     exp_hdl   The export in which to create the handle
 * @param[in]     hdl_desc  Buffer descriptor for the host handle
 * @param[out]    handle    FSAL object handle
 * @param[in,out] attrs_out Optional attributes for newly created object
 *
 * @note On success, @a handle has been ref'd
 *
 * @return FSAL status.
 */
static fsal_status_t kodo_create_handle
    (
        struct fsal_export*      exp_hdl,
        struct gsh_buffdesc*     fh_desc,
        struct fsal_obj_handle** handle,
        struct attrlist*         attrs_out
    )
{
    /* Full 'private' export structure */
    struct kodo_export *pKodoExport 
        = container_of(exp_hdl, struct kodo_export, pub);
    /* FSAL status to return */
    fsal_status_t status = { ERR_FSAL_NO_ERROR, 0 };




    return status;
}


/**
* @brief Allocate a state_t structure
*
* Note that this is not expected to fail since memory allocation is
* expected to abort on failure.
*
* @param[in] exp_hdl               Export state_t will be associated with
* @param[in] state_type            Type of state to allocate
* @param[in] related_state         Related state if appropriate
*
* @returns a state structure.
*/

static struct state_t* kodo_alloc_state
    (
        struct fsal_export* exp_hdl,
        enum state_type state_type,
        struct state_t* related_state
    )
{
    struct state_t*    state;
    struct kodo_fs_fd* my_fd;

    state = init_state(gsh_calloc(1, sizeof(struct kodo_state_fd)), exp_hdl, state_type, related_state);

    my_fd = &container_of(state, struct kodo_state_fd, state)->kodo_fs_fd;

    //todo : 设定自己的 kodo_fs_fd

    return state;
}

/**
* @brief Free a state_t structure
*
* @param[in] exp_hdl               Export state_t is associated with
* @param[in] state                 state_t structure to free.
*
* @returns NULL on failure otherwise a state structure.
*/
static void kodo_free_state(struct fsal_export* exp_hdl, struct state_t* state)
{
    struct kodo_state_fd* state_fd = container_of(state, struct kodo_state_fd, state);

    gsh_free(state_fd);
}



void kodo_export_ops_init(struct export_ops *ops)
{

                                                        /** Export information*/
                                                        /* get_name               //01 */

                                                        /** Export lifecycle management.*/
                                                        /* unexport               //02 */
    ops->release = kodo_release;                        /* release                //03 */

                                                        /* /** Create an object handles within this export */
                                                        /* lookup_path            //04 */
                                                        /* lookup_junction        //05 */
    ops->wire_to_host = kodo_wire_to_host;              /* wire_to_host           //06 */
                                                        /* host_to_key            //07 */
    ops->create_handle = kodo_create_handle;            /* create_handle          //08 */

                                                        /** Statistics and configuration for this filesystem */
                                                        /* get_fs_dynamic_info    //09 */
                                                        /* fs_supports            //10 */
                                                        /* fs_maxfilesize         //11 */
                                                        /* fs_maxread             //12 */
                                                        /* fs_maxwrite            //13 */
                                                        /* fs_maxlink             //14 */
                                                        /* fs_maxnamelen          //15 */
                                                        /* fs_maxpathlen          //16 */
                                                        /* fs_lease_time          //17 */
                                                        /* fs_acl_support         //18 */
                                                        /* fs_supported_attrs     //19 */
                                                        /* fs_umask               //20 */
                                                        /* fs_xattr_access_rights //21 */

                                                        /** Quotas are managed at the file system (export) level.
                                                         *  Someone who uses quotas, please look over these comments to check/expand them.*/
                                                        /* check_quota            //22 */
                                                        /* get_quota              //23 */
                                                        /* set_quota              //24 */
                                                        /* /**pNFS functions*/
                                                        /* getdevicelist          //25 */
                                                        /* fs_layouttypes         //26 */
                                                        /* fs_layout_blocksize    //27 */
                                                        /* fs_maximum_segments    //28 */
                                                        /* fs_loc_body_size       //29 */
                                                        /* get_write_verifier     //30 */

    ops->alloc_state = kodo_alloc_state;                /* alloc_state            //31 */
    ops->free_state  = kodo_free_state;                 /* free_state             //32 */
                                                        /* is_superuser           //33 */
}
// 
// ------------------------------------------------------------------------------------------
