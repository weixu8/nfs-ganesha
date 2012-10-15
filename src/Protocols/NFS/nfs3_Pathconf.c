/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
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
 * @file    nfs3_Pathconf.c
 * @brief   Routines used for managing the NFS4 COMPOUND functions.
 *
 * Routines used for managing the NFS4 COMPOUND functions.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _SOLARIS
#include "solaris_port.h"
#endif

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/file.h>           /* for having FNDELAY */
#include "HashData.h"
#include "HashTable.h"
#include "log.h"
#include "nfs23.h"
#include "nfs4.h"
#include "mount.h"
#include "nfs_core.h"
#include "cache_inode.h"
#include "nfs_exports.h"
#include "nfs_creds.h"
#include "nfs_proto_functions.h"
#include "nfs_tools.h"
#include "nfs_proto_tools.h"

/**
 * @brief Implements NFSPROC3_PATHCONF
 *
 * Implements NFSPROC3_PATHCONF.
 *
 * @param[in]  arg     NFS arguments union
 * @param[in]  export  NFS export list
 * @param[in]  req_ctx Request context
 * @param[in]  worker  Worker thread data
 * @param[in]  req     SVC request related to this call
 * @param[out] res     Structure to contain the result of the call
 *
 * @retval NFS_REQ_OK if successful
 * @retval NFS_REQ_DROP if failed but retryable
 * @retval NFS_REQ_FAILED if failed and not retryable
 */

int
nfs3_Pathconf(nfs_arg_t *arg,
              exportlist_t *export,
              struct req_op_context *req_ctx,
              nfs_worker_data_t *worker,
              struct svc_req *req,
              nfs_res_t *res)
{
        cache_entry_t *entry = NULL;
        int rc = NFS_REQ_OK;
        struct fsal_export *exp_hdl = export->export_hdl;

        if (isDebug(COMPONENT_NFSPROTO)) {
                char str[LEN_FH_STR];
                sprint_fhandle3(str, &(arg->arg_pathconf3.object));
                LogDebug(COMPONENT_NFSPROTO,
                         "REQUEST PROCESSING: Calling nfs3_Pathconf handle: "
                         "%s", str);
        }

        /* to avoid setting it on each error case */
        res->res_pathconf3.PATHCONF3res_u.resfail.obj_attributes
                .attributes_follow = FALSE;

        /* Convert file handle into a fsal_handle */
        entry = nfs_FhandleToCache(req_ctx,
                                   req->rq_vers, 
                                   &arg->arg_pathconf3.object,
                                   NULL, 
                                   &res->res_pathconf3.status, NULL,
                                   export, &rc);
        if (entry == NULL) {
                goto out;
        }

        res->res_pathconf3.PATHCONF3res_u.resok.linkmax
                = exp_hdl->ops->fs_maxlink(exp_hdl);
        res->res_pathconf3.PATHCONF3res_u.resok.name_max
                = exp_hdl->ops->fs_maxnamelen(exp_hdl);
        res->res_pathconf3.PATHCONF3res_u.resok.no_trunc
                = exp_hdl->ops->fs_supports(exp_hdl, fso_no_trunc);
        res->res_pathconf3.PATHCONF3res_u.resok.chown_restricted
                = exp_hdl->ops->fs_supports(exp_hdl, fso_chown_restricted);
        res->res_pathconf3.PATHCONF3res_u.resok.case_insensitive
                = exp_hdl->ops->fs_supports(exp_hdl, fso_case_insensitive);
        res->res_pathconf3.PATHCONF3res_u.resok.case_preserving
                = exp_hdl->ops->fs_supports(exp_hdl, fso_case_preserving);

        /* Build post op file attributes */
        nfs_SetPostOpAttr(entry,
                          req_ctx,
                          &(res->res_pathconf3
                            .PATHCONF3res_u.resok.obj_attributes));

out:

        if (entry) {
                cache_inode_put(entry);
        }

        return rc;
}                               /* nfs3_Pathconf */

/**
 * @brief Free the result structure allocated for nfs3_Pathconf.
 *
 * This function free the result structure allocated for nfs3_Pathconf.
 *
 * @param[in,out] res Result structure.
 *
 */
void nfs3_Pathconf_Free(nfs_res_t *res)
{
        return;
} /* nfs3_Pathconf_Free */
