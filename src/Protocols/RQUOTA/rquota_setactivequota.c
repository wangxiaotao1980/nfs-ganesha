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

//#include "../../include/config.h"
#include "../../include/log.h"
#include "../../include/gsh_rpc.h"
//#include "../../include/nfs23.h"
//#include "../../include/nfs4.h"
//#include "../../include/nfs_core.h"
//#include "../../include/nfs_exports.h"
//#include "../../include/mount.h"
//#include "../../include/rquota.h"
#include "../../include/nfs_proto_functions.h"
//#include "../../include/hashtable.h"
//#include <stdio.h>
//#include <string.h>
//#include <pthread.h>
//#include <fcntl.h>
#include <sys/file.h>

/**
 * @brief The Rquota setactivequota function, for all versions.
 *
 * @param[in]  arg     Ignored
 * @param[in]  req     Ignored
 * @param[out] res     Ignored
 */

int rquota_setactivequota(nfs_arg_t *arg, struct svc_req *req, nfs_res_t *res)
{
	LogFullDebug(COMPONENT_NFSPROTO,
		     "REQUEST PROCESSING: Calling rquota_setactivequota");
	/* 0 is success */
	return 0;
}

/**
 * @brief Frees the result structure allocated for rquota_setactivequota
 *
 * @param[in,out] res Pointer to the result structure.
 *
 */
void rquota_setactivequota_Free(nfs_res_t *res)
{
	/* Nothing to do */
}
