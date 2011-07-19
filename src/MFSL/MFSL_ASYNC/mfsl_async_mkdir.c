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
 * \file    mfsl_async_mkdir.c
 * \author  $Author: delahoussaye $
 * \date    $Date: 2011/06/28 11:50:00 $
 * \version $Revision: 1.0 $
 * \brief   Asynchronous call to mkdir.
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

extern mfsl_filler_data_t * filler_data; /* Filler Data Array, from mfsl_async2_filler.c */

/******************************************************
 *              Common Filesystem calls.
 ******************************************************/

/**
 * MFSL_async_mkdir: finally perform a synchronous operation taken from an asynchronous operation
 *
 * finally perform a synchronous operation taken from an asynchronous operation
 *
 * @param p_operation_description   [IN/OUT] asynchronous operation description
 *
 * @return a FSAL status
 */
fsal_status_t MFSL_async_mkdir(mfsl_async_op_desc_t * p_operation_description)
{
    fsal_status_t fsal_status;

    /* Rename the precreated directory to its final destination */
    fsal_status = FSAL_rename(&p_operation_description->op_args.mkdir.old_parentdir_handle,    /* IN */
                              &p_operation_description->op_args.mkdir.old_dirname,             /* IN */
                              &p_operation_description->op_args.mkdir.new_parentdir_handle,    /* IN */
                              &p_operation_description->op_args.mkdir.new_dirname,             /* IN */
                              &p_operation_description->op_args.mkdir.context,                 /* IN */
                              NULL,                                                            /* src_dir_attributes, not used */
                              &p_operation_description->op_res.mkdir.new_parentdir_attributes  /* IN/OUT */
                             );
    if(FSAL_IS_ERROR(fsal_status))
    {
        LogCrit(COMPONENT_MFSL, "Could not rename precreated dir to its final destination.");
        /* Don't do anything else. MFSL_async_process_async_op will handle it. */
    }

    /* Set correct attributes */
    fsal_status = FSAL_setattrs(&p_operation_description->op_args.mkdir.new_dir_handle,        /* IN */
                                &p_operation_description->op_args.mkdir.context,               /* IN */
                                &p_operation_description->op_guessed.mkdir.new_dir_attributes, /* IN */
                                &p_operation_description->op_res.mkdir.new_dir_attributes      /* IN/OUT */
                               );
    if(FSAL_IS_ERROR(fsal_status))
    {
         LogCrit(COMPONENT_MFSL, "Could not setattrs new dir.");
        /* Don't do anything else. MFSL_async_process_async_op will handle it. */
    }

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
}

/**
 * MFSL_mkdir: creates a directory asynchronously.
 * Creates a directory asynchronously.
 *
 * @param parent_directory_handle [IN]     handle of the parent directory.
 * @param p_dir_name              [IN]     name of the directory to be created.
 * @param p_context               [IN]     fsal_context.
 * @param p_mfsl_context          [IN]     mfsl_context.
 * @param accessmode              [IN]     accessmode
 * @param object_handle           [IN/OUT] handle of the object.
 * @param object_attributes       [IN/OUT] attributes for the directory to be created.
 * @param p_parentdir_attributes  [IN/OUT] attributes for the parentdir.
 * @param pextra                           not used.
 *
 * @return a FSAL status
 */
