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
 * \file    mfsl_async2_setattrs.c
 * \author  $Author: leibovic $
 * \date    $Date: 2011/05/20 16:08:00 $
 * \version $Revision: 1.0 $
 * \brief   Asynchronous call to setattrs.
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
 * MFSL_async_setattrs: finally perform a synchronous setattrs taken from an asynchronous operation
 * finally perform a synchronous setattrs taken from an asynchronous operation
 *
 * @param p_operation_description   [IN/OUT] asynchronous operation description
 *
 * @return a FSAL status
 */
fsal_status_t MFSL_async_setattrs(mfsl_async_op_desc_t * p_operation_description)
{
    fsal_status_t fsal_status;

    fsal_status = FSAL_setattrs(&p_operation_description->op_args.setattrs.file_handle,
                                &p_operation_description->op_args.setattrs.context,
                                &p_operation_description->op_args.setattrs.attrib_set,
                                &p_operation_description->op_args.setattrs.object_attributes);

    /* Don't forget to update the result */
    p_operation_description->op_res.setattrs.object_attributes =
        p_operation_description->op_args.setattrs.object_attributes;

    return fsal_status;
}

/**
 * MFSL_setattrs: Performs a setattrs on a file asynchronously.
 * Performs a setattrs on a file asynchronously.
 *
 * \param file_handle        [IN]     handle of the object
 * \param p_context          [IN]     authentication context of the user.
 * \param p_mfsl_context     [IN]     mfsl_context.
 * \param attrib_set         [IN]     attributes the caller wants to set
 * \param object_attributes  [IN/OUT] attributes of the object
 * \param pextra             [IN]     not used.
 *
 * \return a FSAL status
 */
fsal_status_t MFSL_setattrs(mfsl_object_t      * filehandle,        /* IN */
                            fsal_op_context_t  * p_context,         /* IN */
                            mfsl_context_t     * p_mfsl_context,    /* IN */
                            fsal_attrib_list_t * attrib_set,        /* IN */
                            fsal_attrib_list_t * object_attributes, /* [ IN/OUT ] */
                            void               * pextra)
{
	fsal_status_t            fsal_status;
    mfsl_async_op_desc_t   * p_async_op_desc=NULL; /* asynchronous operation */
    int                      chosen_synclet;

    SetNameFunction("MFSL_setattrs");


	/* Sanity checks
     ***************/
	if(!p_context || !object_attributes || !attrib_set)
		MFSL_return(ERR_FSAL_INVAL, 0);

    
    /* Asynchronous check access
     ***************************/
	fsal_status = FSAL_setattr_access(p_context, attrib_set, object_attributes);

    if(FSAL_IS_ERROR(fsal_status))
    {
        LogDebug(COMPONENT_MFSL, "Error in setattr_access. Status: (%u.%u).", fsal_status.major, fsal_status.minor);
        MFSL_return(fsal_status.major, 0);
    }

    
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

    /* Choose a synclet to operate on */
    chosen_synclet = MFSL_async_choose_synclet(p_async_op_desc);

    /* Keep in mind this index: it will be used by process_async_op and for scheduling */
    p_async_op_desc->related_synclet_index = chosen_synclet;

    /* Guess attributes
     ******************/
    memcpy((void *) &p_async_op_desc->op_guessed.setattrs.object_attributes,
           (void *) attrib_set,
           sizeof(fsal_attrib_list_t));

    fsal_status = FSAL_merge_attrs(&p_async_op_desc->op_guessed.setattrs.object_attributes,
                                   attrib_set,
                                   &p_async_op_desc->op_guessed.setattrs.object_attributes);
    if(FSAL_IS_ERROR(fsal_status))
        MFSL_return(fsal_status.major, fsal_status.minor);


    /* Populate Asynchronous operation description
     *********************************************/
    p_async_op_desc->op_type = MFSL_ASYNC_OP_SETATTRS;
    p_async_op_desc->op_func = MFSL_async_setattrs;

    p_async_op_desc->op_args.setattrs.file_handle       = filehandle->handle;
    p_async_op_desc->op_args.setattrs.context           = *p_context;
    p_async_op_desc->op_args.setattrs.attrib_set        = *attrib_set;
    p_async_op_desc->op_args.setattrs.object_attributes = *object_attributes;
    
    p_async_op_desc->op_res.setattrs.object_attributes = *object_attributes;

    /** \todo what is that?
     * p_async_op_desc->op_mobject       = ;
     */
    p_async_op_desc->fsal_op_context  = *p_context;
    p_async_op_desc->ptr_mfsl_context = (caddr_t) p_mfsl_context;


    /* Post the asynchronous operation description
     *********************************************/
    fsal_status = MFSL_async_post(p_async_op_desc);

    if(FSAL_IS_ERROR(fsal_status))
        return fsal_status;


    /* return attributes and status
     ******************************/
    *object_attributes = p_async_op_desc->op_guessed.setattrs.object_attributes;

    MFSL_return(ERR_FSAL_NO_ERROR, 0);


}                               /* MFSL_setattrs */
