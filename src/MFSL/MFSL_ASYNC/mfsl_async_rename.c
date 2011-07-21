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
 * \file    mfsl_async_rename.c
 * \author  $Author: leibovic $
 * \date    $Date: 2011/05/20 14:08:00 $
 * \version $Revision: 1.0 $
 * \brief   Asynchronous call to rename.
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
 * MFSL_async_rename: finally perform a synchronous rename taken from an asynchronous operation
 * finally perform a synchronous rename taken from an asynchronous operation
 *
 * @param p_operation_description   [IN/OUT] asynchronous operation description
 *
 * @return a FSAL status
 */
fsal_status_t MFSL_async_rename(mfsl_async_op_desc_t * p_operation_description)
{
    fsal_status_t fsal_status;

    fsal_status = FSAL_rename(&p_operation_description->op_args.rename.src_parentdir_handle,
                              &p_operation_description->op_args.rename.src_name,
                              &p_operation_description->op_args.rename.tgt_parentdir_handle,
                              &p_operation_description->op_args.rename.tgt_name,
                              &p_operation_description->op_args.rename.context,
                              &p_operation_description->op_args.rename.src_dir_attributes,
                              &p_operation_description->op_args.rename.tgt_dir_attributes);

    /* Don't forget to update the result */
    p_operation_description->op_res.rename.src_dir_attributes =
        p_operation_description->op_args.rename.src_dir_attributes;

    p_operation_description->op_res.rename.tgt_dir_attributes =
        p_operation_description->op_args.rename.tgt_dir_attributes;

    return fsal_status;
}

/**
 * MFSL_rename: Renames a file asynchronously.
 * Renames a file asynchronously.
 *
 * \param old_parentdir_handle [IN]     handle of the source directory
 * \param p_old_name           [IN]     old_name of the object to be renamed
 * \param new_parentdir_handle [IN]     handle of the destination directory
 * \param p_new_name           [IN]     new name of the object to be renamed
 * \param p_context            [IN]     authentication context of the user.
 * \param p_mfsl_context       [IN]     mfsl_context.
 * \param psrc_dir_attributes  [IN/OUT] attributes of the source directory
 * \param ptgt_dir_attributes  [IN/OUT] attributes of the target directory
 * \param pextra               [IN]     old object attributes.
 *
 * \return a FSAL status
 */
