/*
 * Copyright CEA/DAM/DIF  2010
 *  Author: Philippe Deniel (philippe.deniel@cea.fr)
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
 */

#include "../../include/config.h"
#include "../../include/os/quota.h"
#include "../../include/hashtable.h"
#include "../../include/log.h"
#include "../../include/gsh_rpc.h"
#include "../../include/nfs23.h"
#include "../../include/nfs4.h"
#include "../../include/nfs_core.h"
#include "../../include/nfs_exports.h"
 //#include "../../include/mount.h"
#include "../../include/rquota.h"
#include "../../include/nfs_proto_functions.h"
#include "../../include/export_mgr.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
//#include <fcntl.h>
#include <sys/file.h>

static int do_rquota_setquota(char* quota_path, int quota_type,
                              int quota_id,
                              sq_dqblk* quota_dqblk,
                              setquota_rslt* qres);

/**
 * @brief The Rquota setquota function, for all versions.
 *
 * The RQUOTA setquota function, for all versions.
 *
 * @param[in]  arg     quota args
 * @param[in]  req     contains quota version
 * @param[out] res     returned quota (modified)
 *
 */
int rquota_setquota(nfs_arg_t* arg, struct svc_req* req, nfs_res_t* res)
{
    char* quota_path;
    int quota_id;
    int quota_type = USRQUOTA;
    struct sq_dqblk* quota_dqblk;
    setquota_rslt* qres = &res->res_rquota_setquota;

    LogFullDebug(COMPONENT_NFSPROTO,
                 "REQUEST PROCESSING: Calling rquota_setquota");

    /* check rquota version and extract arguments */
    if (req->rq_msg.cb_vers == EXT_RQUOTAVERS)
    {
        quota_path  = arg->arg_ext_rquota_setquota.sqa_pathp;
        quota_id    = arg->arg_ext_rquota_setquota.sqa_id;
        quota_type  = arg->arg_ext_rquota_setquota.sqa_type;
        quota_dqblk = &arg->arg_ext_rquota_setquota.sqa_dqblk;
    }
    else
    {
        quota_path  = arg->arg_rquota_setquota.sqa_pathp;
        quota_id    = arg->arg_rquota_setquota.sqa_id;
        quota_dqblk = &arg->arg_rquota_setquota.sqa_dqblk;
    }

    return do_rquota_setquota(quota_path,
                              quota_type,
                              quota_id,
                              quota_dqblk,
                              qres);
} /* rquota_setquota */

static int do_rquota_setquota(char* quota_path, int quota_type,
                              int quota_id,
                              sq_dqblk* quota_dqblk,
                              setquota_rslt* qres)
{
    fsal_status_t fsal_status;
    fsal_quota_t fsal_quota_in;
    fsal_quota_t fsal_quota_out;
    struct gsh_export* exp;
    char* qpath;
    char path[MAXPATHLEN];

    qres->status = Q_EPERM;

    qpath = check_handle_lead_slash(quota_path,
                                    path,
                                    MAXPATHLEN);
    if (qpath == NULL)
        goto out;

    /*  Find the export for the dirname (using as well Path, Pseudo, or Tag)
     */
    if (qpath[0] != '/')
    {
        LogFullDebug(COMPONENT_NFSPROTO,
                     "Searching for export by tag for %s",
                     qpath);
        exp = get_gsh_export_by_tag(qpath);
        if (exp != NULL)
        {
            /* By Tag must use fullpath for actual request. */
            qpath = exp->fullpath;
        }
    }
    else if (nfs_param.core_param.mount_path_pseudo)
    {
        LogFullDebug(COMPONENT_NFSPROTO,
                     "Searching for export by pseudo for %s",
                     qpath);
        exp = get_gsh_export_by_pseudo(qpath, false);
        if (exp != NULL)
        {
            /* By Pseudo must use fullpath for actual request. */
            qpath = exp->fullpath;
        }
    }
    else
    {
        LogFullDebug(COMPONENT_NFSPROTO,
                     "Searching for export by path for %s",
                     qpath);
        exp = get_gsh_export_by_path(qpath, false);
    }

    if (exp == NULL)
    {
        /* No export found, return ACCESS error. */
        LogEvent(COMPONENT_NFSPROTO,
                 "Export entry for %s not found", qpath);

        /* entry not found. */
        goto out;
    }

    memset(&fsal_quota_in, 0, sizeof(fsal_quota_t));
    memset(&fsal_quota_out, 0, sizeof(fsal_quota_t));

    fsal_quota_in.bhardlimit = quota_dqblk->rq_bhardlimit;
    fsal_quota_in.bsoftlimit = quota_dqblk->rq_bsoftlimit;
    fsal_quota_in.curblocks = quota_dqblk->rq_curblocks;
    fsal_quota_in.fhardlimit = quota_dqblk->rq_fhardlimit;
    fsal_quota_in.fsoftlimit = quota_dqblk->rq_fsoftlimit;
    fsal_quota_in.btimeleft = quota_dqblk->rq_btimeleft;
    fsal_quota_in.ftimeleft = quota_dqblk->rq_ftimeleft;

    fsal_status = exp->fsal_export->exp_ops.set_quota(exp->fsal_export,
                                                      qpath,
                                                      quota_type,
                                                      quota_id,
                                                      &fsal_quota_in,
                                                      &fsal_quota_out);
    if (FSAL_IS_ERROR(fsal_status))
    {
        if (fsal_status.major == ERR_FSAL_NO_QUOTA)
            qres->status = Q_NOQUOTA;
        goto out;
    }

    /* is success */

    qres->setquota_rslt_u.sqr_rquota.rq_active = TRUE;
    qres->setquota_rslt_u.sqr_rquota.rq_bhardlimit =
        fsal_quota_out.bhardlimit;
    qres->setquota_rslt_u.sqr_rquota.rq_bsoftlimit =
        fsal_quota_out.bsoftlimit;
    qres->setquota_rslt_u.sqr_rquota.rq_curblocks =
        fsal_quota_out.curblocks;
    qres->setquota_rslt_u.sqr_rquota.rq_fhardlimit =
        fsal_quota_out.fhardlimit;
    qres->setquota_rslt_u.sqr_rquota.rq_fsoftlimit =
        fsal_quota_out.fsoftlimit;
    qres->setquota_rslt_u.sqr_rquota.rq_btimeleft =
        fsal_quota_out.btimeleft;
    qres->setquota_rslt_u.sqr_rquota.rq_ftimeleft =
        fsal_quota_out.ftimeleft;
    qres->status = Q_OK;

out:
    return NFS_REQ_OK;
} /* do_rquota_setquota */

/**
 * @brief Frees the result structure allocated for rquota_setquota
 *
 * @param[in,out] res Pointer to the result structure.
 *
 */
void rquota_setquota_Free(nfs_res_t* res)
{
    /* Nothing to do */
}
