/*******************************************************************************************
 *  @file      fsal_kodo_export.c 2018\1\11 13:46:18 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com> (中文编码测试)
 *******************************************************************************************/
#include "fsal_kodo_internal.h"




void kodo_export_ops_init(struct export_ops *ops)
{

                                                        /** Export information*/
                                                        /* get_name               //01 */

                                                        /** Export lifecycle management.*/
                                                        /* unexport               //02 */
                                                        /* release                //03 */

                                                        /* /** Create an object handles within this export */
                                                        /* lookup_path            //04 */
                                                        /* lookup_junction        //05 */
                                                        /* wire_to_host           //06 */
                                                        /* host_to_key            //07 */
                                                        /* create_handle          //08 */

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

                                                        /* alloc_state            //31 */
                                                        /* free_state             //32 */
                                                        /* is_superuser           //33 */
}
// 
// ------------------------------------------------------------------------------------------