fsal_status_t MFSL_rename(mfsl_object_t      * old_parentdir_handle, /* IN */
                          fsal_name_t        * p_old_name,           /* IN */
                          mfsl_object_t      * new_parentdir_handle, /* IN */
                          fsal_name_t        * p_new_name,           /* IN */
                          fsal_op_context_t  * p_context,            /* IN */
                          mfsl_context_t     * p_mfsl_context,       /* IN */
                          fsal_attrib_list_t * psrc_dir_attributes,  /* [ IN/OUT ] */
                          fsal_attrib_list_t * ptgt_dir_attributes,  /* [ IN/OUT ] */
                          void               * pextra)               /* IN */
{
    /** @todo change this to add object handle */
	fsal_status_t fsal_status;

	fsal_attrib_list_t attr_new_srcdir;
	fsal_attrib_list_t attr_new_destdir;
	fsal_attrib_list_t * attr_old_obj;
    int                  chosen_synclet;

	int samedirs;

    mfsl_async_op_desc_t   * p_async_op_desc=NULL; /* asynchronous operation */

    SetNameFunction("MFSL_rename");


	/* Sanity checks
     ***************/
	if(!psrc_dir_attributes || !ptgt_dir_attributes || !pextra)
		MFSL_return(ERR_FSAL_FAULT, 0);

    /* Are these directories? */
	if( (psrc_dir_attributes->type != FSAL_TYPE_DIR) || (ptgt_dir_attributes->type != FSAL_TYPE_DIR) )
		MFSL_return(ERR_FSAL_NOTDIR, 0);

	/* pextra contains object attributes */
	attr_old_obj = pextra;


	/* Asynchronous check access
     ***************************/
	fsal_status = FSAL_rename_access(p_context, psrc_dir_attributes, ptgt_dir_attributes, attr_old_obj);

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

    /* Choose a synclet to operate on */
    chosen_synclet = MFSL_async_choose_synclet(p_async_op_desc);

    /* Keep in mind this index: it will be used by process_async_op and for scheduling */
    p_async_op_desc->related_synclet_index = chosen_synclet;

    /* Guess attributes
     ******************/
	/* Source directory attributes */
	memcpy((void *) &p_async_op_desc->op_guessed.rename.src_dir_attributes,
           (void *) psrc_dir_attributes,
           sizeof(fsal_attrib_list_t));

    /* ctime */
    p_async_op_desc->op_guessed.rename.src_dir_attributes.ctime.seconds = time(NULL);
    p_async_op_desc->op_guessed.rename.src_dir_attributes.ctime.nseconds = 0;
    /* mtime */
    p_async_op_desc->op_guessed.rename.src_dir_attributes.mtime.seconds =
        p_async_op_desc->op_guessed.rename.src_dir_attributes.ctime.seconds;
    p_async_op_desc->op_guessed.rename.src_dir_attributes.mtime.nseconds =
        p_async_op_desc->op_guessed.rename.src_dir_attributes.ctime.nseconds;

	/* Destination directory attributes */
	memcpy((void *) &p_async_op_desc->op_guessed.rename.tgt_dir_attributes,
           (void *) ptgt_dir_attributes,
           sizeof(fsal_attrib_list_t));

    /* ctime */
    p_async_op_desc->op_guessed.rename.tgt_dir_attributes.ctime.seconds =
        p_async_op_desc->op_guessed.rename.src_dir_attributes.ctime.seconds;
    p_async_op_desc->op_guessed.rename.tgt_dir_attributes.ctime.nseconds =
        p_async_op_desc->op_guessed.rename.src_dir_attributes.ctime.nseconds;
    /* mtime */
    p_async_op_desc->op_guessed.rename.tgt_dir_attributes.mtime.seconds =
        p_async_op_desc->op_guessed.rename.src_dir_attributes.ctime.seconds;
    p_async_op_desc->op_guessed.rename.tgt_dir_attributes.mtime.nseconds =
        p_async_op_desc->op_guessed.rename.src_dir_attributes.ctime.nseconds;

	/* those directories' size change only if they're different */
	samedirs = FSAL_handlecmp(&old_parentdir_handle->handle, &new_parentdir_handle->handle, &fsal_status);

	if(FSAL_IS_ERROR(fsal_status))
    {
        LogMajor(COMPONENT_MFSL, "Something happened when comparing directories handles.");
		MFSL_return(fsal_status.major, 0);
    }

	if(!samedirs)
	{
		/* different dirs */
        p_async_op_desc->op_guessed.rename.src_dir_attributes.filesize -= 1;
        p_async_op_desc->op_guessed.rename.tgt_dir_attributes.filesize += 1;
	}


    /* Populate Asynchronous operation description
     *********************************************/
    p_async_op_desc->op_type = MFSL_ASYNC_OP_RENAME;
    p_async_op_desc->op_func = MFSL_async_rename;

    p_async_op_desc->op_args.rename.src_parentdir_handle = old_parentdir_handle->handle;
    p_async_op_desc->op_args.rename.src_name             = *p_old_name;
    p_async_op_desc->op_args.rename.tgt_parentdir_handle = new_parentdir_handle->handle;
    p_async_op_desc->op_args.rename.tgt_name             = *p_new_name;
    p_async_op_desc->op_args.rename.context              = *p_context;
    p_async_op_desc->op_args.rename.src_dir_attributes   = *psrc_dir_attributes;
    p_async_op_desc->op_args.rename.tgt_dir_attributes   = *ptgt_dir_attributes;

    p_async_op_desc->op_res.rename.src_dir_attributes    = *psrc_dir_attributes;
    p_async_op_desc->op_res.rename.tgt_dir_attributes    = *ptgt_dir_attributes;

    /** \todo what is that?
     * p_async_op_desc->op_mobject       = ;
     */
    p_async_op_desc->fsal_op_context  = *p_context;
    p_async_op_desc->ptr_mfsl_context = (caddr_t) p_mfsl_context;

    p_async_op_desc->concerned_objects[0] = old_parentdir_handle;
    p_async_op_desc->concerned_objects[1] = new_parentdir_handle;
    p_async_op_desc->concerned_objects[2] = NULL;

    if(!old_parentdir_handle->p_last_op_desc ||
        timercmp(&old_parentdir_handle->last_op_time, &p_async_op_desc->op_time, < ))
    {
        old_parentdir_handle->p_last_op_desc = p_async_op_desc;
        old_parentdir_handle->last_op_time   = p_async_op_desc->op_time;
    }

    if(!new_parentdir_handle->p_last_op_desc ||
        timercmp(&new_parentdir_handle->last_op_time, &p_async_op_desc->op_time, < ))
    {
        new_parentdir_handle->p_last_op_desc = p_async_op_desc;
        new_parentdir_handle->last_op_time   = p_async_op_desc->op_time;
    }


    /* Post the asynchronous operation description
     *********************************************/
    fsal_status = MFSL_async_post(p_async_op_desc);

    if(FSAL_IS_ERROR(fsal_status))
        return fsal_status;

    
    /* return attributes and status
     ******************************/
    *psrc_dir_attributes = p_async_op_desc->op_guessed.rename.src_dir_attributes;
    *ptgt_dir_attributes = p_async_op_desc->op_guessed.rename.tgt_dir_attributes;

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
}                               /* MFSL_rename */
