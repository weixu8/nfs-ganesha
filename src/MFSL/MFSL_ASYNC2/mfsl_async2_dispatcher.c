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
 * \file    mfsl_async2_dispatcher.c
 * \author  $Author: leibovic $
 * \date    $Date: 2011/05/10 10:24:00 $
 * \version $Revision: 1.0 $
 * \brief   Dispatcher functions and thread.
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

extern mfsl_parameter_t mfsl_param;        /* MFSL parameters, from mfsl_async_init.c */

pthread_t         dispatcher_thread;       /* Dispatcher Thread. */
LRU_list_t      * dispatcher_lru;          /* The LRU list that owns posted operations. */
pthread_mutex_t   dispatcher_lru_mutex;    /* Mutex associated with previously declared list. */

extern mfsl_synclet_data_t * synclet_data; /* Synclet Data Array, from mfsl_async_synclet.c */

/**
 *
 * MFSL_async_post: posts an asynchronous operation to the pending operations list.
 *
 * Posts an asynchronous operation to the pending operations list.
 *
 * @param p_operation_description [IN]    the asynchronous operation descriptor
 *
 */
fsal_status_t MFSL_async_post(mfsl_async_op_desc_t * p_operation_description /* IN */)
{
    /** @todo: implement this */
    MFSL_return(ERR_FSAL_NO_ERROR, 0);
}

/**
 * mfsl_async_asynchronous_dispatcher_thread: this thread will assign asynchronous operation to the synclets.
 *
 *
 * @param Arg (unused)
 *
 * @return Pointer to the result (but this function will mostly loop forever).
 *
 */
void * mfsl_async_asynchronous_dispatcher_thread(void * arg)
{
    /** @todo: implement this */
    return NULL;
}

/**
 *
 * MFSL_async_dispatcher_init: initializes dispatcher thread.
 *
 * Initializes dispatcher thread
 *
 * @param *arg   Not used
 *
 */
fsal_status_t MFSL_async_dispatcher_init(void * arg)
{
    /** @todo: implement this */
    MFSL_return(ERR_FSAL_NO_ERROR, 0);
}

#endif /* _USE_SWIG */
