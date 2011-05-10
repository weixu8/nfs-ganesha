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
 * \brief   Initialisation of MFSL_ASYNC2.
 *
 *
 */

#include "config.h"

/* fsal_types contains constants and type definitions for FSAL */
#include "fsal_types.h"
#include "fsal.h"
#include "mfsl_types.h"
#include "mfsl.h"
#include "common_utils.h"


#ifndef _USE_SWIG

mfsl_parameter_t mfsl_param; /* MFSL parameters */

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
    /** @todo implement this. */
	fsal_status_t status;

	status.major = ERR_FSAL_NO_ERROR;
	status.minor = 0;

	return status;
}

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
    /** @todo implement this. */
	fsal_status_t status;

	status.major = ERR_FSAL_NO_ERROR;
	status.minor = 0;

	return status;
}

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
    /** @todo implement this. */
	fsal_status_t status;

	status.major = ERR_FSAL_NO_ERROR;
	status.minor = 0;

	return status;
}

#endif                          /* ! _USE_SWIG */
