/*
 * vim:noexpandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright CEA/DAM/DIF  (2008)
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
 *
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
 * ---------------------------------------
 */

/**
 * @file    nfs4_op_reclaim_complete.c
 * @brief   Routines used for managing the NFS4 COMPOUND functions.
 *
 * Routines used for managing the NFS4 COMPOUND functions.
 *
 *
 */
  //#include "../../include/config.h"
#include "../../include/hashtable.h"
//#include "../../include/log.h"
#include "../../include/gsh_rpc.h"
//#include "../../include/nfs23.h"
//#include "../../include/nfs4.h"
//#include "../../include/mount.h"
#include "../../include/nfs_core.h"
//#include "../../include/nfs_exports.h"
#include "../../include/nfs_proto_functions.h"
//#include "../../include/nfs_file_handle.h"
#include "../../include/sal_data.h"

#include <stdio.h>
//#include <string.h>
//#include <pthread.h>
//#include <fcntl.h>
//#include <sys/file.h>


/**
 *
 * @brief The NFS4_OP_RECLAIM_COMPLETE4 operation.
 *
 * This function implements the NFS4_OP_RECLAIM_COMPLETE4 operation.
 *
 * @param[in]     op    Arguments for the nfs4_op
 * @param[in,out] data  Compound request's data
 * @param[out]    resp  Retuls for the nfs4_op
 *
 * @return per RFC5661 p. 372
 *
 * @see nfs4_Compound
 *
 */

int nfs4_op_reclaim_complete(struct nfs_argop4* op, compound_data_t* data,
                             struct nfs_resop4* resp)
{
    RECLAIM_COMPLETE4args* const arg_RECLAIM_COMPLETE4
        __attribute__((unused))
        = &op->nfs_argop4_u.opreclaim_complete;
    RECLAIM_COMPLETE4res* const res_RECLAIM_COMPLETE4 =
        &resp->nfs_resop4_u.opreclaim_complete;

    resp->resop = NFS4_OP_RECLAIM_COMPLETE;

    res_RECLAIM_COMPLETE4->rcr_status = NFS4_OK;

    if (data->minorversion == 0)
    {
        res_RECLAIM_COMPLETE4->rcr_status = NFS4ERR_INVAL;
        return res_RECLAIM_COMPLETE4->rcr_status;
    }

    if (data->session == NULL)
    {
        res_RECLAIM_COMPLETE4->rcr_status = NFS4ERR_OP_NOT_IN_SESSION;
        return res_RECLAIM_COMPLETE4->rcr_status;
    }

    /* For now, we don't handle rca_one_fs, so we won't complain about
     * complete already for it.
     */
    if (data->session->clientid_record->cid_cb.v41.cid_reclaim_complete &&
        !arg_RECLAIM_COMPLETE4->rca_one_fs)
    {
        res_RECLAIM_COMPLETE4->rcr_status = NFS4ERR_COMPLETE_ALREADY;
        return res_RECLAIM_COMPLETE4->rcr_status;
    }

    if (!arg_RECLAIM_COMPLETE4->rca_one_fs)
    {
        data->session->clientid_record->cid_cb.v41.
            cid_reclaim_complete = true;
    }

    return res_RECLAIM_COMPLETE4->rcr_status;
} /* nfs41_op_reclaim_complete */

/**
 * @brief Free memory allocated for RECLAIM_COMPLETE result
 *
 * This function frees anty memory allocated for the result of the
 * NFS4_OP_RECLAIM_COMPLETE operation.
 */
void nfs4_op_reclaim_complete_Free(nfs_resop4* resp)
{
    /* Nothing to be done */
}
