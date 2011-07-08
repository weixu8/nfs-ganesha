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
 * \file    mfsl_async_read_conf.c
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


extern mfsl_parameter_t * mfsl_param;      /* MFSL parameters, from mfsl_async_init.c */
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
    SetNameFunction("MFSL_SetDefault_parameter");

    /* Sanitize */
    if(!out_parameter)
        MFSL_return(ERR_FSAL_FAULT, 0);

    out_parameter->nb_pre_async_op_desc = 50;
    out_parameter->nb_synclet           = 1;
    out_parameter->async_window_sec     = 1;
    out_parameter->async_window_usec    = 0;
    out_parameter->nb_before_gc         = 500;

    out_parameter->ADT_sleep_time = 60000;

    out_parameter->nb_pre_create_dirs  = 10;
    out_parameter->nb_pre_create_files = 10;
    strncpy(out_parameter->pre_create_obj_dir, "/tmp", MAXPATHLEN);

    out_parameter->lru_param.nb_entry_prealloc  = 100;
    out_parameter->lru_param.nb_call_gc_invalid = 30;
    out_parameter->lru_param.clean_entry        = mfsl_async_clean_pending_op;
    out_parameter->lru_param.entry_to_str       = mfsl_async_print_pending_op;

    out_parameter->AFT_low_watermark    = 3;
    out_parameter->AFT_nb_fill_critical = 5;

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
} /* MFSL_SetDefault_parameter */

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
fsal_status_t MFSL_load_parameter_from_conf(config_file_t      in_config,        /* IN */
                                            mfsl_parameter_t * p_out_parameter) /* IN/OUT */
{
    fsal_status_t   status;
    config_item_t   block;
    config_item_t   current_item;
    int             var_nb;
    int             var_cur;
    int             err;
    char          * key_name;
    char          * key_value;
    char          * LogFile    = NULL;
    int             DebugLevel = -1;

    SetNameFunction("MFSL_load_parameter_from_conf");

    /* Sanitize */
    if(!in_config || !p_out_parameter)
        MFSL_return(ERR_FSAL_FAULT, 0);

    /* Get configuration Block */
    if((block = config_FindItemByName(in_config, CONF_LABEL_MFSL_ASYNC)) == NULL)
    {
        LogMajor(COMPONENT_MFSL, "/!\\ Cannot read item \"%s\" from configuration file\n",
                CONF_LABEL_MFSL_ASYNC);

        MFSL_return(ERR_FSAL_NOENT, 0);
    }

    /* Total number of configuration items for this block */
    var_nb = config_GetNbItems(block);

    /* Let's check parameters */
    for(var_cur=0; var_cur < var_nb; var_cur++)
    {
        current_item = config_GetItemByIndex(block, var_cur);

        /* get key value */
        err = config_GetKeyValue(current_item, &key_name, &key_value);
        if(err > 0)
        {
            LogCrit(COMPONENT_MFSL, "Error reading key[%d] from section \"%s\" of configuration file.",
                    var_cur, CONF_LABEL_MFSL_ASYNC);
            MFSL_return(ERR_FSAL_SERVERFAULT, err);
        }

        /* Set keys */
        if(!strcasecmp(key_name, "Nb_Synclet"))
        {
            /* Number of synclet managed */
            /** \todo update this when scheduler is implemented */
            LogMajor(COMPONENT_MFSL,
                    "The asynchronous operation scheduler is not yet implemented. Only one synclet managed.");
            LogMajor(COMPONENT_MFSL, "Parameter Nb_Synclet = %s will be ignored", key_value);

            /* p_out_parameter->nb_synclet = atoi(key_value); */
            p_out_parameter->nb_synclet = 1;
        }
        else if(!strcasecmp(key_name, "Async_Window_sec"))
        {
            /* Size of the asynchronous window: seconds */
            p_out_parameter->async_window_sec = atoi(key_value);
        }
        else if(!strcasecmp(key_name, "Async_Window_usec"))
        {
            /* Size of the asynchronous window: micro seconds */
            p_out_parameter->async_window_usec = atoi(key_value);
        }
        else if(!strcasecmp(key_name, "Nb_Sync_Before_GC"))
        {
            /* Number of loops before garbage collect a LRU */
            p_out_parameter->nb_before_gc = atoi(key_value);
        }
        else if(!strcasecmp(key_name, "Nb_Pre_Async_Op_desc"))
        {
            /* Number of preallocated operation description */
            p_out_parameter->nb_pre_async_op_desc = atoi(key_value);
        }
        else if(!strcasecmp(key_name, "ADT_Sleep_Time"))
        {
            /* Asynchronous Dispatcher thread sleep time in microseconds */
            p_out_parameter->ADT_sleep_time = atoi(key_value);
        }
        else if(!strcasecmp(key_name, "PreCreatedObject_Directory"))
        {
            /* Directory on the FSAL where precreated objects will be stored */
            strncpy(p_out_parameter->pre_create_obj_dir, key_value, MAXPATHLEN);
        }
        else if(!strcasecmp(key_name, "Nb_PreCreated_Directories"))
        {
            /* Number of precreated directories we'll store */
            p_out_parameter->nb_pre_create_dirs = atoi(key_value);
        }
        else if(!strcasecmp(key_name, "Nb_PreCreated_Files"))
        {
            /* Number of precreated files we'll store */
            p_out_parameter->nb_pre_create_files = atoi(key_value);
        }
        else if(!strcasecmp(key_name, "LRU_Prealloc_PoolSize"))
        {
            /* Number of entries to prealloc in a pool */
            p_out_parameter->lru_param.nb_entry_prealloc = atoi(key_value);
        }
        else if(!strcasecmp(key_name, "LRU_Nb_Call_Gc_invalid"))
        {
            /* How many call before garbagging invalid entries */
            p_out_parameter->lru_param.nb_call_gc_invalid = atoi(key_value);
        }
        else if(!strcasecmp(key_name, "DebugLevel"))
        {
            /* Debug Level */
            DebugLevel = ReturnLevelAscii(key_value);

            if(DebugLevel == -1)
            {
                LogMajor(COMPONENT_MFSL, "Invalid debug level name: \"%s\"", key_value);
                MFSL_return(ERR_FSAL_INVAL, 0);
            }
        }
        else if(!strcasecmp(key_name, "LogFile"))
        {
            LogFile = key_value;
        }
        else
        {
            /* Unknown key */
            LogMajor(COMPONENT_MFSL,
                    "Unknown or unsettable key %s from section \"%s\" of configuration file.",
                    key_name, CONF_LABEL_MFSL_ASYNC);

            MFSL_return(ERR_FSAL_INVAL, 0);
        }
    }

    if(LogFile)
        SetComponentLogFile(COMPONENT_MFSL, LogFile);

    if(DebugLevel != -1)
        SetComponentLogLevel(COMPONENT_MFSL, DebugLevel);

    /* to sum up */
    LogDebug(COMPONENT_MFSL, "parameter->nb_synclet = %d",                   p_out_parameter->nb_synclet);
    LogDebug(COMPONENT_MFSL, "parameter->async_window_sec = %ld",            p_out_parameter->async_window_sec);
    LogDebug(COMPONENT_MFSL, "parameter->async_window_usec = %ld",           p_out_parameter->async_window_usec);
    LogDebug(COMPONENT_MFSL, "parameter->nb_before_gc = %d",                 p_out_parameter->nb_before_gc);
    LogDebug(COMPONENT_MFSL, "parameter->nb_pre_async_op_desc = %d",         p_out_parameter->nb_pre_async_op_desc);
    LogDebug(COMPONENT_MFSL, "parameter->ADT_sleep_time = %d",               p_out_parameter->ADT_sleep_time);
    LogDebug(COMPONENT_MFSL, "parameter->lru_param.nb_entry_prealloc = %d",  p_out_parameter->lru_param.nb_entry_prealloc);
    LogDebug(COMPONENT_MFSL, "parameter->lru_param.nb_call_gc_invalid = %d", p_out_parameter->lru_param.nb_call_gc_invalid);
    if(LogFile)
        LogDebug(COMPONENT_MFSL, "LogFile = %s", LogFile);

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
} /* MFSL_load_parameter_from_conf */
#endif                          /* ! _USE_SWIG */
