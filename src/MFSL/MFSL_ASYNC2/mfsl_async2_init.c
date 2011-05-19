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
 * \file    mfsl_async2_init.c
 * \author  $Author: leibovic $
 * \date    $Date: 2011/05/9 15:57:01 $
 * \version $Revision: 1.0 $
 * \brief   Initialisation and termination of MFSL_ASYNC2.
 */

#include "config.h"

/* fsal_types contains constants and type definitions for FSAL */
#include "fsal_types.h"
#include "fsal.h"
#include "mfsl_types.h"
#include "mfsl.h"
#include "common_utils.h"
#include <errno.h>
#include <pthread.h>


#ifndef _USE_SWIG

mfsl_parameter_t * mfsl_param;          /* MFSL parameters */
unsigned int       end_of_mfsl = FALSE; /* Dispatcher and Synclets check this to stop */

/**
 * MFSL_Init: Inits the MFSL layer.
 *
 * Inits the MFSL layer.
 *
 * @param init_info      [IN] pointer to the MFSL parameters
 *
 * @return a FSAL status
 */
fsal_status_t MFSL_Init(mfsl_parameter_t * init_info    /* IN */)
{
	fsal_status_t status;;

    SetNameFunction("MFSL_Init");

    /* Sanitize */
    if(!init_info)
        MFSL_return(ERR_FSAL_FAULT, 0);

    LogEvent(COMPONENT_MFSL, "MFSL Initialisation.");

    /* Keep init_info in mind. */
    /* Had to make a copy. For a curious reason keeping only the pointer leads
     * to uninitialized memory in ganeshell when using cache_inode.
     * */
    mfsl_param = (mfsl_parameter_t *) Mem_Alloc(sizeof(mfsl_parameter_t));
    memcpy(mfsl_param, init_info, sizeof(mfsl_parameter_t));
    /*mfsl_param = init_info;*/

    /* Initializes and launch dispatcher */
    status = MFSL_async_dispatcher_init(NULL);
    if(FSAL_IS_ERROR(status))
        MFSL_return(status.major, 0);

    /* Initializes and launch synclets */
    status = MFSL_async_synclet_init(NULL);
    if(FSAL_IS_ERROR(status))
        MFSL_return(status.major, 0);

	MFSL_return(ERR_FSAL_NO_ERROR, 0);
} /* MFSL_Init */

/**
 *
 * MFSL_GetContext: Creates a MFSL context for a thread.
 *
 * Creates a MFSL context for a thread.
 *
 * @param pcontext      [INOUT] pointer to MFSL context to be used
 * @param pfsal_context [INOUT] pointer to FSAL context to be used
 *
 * @return a FSAL status
 *
 */
fsal_status_t MFSL_GetContext(mfsl_context_t    * pcontext,
                              fsal_op_context_t * pfsal_context)
{
	fsal_status_t status;

    SetNameFunction("MFSL_GetContext");

    /* Sanitize */
    if(!pcontext || !pfsal_context)
        MFSL_return(ERR_FSAL_FAULT, 0);

    LogDebug(COMPONENT_MFSL, "Getting context %p.", pcontext);

    if(pthread_mutex_init(&pcontext->lock, NULL) != 0)
        MFSL_return(ERR_FSAL_SERVERFAULT, errno);

    /* Creates the pool of asynchronous operations */
    MakePool(&pcontext->pool_async_op, mfsl_param->nb_pre_async_op_desc, mfsl_async_op_desc_t, NULL, NULL);

    /* Does Nothing for the moment.
     * Will reallocate files and dirs for this thread */
    P(pcontext->lock);
    status = MFSL_RefreshContext(pcontext, pfsal_context);
    V(pcontext->lock);

    if(FSAL_IS_ERROR(status))
        MFSL_return(status.major, 0);

	MFSL_return(ERR_FSAL_NO_ERROR, 0);
} /* MFSL_GetContext */

/**
 *
 * MFSL_RefreshContext: Refreshes a MFSL context for a thead.
 *
 * Refreshes a MFSL context for a thread.
 *
 * @param pcontext      [INOUT] pointer to MFSL context to be used
 * @param pfsal_context [INOUT] pointer to FSAL context to be used
 *
 * @return a FSAL status
 *
 */
fsal_status_t MFSL_RefreshContext(mfsl_context_t    * pcontext,
                                  fsal_op_context_t * pfsal_context)
{
    SetNameFunction("MFSL_RefreshContext");
    /** @todo implement this. */

    /* Sanitize */
    if(!pcontext || !pfsal_context)
        MFSL_return(ERR_FSAL_FAULT, 0);

    LogDebug(COMPONENT_MFSL, "Refreshing context %p.", pcontext);

	MFSL_return(ERR_FSAL_NO_ERROR, 0);
} /* MFSL_RefreshContext */

#endif                          /* ! _USE_SWIG */

/**
 *
 * MFSL_terminate: Terminates the MFSL Layer.
 *
 * Terminates the MFSL Layer. Is called before exiting.
 *
 * @param void
 *
 * @return a FSAL status
 *
 */
fsal_status_t MFSL_terminate(void)
{
    SetNameFunction("MFSL_terminate");

    LogEvent(COMPONENT_MFSL, "MFSL termination.");

    /* Tells Dispatcher and Synclets loops to stop looping. */
    end_of_mfsl = TRUE;

    /** @todo: join dispatcher and synclets */

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
} /* MFSL_terminate */
