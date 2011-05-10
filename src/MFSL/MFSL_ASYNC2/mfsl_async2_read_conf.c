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
 * \file    mfsl_async2_read_conf.c
 * \author  $Author: leibovic $
 * \date    $Date: 2011/05/9 15:57:01 $
 * \version $Revision: 1.0 $
 * \brief   Initialisation of parameters.
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
/**
 *
 * mfsl_async_clean_pending_op: cleans an entry in an MFSL async op LRU,
 *
 * cleans an entry in an MFSL async op LRU.
 *
 * @param pentry [INOUT] entry to be cleaned.
 * @param addparam [IN] additional parameter used for cleaning.
 *
 * @return 0 if ok, other values mean an error.
 *
 */
int mfsl_async_clean_pending_op(LRU_entry_t * pentry, void *addparam)
{
    /** @todo: implement this */
    return 0;
}                               /* mfsl_async_clean_pending_request */

/**
 *
 * mfsl_async_print_pending_op: prints an entry related to a pending op in the LRU list.
 *
 * prints an entry related to a pending op in the LRU list.
 *
 * @param data [IN] data stored in a LRU entry to be printed.
 * @param str [OUT] string used to store the result.
 *
 * @return 0 if ok, other values mean an error.
 *
 */
int mfsl_async_print_pending_op(LRU_data_t data, char *str)
{
    /** @todo: implement this */
    return snprintf(str, LRU_DISPLAY_STRLEN, "not implemented for now");
}                               /* print_pending_request */


/**
 * Those routines set the default parameters
 * for FSAL init structure.
 * \return ERR_FSAL_NO_ERROR (no error) ,
 *         ERR_FSAL_FAULT (null pointer given as parameter),
 *         ERR_FSAL_SERVERFAULT (unexpected error)
 */
fsal_status_t MFSL_SetDefault_parameter(mfsl_parameter_t * out_parameter /* IN/OUT */)
{
    /* Sanitize */
    if(!out_parameter)
        MFSL_return(ERR_FSAL_FAULT, 0);

    out_parameter->nb_pre_async_op_desc = 50;
    out_parameter->nb_synclet = 1;
    out_parameter->async_window_sec = 1;
    out_parameter->async_window_usec = 0;
    out_parameter->nb_before_gc = 500;

    out_parameter->lru_param.nb_entry_prealloc = 100;
    out_parameter->lru_param.nb_call_gc_invalid = 30;
    out_parameter->lru_param.clean_entry = mfsl_async_clean_pending_op;
    out_parameter->lru_param.entry_to_str = mfsl_async_print_pending_op;

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
}                               /* MFSL_SetDefault_parameter */

/**
 * MFSL_load_parameter_from_conf,
 *
 * Those functions initialize the MFSL init parameter
 * structure from a configuration structure.
 *
 * \param in_config (input):
 *        Structure that represents the parsed configuration file.
 * \param out_parameter (ouput)
 *        MFSL initialization structure filled according
 *        to the configuration file given as parameter.
 *
 * \return ERR_FSAL_NO_ERROR (no error) ,
 *         ERR_FSAL_NOENT (missing a mandatory stanza in config file),
 *         ERR_FSAL_INVAL (invalid parameter),
 *         ERR_FSAL_SERVERFAULT (unexpected error)
 *         ERR_FSAL_FAULT (null pointer given as parameter),
 */
fsal_status_t MFSL_load_parameter_from_conf(config_file_t      in_config,     /* IN */
                                            mfsl_parameter_t * out_parameter) /* IN/OUT */
{
    /** @todo: implement this */
	fsal_status_t status;

	status.major = ERR_FSAL_NO_ERROR;
	status.minor = 0;

	return status;
}

#endif                          /* ! _USE_SWIG */
