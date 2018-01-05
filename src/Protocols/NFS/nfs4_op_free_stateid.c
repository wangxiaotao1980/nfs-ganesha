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
  * @file    nfs4_op_free_stateid.c
  * @brief   Routines used for managing the NFS4 COMPOUND functions.
  *
  * Routines used for managing the NFS4 COMPOUND functions.
  *
  *
  */
#include "../../include/config.h"
#include "../../include/log.h"
#include "../../include/gsh_rpc.h"
#include "../../include/nfs4.h"
#include "../../include/nfs_core.h"
#include "../../include/sal_functions.h"
#include "../../include/nfs_proto_functions.h"
#include "../../include/nfs_proto_tools.h"
#include "../../include/gsh_list.h"
#include <stdio.h>
  //#include <string.h>
#include <pthread.h>
#include "hashtable.h"

/**
 *
 * @brief The NFS4_OP_FREE_STATEID operation.
 *
 * This function implements the NFS4_OP_FREE_STATEID operation in
 * nfs4_Compound.
 *
 * @param[in]     op    Arguments for nfs4_op
 * @param[in,out] data  Compound request's data
 * @param[out]    resp  Results for nfs4_op
 *
 * @return per RFC5661 pp. 364-5
 *
 * @see nfs4_Compound
 */

int nfs4_op_free_stateid(struct nfs_argop4* op, compound_data_t* data,
                         struct nfs_resop4* resp)
{
    FREE_STATEID4args* const arg_FREE_STATEID4 __attribute__((unused))
        = &op->nfs_argop4_u.opfree_stateid;
    FREE_STATEID4res* const res_FREE_STATEID4 =
        &resp->nfs_resop4_u.opfree_stateid;
    state_t* state;
    struct fsal_export* save_exp;
    struct fsal_obj_handle* obj;

    resp->resop = NFS4_OP_FREE_STATEID;
    res_FREE_STATEID4->fsr_status = NFS4_OK;

    if (data->minorversion == 0)
        return res_FREE_STATEID4->fsr_status = NFS4ERR_INVAL;

    res_FREE_STATEID4->fsr_status =
        nfs4_Check_Stateid(&arg_FREE_STATEID4->fsa_stateid,
                           NULL,
                           &state,
                           data,
                           STATEID_SPECIAL_CURRENT,
                           0,
                           false,
                           "FREE_STATEID");

    if (res_FREE_STATEID4->fsr_status != NFS4_OK)
        return res_FREE_STATEID4->fsr_status;

    obj = get_state_obj_ref(state);

    if (obj == NULL)
    {
        /* This shouldn't really happen, but check anyway. */
        res_FREE_STATEID4->fsr_status = NFS4ERR_BAD_STATEID;
        return res_FREE_STATEID4->fsr_status;
    }

    PTHREAD_RWLOCK_wrlock(&obj->state_hdl->state_lock);

    if (state->state_type == STATE_TYPE_LOCK &&
        glist_empty(&state->state_data.lock.state_locklist))
    {
        /* At the moment, only return success for a lock stateid with
         * no locks.
         */
         /** @todo: Do we also have to handle other kinds of stateids?
          */
        res_FREE_STATEID4->fsr_status = NFS4_OK;
    }
    else
    {
        res_FREE_STATEID4->fsr_status = NFS4ERR_LOCKS_HELD;
    }

    save_exp = op_ctx->fsal_export;
    op_ctx->fsal_export = state->state_exp;

    state_del_locked(state);

    PTHREAD_RWLOCK_unlock(&obj->state_hdl->state_lock);

    dec_state_t_ref(state);

    op_ctx->fsal_export = save_exp;

    return res_FREE_STATEID4->fsr_status;
} /* nfs41_op_free_stateid */

/**
 * @brief free memory allocated for FREE_STATEID result
 *
 * This function frees memory allocated for the NFS4_OP_FREE_STATEID
 * result.
 *
 * @param[in,out] resp nfs4_op results
 *
 */
void nfs4_op_free_stateid_Free(nfs_resop4* resp)
{
    /* Nothing to be done */
}
