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
 * \file    mfsl_async2_link.c
 * \author  $Author: leibovic $
 * \date    $Date: 2011/05/19 14:01:00 $
 * \version $Revision: 1.0 $
 * \brief   Asynchronous call to link.
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
 * MFSL_async_link: finally perform a synchronous link taken from an asynchronous operation
 * finally perform a synchronous link taken from an asynchronous operation
 *
 * @param p_operation_description   [IN/OUT] asynchronous operation description
 *
 * @return a FSAL status
 */
fsal_status_t MFSL_async_link(mfsl_async_op_desc_t * p_operation_description)
{
    fsal_status_t fsal_status;

    fsal_status = FSAL_link(&p_operation_description->op_args.link.target_handle,
                            &p_operation_description->op_args.link.dir_handle,
                            &p_operation_description->op_args.link.link_name,
                            &p_operation_description->op_args.link.context,
                            &p_operation_description->op_args.link.linked_object_attributes);

    /* Don't forget to update the result */
    p_operation_description->op_res.link.linked_object_attributes =
        p_operation_description->op_args.link.linked_object_attributes;

    return fsal_status;
}

/**
 * MFSL_link: Links a file asynchronously.
 * Links a file asynchronously.
 *
 * @param target_handle      [IN]     handle of the target object.
 * @param dir_handle         [IN]     handle of the directory where the hardlink is to be created.
 * @param p_link_name        [IN]     name of the hardlink to be crerated.
 * @param p_context          [IN]     authentication context of the user.
 * @param p_mfsl_context     [IN]     mfsl_context.
 * @param p_attr_obj         [IN/OUT] The post_operation attributes of the linked object.
 * @param pextra             [IN]     destination and source directory attributes.
 *
 * @return a FSAL status
 */
fsal_status_t MFSL_link(mfsl_object_t      * target_handle,  /* IN */
                        mfsl_object_t      * dir_handle,     /* IN */
                        fsal_name_t        * p_link_name,    /* IN */
                        fsal_op_context_t  * p_context,      /* IN */
                        mfsl_context_t     * p_mfsl_context, /* IN */
                        fsal_attrib_list_t * p_attr_obj,     /* [ IN/OUT ] */
                        void               * pextra)         /* IN */
{
	fsal_status_t fsal_status;

	mfsl_dirs_attributes_t * dirs_attrs;
	fsal_attrib_list_t     * p_attr_srcdir;        /* given source directory attributes */
	fsal_attrib_list_t     * p_attr_destdir;       /* given destination directory attributes */

    mfsl_async_op_desc_t   * p_async_op_desc=NULL; /* asynchronous operation */

    SetNameFunction("MFSL_link");


	/* Sanity checks 
     ***************/
	if(!p_context || !p_attr_destdir || !pextra)
		MFSL_return(ERR_FSAL_FAULT, 0);

	/* pextra contains destination and source directory attributes */
	dirs_attrs     = (mfsl_dirs_attributes_t *) pextra;
	p_attr_destdir = dirs_attrs->src_dir_attrs;
	p_attr_srcdir  = dirs_attrs->dest_dir_attrs;

	/* Are theses directories? */
	if( (p_attr_srcdir->type != FSAL_TYPE_DIR) || (p_attr_destdir->type != FSAL_TYPE_DIR) )
		MFSL_return(ERR_FSAL_NOTDIR, 0);


    /* Asynchronous check
     ********************/
    fsal_status = FSAL_link_access(p_context, p_attr_destdir, p_attr_srcdir);
    
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
    memcpy((void *) &p_async_op_desc->op_guessed.link.linked_object_attributes,
           (void *) p_attr_obj,
           sizeof(fsal_attrib_list_t));

    p_async_op_desc->op_guessed.link.linked_object_attributes.numlinks      +=1;
    p_async_op_desc->op_guessed.link.linked_object_attributes.ctime.seconds  = time(NULL);
    p_async_op_desc->op_guessed.link.linked_object_attributes.ctime.nseconds = 0;


    /* Populate Asynchronous operation description
     *********************************************/
    p_async_op_desc->op_type          = MFSL_ASYNC_OP_LINK;
    p_async_op_desc->op_func          = MFSL_async_link;

    p_async_op_desc->op_args.link.target_handle            = target_handle->handle;
    p_async_op_desc->op_args.link.dir_handle               = dir_handle->handle;
    p_async_op_desc->op_args.link.link_name                = *p_link_name;
    p_async_op_desc->op_args.link.context                  = *p_context;
    p_async_op_desc->op_args.link.linked_object_attributes = *p_attr_obj;
    
    p_async_op_desc->op_res.link.linked_object_attributes = *p_attr_obj;

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
    *p_attr_obj = p_async_op_desc->op_guessed.link.linked_object_attributes;

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
}                               /* MFSL_link */


