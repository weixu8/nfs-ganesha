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
 * @file    nfs3_Fsinfo.c
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
#include "nfs_proto_tools.h"
#include "nfs_tools.h"

/**
 * @brief Implement NFSPROC3_FSINFO
 *
 * This function Implements NFSPROC3_FSINFO.
 *
 * @todo ACE: This function uses a lot of constants instead of
 * retrieving information from the FSAL as it ought.
 *
 * @param[in]  arg     NFS arguments union
 * @param[in]  export  NFS export list
 * @param[in]  req_ctx Credentials to be used for this request
 * @param[in]  worker  Worker thread data
 * @param[in]  req     SVC request related to this call
 * @param[out] res     Structure to contain the result of the call
 *
 * @retval NFS_REQ_OK if successful
 * @retval NFS_REQ_DROP if failed but retryable
 * @retval NFS_REQ_FAILED if failed and not retryable
 */

int
nfs3_Fsinfo(nfs_arg_t *arg,
            exportlist_t *export,
            struct req_op_context *req_ctx,
            nfs_worker_data_t *worker,
            struct svc_req *req,
            nfs_res_t *res)
{
        cache_entry_t *entry = NULL;
        int rc = NFS_REQ_OK;

        if (isDebug(COMPONENT_NFSPROTO)) {
                char str[LEN_FH_STR];
                sprint_fhandle3(str, &(arg->arg_fsinfo3.fsroot));
                LogDebug(COMPONENT_NFSPROTO,
                         "REQUEST PROCESSING: Calling nfs3_Fsinfo handle: %s",
                         str);
        }

        /* To avoid setting it on each error case */
        res->res_fsinfo3.FSINFO3res_u.resfail.obj_attributes.attributes_follow
                = FALSE;

        entry = nfs_FhandleToCache(req_ctx,
                                   req->rq_vers, 
                                   &arg->arg_fsinfo3.fsroot,
                                   NULL, &res->res_fsinfo3.status, NULL,
                                   export, &rc);
        if (entry == NULL) {
                goto out;
        }

        /* New fields were added to nfs_config_t to handle this
           value. We use them */

        FSINFO3resok *const FSINFO_FIELD
                = &res->res_fsinfo3.FSINFO3res_u.resok;

        FSINFO_FIELD->rtmax = export->MaxRead;
        FSINFO_FIELD->rtpref = export->PrefRead;
        /* This field is generally unused, it will be removed in V4 */
        FSINFO_FIELD->rtmult = DEV_BSIZE;

        FSINFO_FIELD->wtmax = export->MaxWrite;
        FSINFO_FIELD->wtpref = export->PrefWrite;
        /* This field is generally unused, it will be removed in V4 */
        FSINFO_FIELD->wtmult = DEV_BSIZE;

        FSINFO_FIELD->dtpref = export->PrefReaddir;

        FSINFO_FIELD->maxfilesize = FSINFO_MAX_FILESIZE;
        FSINFO_FIELD->time_delta.seconds = 1;
        FSINFO_FIELD->time_delta.nseconds = 0;

        LogFullDebug(COMPONENT_NFSPROTO,
                     "rtmax = %d | rtpref = %d | trmult = %d",
                     FSINFO_FIELD->rtmax,
                     FSINFO_FIELD->rtpref,
                     FSINFO_FIELD->rtmult);
        LogFullDebug(COMPONENT_NFSPROTO,
                     "wtmax = %d | wtpref = %d | wrmult = %d",
                     FSINFO_FIELD->wtmax,
                     FSINFO_FIELD->wtpref,
                     FSINFO_FIELD->wtmult);
        LogFullDebug(COMPONENT_NFSPROTO,
                     "dtpref = %d | maxfilesize = %llu ",
                     FSINFO_FIELD->dtpref,
                     FSINFO_FIELD->maxfilesize);

        /* Allow all kinds of operations to be performed on the server
           through NFS v3 */
        FSINFO_FIELD->properties = FSF3_LINK | FSF3_SYMLINK |
                FSF3_HOMOGENEOUS | FSF3_CANSETTIME;

        nfs_SetPostOpAttr(entry,
                          req_ctx,
                          &(res->res_fsinfo3.FSINFO3res_u.resok
                            .obj_attributes));
        res->res_fsinfo3.status = NFS3_OK;

out:

        if (entry) {
                cache_inode_put(entry);
        }

        return rc;
}                               /* nfs3_Fsinfo */

/**
 * @brief Free the result structure allocated for nfs3_Fsinfo.
 *
 * This function frees the result structure allocated for nfs3_Fsinfo.
 *
 * @param[in,out] res The result structure
 *
 */
void nfs3_Fsinfo_Free(nfs_res_t *res)
{
        return;
} /* nfs3_Fsinfo_Free */
