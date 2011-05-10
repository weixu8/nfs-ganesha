/*
 *
 *
 * Copyright CEA/DAM/DIF  (2011)
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
 * \file    mfsl_async2_synclet.c
 * \author  $Author: leibovic $
 * \date    $Date: 2011/05/10 10:24:00 $
 * \version $Revision: 1.0 $
 * \brief   Synclets functions and thread.
 *
 *
 */

#include "config.h"

#include "fsal_types.h"
#include "fsal.h"
#include "mfsl_types.h"
#include "mfsl.h"
#include "common_utils.h"


#ifndef _USE_SWIG

extern mfsl_parameter_t mfsl_param;   /* MFSL parameters */

pthread_t           * synclet_thread; /* Synclet Thread Array */
mfsl_synclet_data_t * synclet_data;   /* Synclet Data Array */

/**
 *
 * MFSL_async_process_async_op: processes an asynchronous operation.
 *
 * Processes an asynchronous operation that was taken from a synclet's pending queue
 *
 * @param popdesc [IN]    the asynchronous operation descriptor
 *
 * @return the related fsal_status
 *
 */
fsal_status_t mfsl_async_process_async_op(mfsl_async_op_desc_t * pasyncopdesc)
{
    /** @todo: implement this */
    MFSL_return(ERR_FSAL_NO_ERROR, 0);
}

/**
 *
 * MFSL_async_choose_synclet: Choose the synclet that will receive an asynchornous op to manage
 *
 * @param (none)
 *
 * @return the index for the synclet to be used.
 *
 */
unsigned int MFSL_async_choose_synclet(void){
    /** @todo: implement this */
    return 0;
}

/**
 * mfsl_async_synclet_thread: thread used for asynchronous operation management.
 *
 * This thread is used for managing asynchronous operation management
 *
 * @param arg   the index for the thread
 *
 * @return Pointer to the result (but this function will mostly loop forever).
 *
 */
void * mfsl_async_synclet_thread(void * arg)
{
    /** @todo: implement this */
    return NULL;
}


/**
 *
 * MFSL_async_synclet_init: initializes dispatcher threads.
 *
 * Initializes synclet threads
 *
 * @param *arg   Not used
 *
 */
fsal_status_t MFSL_async_synclet_init(void * arg)
{
    /** @todo: implement this */
    MFSL_return(ERR_FSAL_NO_ERROR, 0);
}

#endif /* _USE_SWIG */