fsal_status_t MFSL_mkdir(mfsl_object_t      * parent_directory_handle, /* IN */
                         fsal_name_t        * p_dirname,               /* IN */
                         fsal_op_context_t  * p_context,               /* IN */
                         mfsl_context_t     * p_mfsl_context,          /* IN */
                         fsal_accessmode_t    accessmode,              /* IN */
                         mfsl_object_t      * object_handle,           /* OUT */
                         fsal_attrib_list_t * object_attributes,       /* [ IN/OUT ] */
                         fsal_attrib_list_t * parent_attributes,       /* [ IN/OUT ] */
                         void               * pextra )
{
    fsal_status_t              fsal_status;            /* status we check asyncly */
    mfsl_async_op_desc_t     * p_async_op_desc = NULL; /* asynchronous operation */
    mfsl_precreated_object_t * precreated_object;      /* Precreated directory we get */
    int                        chosen_synclet;


    /* Sanity checks
     ***************/
    if(!p_context || !parent_attributes || !object_attributes)
        MFSL_return(ERR_FSAL_FAULT, 0);


    /* Asynchronous check
     ********************/
    fsal_status = FSAL_create_access(p_context, parent_attributes);

    if(FSAL_IS_ERROR(fsal_status))
    {
        LogCrit(COMPONENT_MFSL, "Access is forbidden. Status: (%u.%u).", fsal_status.major, fsal_status.minor);
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
    /* Get precreated directory */
    fsal_status = MFSL_async_get_precreated_object(chosen_synclet, FSAL_TYPE_DIR, &precreated_object);
    if(FSAL_IS_ERROR(fsal_status))
    {
        LogCrit(COMPONENT_MFSL, "Impossible to retrieve a precreated directory.");
        MFSL_return(fsal_status.major, fsal_status.minor);
    }

    /* handle */
    p_async_op_desc->op_guessed.mkdir.new_dir_handle = precreated_object->mobject.handle;
    p_async_op_desc->op_args.mkdir.new_dir_handle    = precreated_object->mobject.handle;
    p_async_op_desc->op_res.mkdir.new_dir_handle     = precreated_object->mobject.handle;

    /* attributes */
    memcpy((void *) &p_async_op_desc->op_res.mkdir.new_parentdir_attributes,
           (void *) parent_attributes,
           sizeof(fsal_attrib_list_t));

    memcpy((void *) &p_async_op_desc->op_guessed.mkdir.new_parentdir_attributes,
           (void *) parent_attributes,
           sizeof(fsal_attrib_list_t));

    memcpy((void *) &p_async_op_desc->op_guessed.mkdir.new_dir_attributes,
           (void *) &precreated_object->object_attributes,
           sizeof(fsal_attrib_list_t));

    p_async_op_desc->op_guessed.mkdir.new_dir_attributes.ctime.seconds  = time(NULL);
    p_async_op_desc->op_guessed.mkdir.new_dir_attributes.ctime.nseconds = 0;

    p_async_op_desc->op_guessed.mkdir.new_dir_attributes.mtime.seconds  =
            p_async_op_desc->op_guessed.mkdir.new_dir_attributes.ctime.seconds;
    p_async_op_desc->op_guessed.mkdir.new_dir_attributes.mtime.nseconds =
            p_async_op_desc->op_guessed.mkdir.new_dir_attributes.ctime.nseconds;

    p_async_op_desc->op_guessed.mkdir.new_dir_attributes.mode  = accessmode;
    p_async_op_desc->op_guessed.mkdir.new_dir_attributes.owner = FSAL_OP_CONTEXT_TO_UID(p_context);
    p_async_op_desc->op_guessed.mkdir.new_dir_attributes.group = FSAL_OP_CONTEXT_TO_GID(p_context);

    p_async_op_desc->op_guessed.mkdir.new_dir_attributes.type  = FSAL_TYPE_DIR;

    p_async_op_desc->op_guessed.mkdir.new_parentdir_attributes.filesize += 1;
    p_async_op_desc->op_guessed.mkdir.new_parentdir_attributes.numlinks += 1;


    /* Populate asynchronous operation description
     *********************************************/
    p_async_op_desc->op_type                             = MFSL_ASYNC_OP_MKDIR;
    p_async_op_desc->op_func                             = MFSL_async_mkdir;

    p_async_op_desc->op_args.mkdir.old_parentdir_handle  = filler_data[chosen_synclet].precreated_object_pool.dirs_pool_handle;
    p_async_op_desc->op_args.mkdir.old_dirname           = precreated_object->filename;
    p_async_op_desc->op_args.mkdir.new_parentdir_handle  = parent_directory_handle->handle;
    p_async_op_desc->op_args.mkdir.new_dirname           = *p_dirname;
    p_async_op_desc->op_args.mkdir.context               = *p_context;

    /** \todo what is that?
     * p_async_op_desc->op_mobject                             = object_handle;
     ****/
    p_async_op_desc->fsal_op_context                     = *p_context;
    p_async_op_desc->ptr_mfsl_context                    = (caddr_t) p_mfsl_context;

    p_async_op_desc->concerned_objects[0] = parent_directory_handle;
    p_async_op_desc->concerned_objects[1] = object_handle;
    p_async_op_desc->concerned_objects[2] = NULL;

    P(parent_directory_handle->lock);
    if(!parent_directory_handle->p_last_op_desc ||
        timercmp(&parent_directory_handle->last_op_time, &p_async_op_desc->op_time, < ))
    {
        parent_directory_handle->p_last_op_desc = p_async_op_desc;
        parent_directory_handle->last_op_time   = p_async_op_desc->op_time;
    }
    V(parent_directory_handle->lock);

    P(object_handle->lock);
    if(!object_handle->p_last_op_desc ||
        timercmp(&object_handle->last_op_time, &p_async_op_desc->op_time, < ))
    {
        object_handle->p_last_op_desc = p_async_op_desc;
        object_handle->last_op_time   = p_async_op_desc->op_time;
    }
    V(object_handle->lock);


    /* Free precreated_object
     ************************/
    P(filler_data[chosen_synclet].precreated_object_pool.mutex_pool_objects);
    ReleaseToPool(precreated_object, &filler_data[chosen_synclet].precreated_object_pool.pool_precreated_objects);
    V(filler_data[chosen_synclet].precreated_object_pool.mutex_pool_objects);

    /* Post the asynchronous operation description to the dispatcher
     ***************************************************************/
    fsal_status = MFSL_async_post(p_async_op_desc);

    if(FSAL_IS_ERROR(fsal_status))
    {
        LogCrit(COMPONENT_MFSL, "Impossible to post the operation.");
        return fsal_status;
    }

    /* Return attributes and status
     ******************************/
    *object_attributes = p_async_op_desc->op_guessed.mkdir.new_dir_attributes;
    *parent_attributes = p_async_op_desc->op_guessed.mkdir.new_parentdir_attributes;
    object_handle->handle = p_async_op_desc->op_guessed.mkdir.new_dir_handle;

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
} /* MFSL_mkdir */
