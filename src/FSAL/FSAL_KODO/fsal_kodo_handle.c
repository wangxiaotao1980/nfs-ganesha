/*******************************************************************************************
 *  @file      fsal_kodo_handle.c 2018\1\10 17:40:02 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com> (中文编码测试)
 *******************************************************************************************/
#include "../../include/config.h"
#include "../../include/fsal_api.h"
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
    return fsalstat(0, 0);
}



/**
* @brief Override functions in ops vector
*
* This function overrides implemented functions in the ops vector
* with versions for this FSAL.
*
* @param[in] ops Handle operations vector
*/

void handle_ops_init(struct fsal_obj_ops *ops)
{
    ops->release = kodo_release;
    ops->lookup = kodo_lookup;

    /**Lifecycle management*/
    /* ops->get_ref = NULL;                  //01 */
    /* ops->put_ref = NULL;                  //02 */
    ops->release = kodo_release;             //03                   //I01
    /* ops->merge   = NULL;                  //04 */
    /** Directory operations*/
    ops->lookup                 = NULL;      //05                   //I02
    /* ops->readdir                = NULL;   //06 */
    /* ops->compute_readdir_cookie = NULL;   //07 */
    /* ops->dirent_cmp             = NULL;   //08 */
    /** Creation operations*/
    /* ops->create  = NULL;                  //09 */
    /* ops->mkdir   = NULL;                  //10 */
    /* ops->mknode  = NULL;                  //11 */
    /* ops->symlink = NULL;                  //12 */
    /**File object operations*/
    /* ops->readlink     = NULL;             //13 */
    /* ops->test_access  = NULL;             //14 */
    /* ops->getattrs     = NULL;             //15 */
    /* ops->setattrs     = NULL;             //16 */
    /* ops->link         = NULL;             //17 */
    /* ops->fs_locations = NULL;             //18 */
    /* ops->rename       = NULL;             //19 */
    /* ops->unlink       = NULL;             //20 */
    /**I/O management*/
    /* ops->open       = NULL;               //21 */
    /* ops->reopen     = NULL;               //22 */
    /* ops->status     = NULL;               //23 */
    /* ops->read       = NULL;               //24 */
    /* ops->read_plus  = NULL;               //25 */
    /* ops->write      = NULL;               //26 */
    /* ops->write_plus = NULL;               //27 */
    /* ops->seek       = NULL;               //28 */
    /* ops->io_advise  = NULL;               //29 */
    /* ops->commit     = NULL;               //30 */
    /* ops->lock_op    = NULL;               //31 */
    /* ops->share_op   = NULL;               //32 */
    /* ops->close      = NULL;               //33 */
    /**Extended attribute management*/
    /* ops->list_ext_attrs           = NULL; //34 */
    /* ops->getextattr_id_by_name    = NULL; //35 */
    /* ops->getextattr_value_by_name = NULL; //36 */
    /* ops->getextattr_value_by_id   = NULL; //37 */
    /* ops->setextattr_value         = NULL; //38 */
    /* ops->setextattr_value_by_id   = NULL; //39 */
    /* ops->remove_extattr_by_id     = NULL; //40 */
    /* ops->remove_extattr_by_name   = NULL; //41 */
    /**Handle operations*/
    /* ops->handle_is      = NULL;           //42 */
    /* ops->handle_to_wire = NULL;           //43 */
    /* ops->handle_to_key  = NULL;           //44 */
    /* ops->handle_cmp     = NULL;           //45 */
    /** psNFS functions*/
    /* ops->layoutget    = NULL;             //46 */
    /* ops->layoutreturn = NULL;             //47 */
    /* ops->layoutcommit = NULL;             //48 */
    /* ops->getxattrs    = NULL;             //49 */
    /* ops->setxattrs    = NULL;             //50 */
    /* ops->removexattrs = NULL;             //51 */
    /* ops->listxattrs   = NULL;             //52 */
    /**Extended API functions.*/
    /* ops->open2          = NULL;           //53 */
    /* ops->check_verifier = NULL;           //54 */
    /* ops->status2        = NULL;           //55 */
    /* ops->reopen2        = NULL;           //56 */
    /* ops->read2          = NULL;           //57 */
    /* ops->write2         = NULL;           //58 */
    /* ops->seek2          = NULL;           //59 */
    /* ops->io_advise2     = NULL;           //60 */
    /* ops->commit2        = NULL;           //61 */
    /* ops->lock_op2       = NULL;           //62 */
    /* ops->setattr2       = NULL;           //63 */
    /* ops->close2         = NULL;           //64 */
    


}
// 
// ------------------------------------------------------------------------------------------
