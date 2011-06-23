/*
 *
 *
 * Copyright CEA/DAM/DIF  (2008)
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * ---------------------------------------
 */

/**
 * \file    mfsl_async2_unlink.c
 * \author  $Author: leibovic $
 * \date    $Date: 2011/05/13 14:30:00 $
 * \version $Revision: 1.0 $
 * \brief   Asynchronous call to unlink.
 *
 */

#include "config.h"

/* fsal_types contains constants and type definitions for FSAL */
#include "fsal_types.h"
#include "fsal.h"
#include "mfsl_types.h"
#include "mfsl.h"
#include "common_utils.h"

/* We want to use memcpy() */
#include <string.h>
/* We want to use gettimeofday() */
#include <sys/time.h>

/******************************************************
 *              Common Filesystem calls.
 ******************************************************/

/**
 * MFSL_async_unlink: finally perform a synchronous operation taken from an asynchronous operation
 * finally perform a synchronous operation taken from an asynchronous operation
 *
 * @param p_operation_description   [IN/OUT] asynchronous operation description
 *
 * @return a FSAL status
 */
fsal_status_t MFSL_async_unlink(mfsl_async_op_desc_t * p_operation_description)
{
    fsal_status_t fsal_status;

    fsal_status = FSAL_unlink(&p_operation_description->op_args.unlink.parentdir_handle,
                              &p_operation_description->op_args.unlink.object_name,
                              &p_operation_description->op_args.unlink.context,
                              &p_operation_description->op_args.unlink.parentdir_attributes);

    /* Don't forget to update the result */
    p_operation_description->op_res.unlink.parentdir_attributes =
        p_operation_description->op_args.unlink.parentdir_attributes;

    return fsal_status;
}

/**
 * MFSL_unlink: Unlinks a file asynchronously.
 * Unlinks a file asynchronously.
 *
 * @param parentdir_handle        [IN]     handle of the parent directory.
 * @param p_object_name           [IN]     name of the object.
 * @param object_handle           [IN/OUT] handle of the object.
 * @param p_context               [IN]     fsal_context.
 * @param p_mfsl_context          [IN]     mfsl_context.
 * @param p_parentdir_attributes  [IN/OUT] attributes for the parentdir.
 * @param pextra                  [IN]     attributes of the object.
 *
 * @return a FSAL status
 */
fsal_status_t MFSL_unlink(mfsl_object_t      * parentdir_handle,       /* IN */
                          fsal_name_t        * p_object_name,          /* IN */
                          mfsl_object_t      * object_handle,          /* INOUT */
                          fsal_op_context_t  * p_context,              /* IN */
                          mfsl_context_t     * p_mfsl_context,         /* IN */
                          fsal_attrib_list_t * p_parentdir_attributes, /* [IN/OUT ] */
                          void               * pextra                  /* IN */
        )
{
    fsal_attrib_list_t   * p_object_attributes;          /* object attributes; we have to see if it's a dir. */
    mfsl_async_op_desc_t * p_async_op_desc=NULL;         /* asynchronous operation */
    fsal_status_t          fsal_status;                  /* status we check asyncly */

    SetNameFunction("MFSL_unlink");

    /* Sanity checks
     ***************/
    if(!p_context || !p_parentdir_attributes || !pextra)
	    MFSL_return(ERR_FSAL_FAULT, 0);

    /* pextra contains object's attributes */
    p_object_attributes = (fsal_attrib_list_t *) pextra;


    /* Asynchronous check
     ********************/
    fsal_status = FSAL_unlink_access(p_context, p_parentdir_attributes, p_object_attributes);

    if(FSAL_IS_ERROR(fsal_status))
        MFSL_return(fsal_status.major, 0);


    /* Asynchronous operation description construction
     *************************************************/
    LogDebug(COMPONENT_MFSL, "Gets an asyncop from pool in context %p.", p_mfsl_context);

    P(p_mfsl_context->lock);
    GetFromPool(p_async_op_desc, &p_mfsl_context->pool_async_op, mfsl_async_op_desc_t);
    V(p_mfsl_context->lock);

    if(p_async_op_desc == NULL)
    {
        LogCrit(COMPONENT_MFSL, "Could not get an asynchronous operation from Pool.");
        MFSL_return(ERR_FSAL_SERVERFAULT, 0);
    }

    if(gettimeofday(&p_async_op_desc->op_time, NULL) != 0)
    {
        /* Could'not get time of day... Stopping, this may need a major failure */
        LogMajor(COMPONENT_MFSL, "Cannot get time of day... exiting");
        exit(1);
    }


    /* Guess attributes
     ******************/
    memcpy((void *) &p_async_op_desc->op_guessed.unlink.parentdir_attributes,
           (void *) p_parentdir_attributes,
           sizeof(fsal_attrib_list_t));

    /* if it's a directory, there is 1 link less to its parent dir */
    if(p_object_attributes->type == FSAL_TYPE_DIR)
	    p_async_op_desc->op_guessed.unlink.parentdir_attributes.numlinks -= 1;

    #ifdef _USE_PROXY
    /* PROXY sees filesize in octets */
    #else
    p_async_op_desc->op_guessed.unlink.parentdir_attributes.filesize -= 1;
    #endif
    p_async_op_desc->op_guessed.unlink.parentdir_attributes.ctime.seconds  = time(NULL);
    p_async_op_desc->op_guessed.unlink.parentdir_attributes.ctime.nseconds = 0;
    p_async_op_desc->op_guessed.unlink.parentdir_attributes.mtime.seconds  =
        p_async_op_desc->op_guessed.unlink.parentdir_attributes.ctime.seconds;
    p_async_op_desc->op_guessed.unlink.parentdir_attributes.mtime.nseconds =
        p_async_op_desc->op_guessed.unlink.parentdir_attributes.ctime.nseconds;


    /* Populate asynchronous operation description
     *********************************************/
    p_async_op_desc->op_type                             = MFSL_ASYNC_OP_UNLINK;
    p_async_op_desc->op_func                             = MFSL_async_unlink;
    p_async_op_desc->op_args.unlink.parentdir_handle     = parentdir_handle->handle;
    p_async_op_desc->op_args.unlink.object_name          = *p_object_name;
    p_async_op_desc->op_args.unlink.context              = *p_context;
    p_async_op_desc->op_args.unlink.parentdir_attributes = *p_parentdir_attributes;
    p_async_op_desc->op_res.unlink.parentdir_attributes  = *p_parentdir_attributes;
    /* \todo what is that?
     * p_async_op_desc->op_mobject                             = object_handle;
     ****/
    p_async_op_desc->fsal_op_context                     = *p_context;
    p_async_op_desc->ptr_mfsl_context                    = (caddr_t) p_mfsl_context;


    /* Post the asynchronous operation description to the dispatcher
     ***************************************************************/
    fsal_status = MFSL_async_post(p_async_op_desc);

    if(FSAL_IS_ERROR(fsal_status))
        return fsal_status;


    /* Return attributes and status
     ******************************/
    *p_parentdir_attributes = p_async_op_desc->op_guessed.unlink.parentdir_attributes;

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
}                               /* MFSL_unlink */
