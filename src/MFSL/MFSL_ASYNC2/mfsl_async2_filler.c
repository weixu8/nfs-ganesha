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
 * \file    mfsl_async2_filler.c
 * \author  $Author: de la houssaye $
 * \date    $Date: 2011/06/21 10:00:00 $
 * \version $Revision: 1.0 $
 * \brief   Fillers functions and thread.
 *
 *
 */

#include "config.h"

#include "fsal_types.h"
#include "fsal.h"
#include "mfsl_types.h"
#include "mfsl.h"
#include "common_utils.h"

#include <sys/time.h>

#ifndef _USE_SWIG

extern mfsl_parameter_t * mfsl_param;  /* MFSL parameters, from mfsl_async_init.c */
extern unsigned int       end_of_mfsl; /* from mfsl_async_init.c */

pthread_t          * filler_thread;  /* Filler Thread Array */
mfsl_filler_data_t * filler_data;    /* Filler Data Array */

fsal_handle_t   dir_handle_precreate;
unsigned int    object_dir_inited = FALSE;

pthread_barrier_t filler_barrier;
pthread_once_t    filler_init_once = PTHREAD_ONCE_INIT;


/**
 *
 * MFSL_async_filler_init_precreated_object_directory: Gets the directory where precreated objects are stored. Creates if not exist.
 *
 * Gets the directory where precreated objects are stored. Creates if not exist.
 * This function ensures that it exists a pool tree as following:
 *      <precreated_dir>/
 *        |- <filler_index>/
 *             |- dirs/
 *             |   |- <sec>.<nsec>
 *             |- files/
 *                 |- <sec>.<nsec>
 *
 * \param nothing
 *
 * \return a fsal_status_t
 *
 */
fsal_status_t MFSL_async_filler_init_precreated_object_directory()
{
    fsal_status_t fsal_status;

    fsal_handle_t root_handle;
    fsal_name_t   object_directory_name;

    fsal_name_t   index_dir_name;
    char          index_dir_str[4];

    char          dir_str[10]  = "dirs";
    fsal_name_t   dir_name;
    char          file_str[10] = "files";
    fsal_name_t   file_name;

    int           i;
    int           rc;

    SetNameFunction("MFSL_async_filler_init_precreated_object_directory");

    LogDebug(COMPONENT_MFSL, "Begins initalization of precreated object directory.");

    if(FSAL_IS_ERROR(fsal_status = FSAL_str2name(mfsl_param->pre_create_obj_dir, MAXPATHLEN, &object_directory_name)))
    {
        LogMajor(COMPONENT_MFSL, "Impossible to convert path %s.", mfsl_param->pre_create_obj_dir);
        MFSL_return(fsal_status.major, fsal_status.minor);
    }

    /* lookup root */
    /* There is at least one filler, so we use first filler_data for root_fsal_context */
    fsal_status = FSAL_lookup(NULL, NULL, &filler_data[0].precreated_object_pool.root_fsal_context, &root_handle, NULL);
    if(FSAL_IS_ERROR(fsal_status))
    {
        LogCrit(COMPONENT_MFSL, "Impossible to lookup /. Status: (%u.%u). Exiting...",
                fsal_status.major, fsal_status.minor);

        exit(1);
    }

    /* lookup directory */
    fsal_status = FSAL_lookup(&root_handle,
                              &object_directory_name,
                              &filler_data[0].precreated_object_pool.root_fsal_context,
                              &dir_handle_precreate, NULL);

    if(FSAL_IS_ERROR(fsal_status))
    {
        if(fsal_status.major == ERR_FSAL_NOENT)
        {
            /* This directory doesn't exist. Let's create it */
            fsal_status = FSAL_mkdir(&root_handle, &object_directory_name, &filler_data[0].precreated_object_pool.root_fsal_context, 0700, &dir_handle_precreate, NULL);
            if(FSAL_IS_ERROR(fsal_status))
            {
                LogCrit(COMPONENT_MFSL, "Impossible to create %s. Status: (%u.%u). Exiting...",
                        mfsl_param->pre_create_obj_dir,
                        fsal_status.major, fsal_status.minor);

                exit(1);
            }
            LogDebug(COMPONENT_MFSL, "%s created.", mfsl_param->pre_create_obj_dir);
        }
        else
        {
            /* Unknown error */
            LogCrit(COMPONENT_MFSL, "Impossible to lookup %s. Status: (%u.%u). Exiting...",
                    mfsl_param->pre_create_obj_dir,
                    fsal_status.major, fsal_status.minor);

            exit(1);
        }
    }

    /* Precreated object directory now exists. Let's check next level. */
    for(i=0; i < mfsl_param->nb_synclet; i+=1)
    {
        /* this filler precreated pool */
        if((rc = sprintf(index_dir_str, "%d", i)) < 0)
        {
            LogCrit(COMPONENT_MFSL, "Impossible to copy string. Error: %d.", rc);
            exit(1);
        }

        if(FSAL_IS_ERROR(fsal_status = FSAL_str2name(index_dir_str, MAXPATHLEN, &index_dir_name)))
        {
            LogMajor(COMPONENT_MFSL, "Impossible to get name. Status: (%u.%u). Exiting...",
                     fsal_status.major, fsal_status.minor);
            exit(1);
        }

        fsal_status = FSAL_lookup(&dir_handle_precreate,
                                  &index_dir_name,
                                  &filler_data[i].precreated_object_pool.root_fsal_context,
                                  &filler_data[i].precreated_object_pool.filler_pool_handle,
                                  NULL);

        if(FSAL_IS_ERROR(fsal_status))
        {
            if(fsal_status.major == ERR_FSAL_NOENT)
            {
                /* This directory doesn't exist. Let's create it */
                fsal_status = FSAL_mkdir(&dir_handle_precreate,
                                         &index_dir_name,
                                         &filler_data[i].precreated_object_pool.root_fsal_context,
                                         0700,
                                         &filler_data[i].precreated_object_pool.filler_pool_handle,
                                         NULL);
                if(FSAL_IS_ERROR(fsal_status))
                {
                    LogCrit(COMPONENT_MFSL, "Impossible to create %s/%s. Status: (%u.%u). Exiting...",
                            mfsl_param->pre_create_obj_dir,
                            index_dir_str,
                            fsal_status.major, fsal_status.minor);
                    exit(1);
                }
                LogDebug(COMPONENT_MFSL, "%s/%s created.",
                         mfsl_param->pre_create_obj_dir,
                         index_dir_str);
            }
            else
            {
                LogMajor(COMPONENT_MFSL, "Impossible to lookup %s/%s. Status: (%u.%u). Exiting...",
                         mfsl_param->pre_create_obj_dir,
                         index_dir_str,
                         fsal_status.major, fsal_status.minor);
                exit(1);
            }
        }

        /* this filler precreated pool: directories
         ******************************************/
        if(FSAL_IS_ERROR(fsal_status = FSAL_str2name(dir_str, MAXPATHLEN, &dir_name)))
        {
            LogMajor(COMPONENT_MFSL, "Impossible to get name. Status: (%u.%u). Exiting...",
                     fsal_status.major, fsal_status.minor);
            exit(1);
        }

        fsal_status = FSAL_lookup(&filler_data[i].precreated_object_pool.filler_pool_handle,
                                  &dir_name,
                                  &filler_data[i].precreated_object_pool.root_fsal_context,
                                  &filler_data[i].precreated_object_pool.dirs_pool_handle,
                                  NULL);

        if(FSAL_IS_ERROR(fsal_status))
        {
            if(fsal_status.major == ERR_FSAL_NOENT)
            {
                /* This directory doesn't exist. Let's create it */
                fsal_status = FSAL_mkdir(&filler_data[i].precreated_object_pool.filler_pool_handle,
                                         &dir_name,
                                         &filler_data[i].precreated_object_pool.root_fsal_context,
                                         0700,
                                         &filler_data[i].precreated_object_pool.dirs_pool_handle,
                                         NULL);
                if(FSAL_IS_ERROR(fsal_status))
                {
                    LogCrit(COMPONENT_MFSL, "Impossible to create %s/%s/%s. Status: (%u.%u). Exiting...",
                            mfsl_param->pre_create_obj_dir,
                            index_dir_str,
                            dir_str,
                            fsal_status.major, fsal_status.minor);
                    exit(1);
                }
                LogDebug(COMPONENT_MFSL, "%s/%s/%s created.",
                         mfsl_param->pre_create_obj_dir,
                         index_dir_str,
                         dir_str);
            }
            else
            {
                LogMajor(COMPONENT_MFSL, "Impossible to lookup %s/%s/%s. Status: (%u.%u). Exiting...",
                         mfsl_param->pre_create_obj_dir,
                         index_dir_str,
                         dir_str,
                         fsal_status.major, fsal_status.minor);
                exit(1);
            }
        }

        /* this filler precreated pool: files
         ************************************/
        if(FSAL_IS_ERROR(fsal_status = FSAL_str2name(file_str, MAXPATHLEN, &file_name)))
        {
            LogMajor(COMPONENT_MFSL, "Impossible to get name. Status: (%u.%u). Exiting...",
                     fsal_status.major, fsal_status.minor);
            exit(1);
        }

        fsal_status = FSAL_lookup(&filler_data[i].precreated_object_pool.filler_pool_handle,
                                  &file_name,
                                  &filler_data[i].precreated_object_pool.root_fsal_context,
                                  &filler_data[i].precreated_object_pool.files_pool_handle,
                                  NULL);

        if(FSAL_IS_ERROR(fsal_status))
        {
            if(fsal_status.major == ERR_FSAL_NOENT)
            {
                /* This directory doesn't exist. Let's create it */
                fsal_status = FSAL_mkdir(&filler_data[i].precreated_object_pool.filler_pool_handle,
                                         &file_name,
                                         &filler_data[i].precreated_object_pool.root_fsal_context,
                                         0700,
                                         &filler_data[i].precreated_object_pool.files_pool_handle,
                                         NULL);
                if(FSAL_IS_ERROR(fsal_status))
                {
                    LogCrit(COMPONENT_MFSL, "Impossible to create %s/%s/%s. Status: (%u.%u). Exiting...",
                            mfsl_param->pre_create_obj_dir,
                            index_dir_str,
                            file_str,
                            fsal_status.major, fsal_status.minor);
                    exit(1);
                }
                LogDebug(COMPONENT_MFSL, "%s/%s/%s created.",
                         mfsl_param->pre_create_obj_dir,
                         index_dir_str,
                         file_str);
            }
            else
            {
                LogMajor(COMPONENT_MFSL, "Impossible to lookup %s/%s/%s. Status: (%u.%u). Exiting...",
                         mfsl_param->pre_create_obj_dir,
                         index_dir_str,
                         file_str,
                         fsal_status.major, fsal_status.minor);
                exit(1);
            }
        }

    } /* for */

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
} /* MFSL_async_filler_init_precreated_object_directory */


/**
 *
 * MFSL_async_filler_dispatch_objects: Get previsouly created objects and dispatches them in fillers.
 *
 * Get previsouly created objects and dispatches them in fillers.
 *
 * \param nothing
 *
 * \return a fsal_status_t
 *
 */
#define NB_DIRENT_CLEAN 100
fsal_status_t MFSL_async_filler_dispatch_objects()
{
    fsal_status_t  fsal_status;
    fsal_boolean_t eod = FALSE;
    int            i = 0;

    fsal_dir_t     dir_dir_descriptor;
    fsal_cookie_t  dir_fsal_cookie_beginning;
    fsal_dirent_t  dir_dirent[NB_DIRENT_CLEAN];
    fsal_cookie_t  dir_end_cookie;
    fsal_count_t   dir_nb_count;
    fsal_count_t   dir_nb_entries;

    fsal_dir_t     file_dir_descriptor;
    fsal_cookie_t  file_fsal_cookie_beginning;
    fsal_dirent_t  file_dirent[NB_DIRENT_CLEAN];
    fsal_cookie_t  file_end_cookie;
    fsal_count_t   file_nb_count;
    fsal_count_t   file_nb_entries;

    mfsl_precreated_object_t * object_entry;
    LRU_entry_t              * lru_object_entry;
    LRU_status_t               lru_status;

    SetNameFunction("MFSL_async_filler_dispatch_objects");

    /* Loop on fillers index */
    for(i=0; i < mfsl_param->nb_synclet; i+=1)
    {
        /* precreated directories
         ************************/
        while(eod == FALSE)
        {
            if(FSAL_IS_ERROR(fsal_status = FSAL_opendir(&filler_data[i].precreated_object_pool.dirs_pool_handle,
                                                        &filler_data[i].precreated_object_pool.root_fsal_context,
                                                        &dir_dir_descriptor, NULL)))
            {
                LogMajor(COMPONENT_MFSL, "Impossible to open precreated_directories directory on filler #%d.", i);
                MFSL_return(fsal_status.major, fsal_status.minor);
            }

            FSAL_SET_COOKIE_BEGINNING(dir_fsal_cookie_beginning);
            fsal_status = FSAL_readdir(&dir_dir_descriptor,
                                       dir_fsal_cookie_beginning,
                                       FSAL_ATTRS_MANDATORY,
                                       NB_DIRENT_CLEAN * sizeof(fsal_dirent_t),
                                       dir_dirent, &dir_end_cookie, &dir_nb_entries, &eod);

            if(FSAL_IS_ERROR(fsal_status))
            {
                LogMajor(COMPONENT_MFSL,
                         "Impossible to readdir precreated_directories directory on filler #%d.", i);

                MFSL_return(fsal_status.major, fsal_status.minor);
            }

            fsal_status = FSAL_closedir(&dir_dir_descriptor);
            if(FSAL_IS_ERROR(fsal_status))
            {
                LogMajor(COMPONENT_MFSL, "Impossible to close precreated_directories directory on filler #%d.", i);

                MFSL_return(fsal_status.major, fsal_status.minor);
            }

            /* retrieve all found entries in filler #i */
            for(dir_nb_count=0; dir_nb_count < dir_nb_entries; dir_nb_count++)
            {
                /* POOL */
                P(filler_data[i].precreated_object_pool.mutex_pool_objects);
                GetFromPool(object_entry, &filler_data[i].precreated_object_pool.pool_precreated_objects, mfsl_precreated_object_t);
                V(filler_data[i].precreated_object_pool.mutex_pool_objects);
            
                if(object_entry == NULL)
                {
                    LogCrit(COMPONENT_MFSL, "Could not get a precreated object from pool_precreated_objects.");
                    MFSL_return(ERR_FSAL_SERVERFAULT, 0);
                }

                object_entry->mobject.handle    = dir_dirent[dir_nb_count].handle;
                object_entry->filename          = dir_dirent[dir_nb_count].name;
                object_entry->object_attributes = dir_dirent[dir_nb_count].attributes;

                /* LRU */
                P(filler_data[i].precreated_object_pool.mutex_dirs_lru);
                
                lru_object_entry = LRU_new_entry(filler_data[i].precreated_object_pool.dirs_lru, &lru_status);
                
                if(!lru_object_entry)
                {
                    LogCrit(COMPONENT_MFSL, "Impossible to get a new entry.");
                    V(filler_data[i].precreated_object_pool.mutex_dirs_lru);
                    MFSL_return(ERR_FSAL_SERVERFAULT, (int) lru_status);
                }
                
                lru_object_entry->buffdata.pdata = (caddr_t) object_entry;
                lru_object_entry->buffdata.len   = sizeof(mfsl_precreated_object_t);
                lru_object_entry->valid_state    = LRU_ENTRY_VALID;
                
                V(filler_data[i].precreated_object_pool.mutex_dirs_lru);

                LogDebug(COMPONENT_MFSL, "dir %s dispatched to filler #%d.", object_entry->filename.name, i);
            }/* for */

        } /* while */

        eod = FALSE;

        /* precreated files
         ******************/
        while(eod == FALSE)
        {
            if(FSAL_IS_ERROR(fsal_status = FSAL_opendir(&filler_data[i].precreated_object_pool.files_pool_handle,
                                                        &filler_data[i].precreated_object_pool.root_fsal_context,
                                                        &file_dir_descriptor, NULL)))
            {
                LogMajor(COMPONENT_MFSL, "Impossible to open precreated_files directory on filler #%d.", i);
                MFSL_return(fsal_status.major, fsal_status.minor);
            }

            FSAL_SET_COOKIE_BEGINNING(file_fsal_cookie_beginning);
            fsal_status = FSAL_readdir(&file_dir_descriptor,
                                       file_fsal_cookie_beginning,
                                       FSAL_ATTRS_MANDATORY,
                                       NB_DIRENT_CLEAN * sizeof(fsal_dirent_t),
                                       file_dirent, &file_end_cookie, &file_nb_entries, &eod);

            if(FSAL_IS_ERROR(fsal_status))
            {
                LogMajor(COMPONENT_MFSL,
                         "Impossible to readdir precreated_files directory on filler #%d.", i);

                MFSL_return(fsal_status.major, fsal_status.minor);
            }

            fsal_status = FSAL_closedir(&file_dir_descriptor);
            if(FSAL_IS_ERROR(fsal_status))
            {
                LogMajor(COMPONENT_MFSL, "Impossible to close precreated_files directory on filler #%d.", i);

                MFSL_return(fsal_status.major, fsal_status.minor);
            }

            /* retrieve all found entries in filler #i */
            for(file_nb_count=0; file_nb_count < file_nb_entries; file_nb_count++)
            {
                /* POOL */
                P(filler_data[i].precreated_object_pool.mutex_pool_objects);
                GetFromPool(object_entry, &filler_data[i].precreated_object_pool.pool_precreated_objects, mfsl_precreated_object_t);
                V(filler_data[i].precreated_object_pool.mutex_pool_objects);
            
                if(object_entry == NULL)
                {
                    LogCrit(COMPONENT_MFSL, "Could not get a precreated object from pool_precreated_objects.");
                    MFSL_return(ERR_FSAL_SERVERFAULT, 0);
                }

                object_entry->mobject.handle    = file_dirent[file_nb_count].handle;
                object_entry->filename          = file_dirent[file_nb_count].name;
                object_entry->object_attributes = file_dirent[file_nb_count].attributes;

                /* LRU */
                P(filler_data[i].precreated_object_pool.mutex_files_lru);
                
                lru_object_entry = LRU_new_entry(filler_data[i].precreated_object_pool.files_lru, &lru_status);
                
                if(!lru_object_entry)
                {
                    LogCrit(COMPONENT_MFSL, "Impossible to get a new entry.");
                    V(filler_data[i].precreated_object_pool.mutex_files_lru);
                    MFSL_return(ERR_FSAL_SERVERFAULT, (int) lru_status);
                }
                
                lru_object_entry->buffdata.pdata = (caddr_t) object_entry;
                lru_object_entry->buffdata.len   = sizeof(mfsl_precreated_object_t);
                lru_object_entry->valid_state    = LRU_ENTRY_VALID;
                
                V(filler_data[i].precreated_object_pool.mutex_files_lru);

                LogDebug(COMPONENT_MFSL, "file %s dispatched to filler #%d.", object_entry->filename.name, i);
            } /* for */

        } /* while */

    } /* for */

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
} /* MFSL_async_filler_dispatch_objects */


/**
 * MFSL_async_filler_fill_directories: this function fills a pool of directories.
 *
 * This function fills a pool of directories
 *
 * \param index  [IN] The index of the pool
 * \param number [IN] The number of directories to create
 * \param object [IN/OUT] if not NULL, don't fill lru. return a pointer to the objects.
 *
 * \return 0 if ok, another value instead
 */
int MFSL_async_filler_fill_directories(unsigned int index, unsigned int number, mfsl_precreated_object_t ** pp_object)
{
    fsal_status_t              fsal_status;
    struct timeval             current_time;
    char                       filename_str[MAXNAMLEN];
    mfsl_precreated_object_t * object_entry;
    LRU_entry_t              * lru_object_entry;
    LRU_status_t               lru_status;

    unsigned int remaining_in_lru = 0;
    unsigned int missing_in_lru   = 0;
    unsigned int remaining_todo   = 0;

    int i = 0;

    SetNameFunction("MFSL_async_filler_fill_directories");

    /* Do we add to LRU or do we give objects directly to caller?
     ************************************************************/
    if(!pp_object)
    {
        /* Take the ressource */
        P(filler_data[index].precreated_object_pool.mutex_dirs_lru);

        /* how many objects do we add? */
        remaining_in_lru = filler_data[index].precreated_object_pool.dirs_lru->nb_entry - filler_data[index].precreated_object_pool.dirs_lru->nb_invalid;

        missing_in_lru = mfsl_param->nb_pre_create_dirs - remaining_in_lru;

        if(missing_in_lru < number)
        {
            remaining_todo = missing_in_lru;
        }
        else
        {
            remaining_todo = number;
        }
    }
    else
    {
        /* In this case we don't care about the state of LRU.
         * We're asked to give 'number' directories to caller.
         *****************************************************/
        remaining_todo = number;
    }

    LogDebug(COMPONENT_MFSL, "Beginning filling of directories. Filler #%d, %d directories to add.", index, remaining_todo);


    /* Begin to fill
     ***************/
    while(remaining_todo > 0)
    {
        /* POOL */
        P(filler_data[index].precreated_object_pool.mutex_pool_objects);
        GetFromPool(object_entry, &filler_data[index].precreated_object_pool.pool_precreated_objects, mfsl_precreated_object_t);
        V(filler_data[index].precreated_object_pool.mutex_pool_objects);

        if(object_entry == NULL)
        {
            LogCrit(COMPONENT_MFSL, "Could not get a precreated object from pool_precreated_objects.");
            exit(1);
        }

        /* Compute new file name */
        if(gettimeofday(&current_time, NULL) != 0)
        {
            /* Could'not get time of day... */
            LogCrit(COMPONENT_MFSL, "Cannot get time of day...");
            exit(1);
        }

        sprintf(filename_str, "%ld.%ld", current_time.tv_sec, current_time.tv_usec);
        if(FSAL_IS_ERROR(fsal_status = FSAL_str2name(filename_str, MAXPATHLEN, &object_entry->filename)))
        {
            LogMajor(COMPONENT_MFSL, "Impossible to get name. Status: (%u.%u). Exiting...",
                     fsal_status.major, fsal_status.minor);
            exit(1);
        }

        /* Dir creation */
        fsal_status = FSAL_mkdir(&filler_data[index].precreated_object_pool.dirs_pool_handle,
                                 &object_entry->filename,
                                 &filler_data[index].precreated_object_pool.root_fsal_context,
                                 0700,
                                 &object_entry->mobject.handle,
                                 &object_entry->object_attributes);

        if(FSAL_IS_ERROR(fsal_status))
        {
            LogCrit(COMPONENT_MFSL, "Impossible to precreate a new directory. Status: (%u.%u).",
                    fsal_status.major, fsal_status.minor);
            return 1;
        }
        LogDebug(COMPONENT_MFSL, "%s/%d/dirs/%s created.",
                 mfsl_param->pre_create_obj_dir,
                 index, filename_str);

        if(pp_object == NULL)
        {
            /* LRU */
            lru_object_entry = LRU_new_entry(filler_data[index].precreated_object_pool.dirs_lru, &lru_status);

            if(!lru_object_entry)
            {
                LogCrit(COMPONENT_MFSL, "Impossible to get a new entry. LRU_status: %d", (int) lru_status);
                exit(1);
            }
                
            lru_object_entry->buffdata.pdata = (caddr_t) object_entry;
            lru_object_entry->buffdata.len   = sizeof(mfsl_precreated_object_t);
            lru_object_entry->valid_state    = LRU_ENTRY_VALID;
        }
        else
        {
            /* Give them to caller */
            pp_object[i] = object_entry;
            i++;
        }

        remaining_todo -= 1;
        LogDebug(COMPONENT_MFSL, "%d directories to create.", remaining_todo);
    } /* while */


    /* Don't forget to free mutex
     ****************************/
    if(pp_object == NULL)
    {
        V(filler_data[index].precreated_object_pool.mutex_dirs_lru);        
    }

    return 0;
} /* MFSL_async_filler_fill_directories */


/**
 * MFSL_async_fill_files: this function fills a pool of file.
 *
 * This function fills a pool of file
 *
 * \param index  [IN]     The index of the pool
 * \param number [IN]     The number of file to create
 * \param object [IN/OUT] if not NULL, don't fill lru. return a pointer to the objects.
 *
 * \return 0 if ok, another value instead
 */
int MFSL_async_filler_fill_files(unsigned int index, int number, mfsl_precreated_object_t ** object)
{
    fsal_status_t              fsal_status;
    struct timeval             current_time;
    char                       filename_str[MAXNAMLEN];
    mfsl_precreated_object_t * object_entry;
    LRU_entry_t              * lru_object_entry;
    LRU_status_t               lru_status;

    unsigned int remaining_in_lru = 0;
    unsigned int missing_in_lru   = 0;
    unsigned int remaining_todo   = 0;

    int i = 0;

    SetNameFunction("MFSL_async_filler_fill_files");

    /* Do we add to LRU or do we give objects directly to caller?
     ************************************************************/
    if(!object)
    {
        /* Take the ressource */
        P(filler_data[index].precreated_object_pool.mutex_files_lru);

        /* how many objects do we add? */
        remaining_in_lru = filler_data[index].precreated_object_pool.files_lru->nb_entry - filler_data[index].precreated_object_pool.files_lru->nb_invalid;

        missing_in_lru = mfsl_param->nb_pre_create_files - remaining_in_lru;

        if(missing_in_lru < number)
        {
            remaining_todo = missing_in_lru;
        }
        else
        {
            remaining_todo = number;
        }
    }
    else
    {
        /* In this case we don't care about the state of LRU.
         * We're asked to give 'number' files to caller.
         ****************************************************/
        remaining_todo = number;
    }

    LogDebug(COMPONENT_MFSL, "Beginning filling of files. Filler #%d, %d files to add.", index, remaining_todo);


    /* Begin to fill
     ***************/
    while(remaining_todo > 0)
    {
        /* POOL */
        P(filler_data[index].precreated_object_pool.mutex_pool_objects);
        GetFromPool(object_entry, &filler_data[index].precreated_object_pool.pool_precreated_objects, mfsl_precreated_object_t);
        V(filler_data[index].precreated_object_pool.mutex_pool_objects);

        if(object_entry == NULL)
        {
            LogCrit(COMPONENT_MFSL, "Could not get a precreated object from pool_precreated_objects.");
            exit(1);
        }

        /* Compute new file name */
        if(gettimeofday(&current_time, NULL) != 0)
        {
            /* Could'not get time of day... */
            LogCrit(COMPONENT_MFSL, "Cannot get time of day...");
            exit(1);
        }

        sprintf(filename_str, "%ld.%ld", current_time.tv_sec, current_time.tv_usec);
        if(FSAL_IS_ERROR(fsal_status = FSAL_str2name(filename_str, MAXPATHLEN, &object_entry->filename)))
        {
            LogMajor(COMPONENT_MFSL, "Impossible to get name. Status: (%u.%u). Exiting...",
                     fsal_status.major, fsal_status.minor);
            exit(1);
        }

        /* Dir creation */
        fsal_status = FSAL_create(&filler_data[index].precreated_object_pool.files_pool_handle,
                                 &object_entry->filename,
                                 &filler_data[index].precreated_object_pool.root_fsal_context,
                                 0700,
                                 &object_entry->mobject.handle,
                                 &object_entry->object_attributes);

        if(FSAL_IS_ERROR(fsal_status))
        {
            LogCrit(COMPONENT_MFSL, "Impossible to precreate a new file. Status: (%u.%u).",
                    fsal_status.major, fsal_status.minor);
            return 1;
        }
        LogDebug(COMPONENT_MFSL, "%s/%d/files/%s created.",
                 mfsl_param->pre_create_obj_dir,
                 index, filename_str);

        if(!object)
        {
            /* LRU */
            lru_object_entry = LRU_new_entry(filler_data[index].precreated_object_pool.files_lru, &lru_status);

            if(!lru_object_entry)
            {
                LogCrit(COMPONENT_MFSL, "Impossible to get a new entry. LRU_status: %d", (int) lru_status);
                exit(1);
            }
                
            lru_object_entry->buffdata.pdata = (caddr_t) object_entry;
            lru_object_entry->buffdata.len   = sizeof(mfsl_precreated_object_t);
            lru_object_entry->valid_state    = LRU_ENTRY_VALID;
        }
        else
        {
            /* Give them to caller */
            object[i] = object_entry;
            i++;
        }

        remaining_todo -= 1;
        LogDebug(COMPONENT_MFSL, "%d files to create.", remaining_todo);
    } /* while */


    /* Don't forget to free mutex
     ****************************/
    if(!object)
    {
        V(filler_data[index].precreated_object_pool.mutex_files_lru);        
    }

    return 0;
} /* MFSL_async_filler_fill_files */

/**
 * MFSL_async_filler_init_objects: initializes precreated objects pools.
 *
 * Initializes precreated objects pools. this function is only called once.
 *
 * \param nothing
 *
 * \return fsal_status_t
 */
fsal_status_t MFSL_async_filler_init_objects()
{
    fsal_status_t fsal_status;
    int i;
    int rc;

    /* Init precreated object directory
     **********************************/
    if(!object_dir_inited)
    {
        fsal_status = MFSL_async_filler_init_precreated_object_directory();
        if(FSAL_IS_ERROR(fsal_status))
            MFSL_return(fsal_status.major, 0);

        object_dir_inited = TRUE;
    }

    /* Dispatch previously created objects
     *************************************/
    fsal_status = MFSL_async_filler_dispatch_objects();    
    if(FSAL_IS_ERROR(fsal_status))
        MFSL_return(fsal_status.major, 0);

    /* Fill pools
     ************/
    for(i=0; i < mfsl_param->nb_synclet; i++)
    {
        LogDebug(COMPONENT_MFSL, "Filling filler pool number %d.", i);

        if((rc = MFSL_async_filler_fill_directories(i, mfsl_param->nb_pre_create_dirs, NULL)) != 0)
            MFSL_return(ERR_FSAL_SERVERFAULT, rc);

        if((rc = MFSL_async_filler_fill_files(i, mfsl_param->nb_pre_create_files, NULL)) != 0)
            MFSL_return(ERR_FSAL_SERVERFAULT, rc);
    }

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
} /* MFSL_async_filler_init_objects */


/**
 * MFSL_async_get_precreated_object: get a precreated object from a pool.
 *
 * get a precreated object from a pool.
 */
fsal_status_t MFSL_async_get_precreated_object(unsigned int filler_index, fsal_nodetype_t type, mfsl_precreated_object_t ** object)
{
    fsal_status_t   fsal_status;
    LRU_entry_t   * current_lru_entry;
    int             rc;

    switch(type)
    {
        case FSAL_TYPE_FILE:
            LogDebug(COMPONENT_MFSL, "Getting a precreated file.");
            /* Get first available entry */
            P(filler_data[filler_index].precreated_object_pool.mutex_files_lru);
            for(current_lru_entry=filler_data[filler_index].precreated_object_pool.files_lru->LRU;
                    (current_lru_entry->valid_state != LRU_ENTRY_VALID) && (current_lru_entry != NULL);
                    current_lru_entry=current_lru_entry->next);

            if(current_lru_entry->valid_state != LRU_ENTRY_VALID)
            {
                /* fill with one and alert administrators */
                LogWarn(COMPONENT_MFSL, "We ran out of precreated files. Creating one on the fly.");
                if(MFSL_async_filler_fill_files(filler_index, 1, object) != 0)
                {
                    V(filler_data[filler_index].precreated_object_pool.mutex_files_lru);
                    LogCrit(COMPONENT_MFSL, "Impossible to fill files.");
                    MFSL_return(ERR_FSAL_NOENT, 0);
                }

                /* Warn filler to wake up */
                P(filler_data[filler_index].mutex_watermark_condvar);
                if((rc = pthread_cond_signal(&filler_data[filler_index].watermark_condvar)) != 0)
                {
                    LogCrit(COMPONENT_MFSL, "Impossible to pthread_cond_signal to filler: %d.", rc);
                }
                V(filler_data[filler_index].mutex_watermark_condvar);
            }
            else
            {
                /* Entry found */
                *object = (mfsl_precreated_object_t *) current_lru_entry->buffdata.pdata;

                if(LRU_invalidate(filler_data[filler_index].precreated_object_pool.files_lru, current_lru_entry) != LRU_LIST_SUCCESS)
                    LogCrit(COMPONENT_MFSL, "Impossible to invalidate current_lru_entry in dispatcher_lru.");

                if((filler_data[filler_index].precreated_object_pool.files_lru->nb_entry
                            - filler_data[filler_index].precreated_object_pool.files_lru->nb_invalid)
                        < mfsl_param->AFT_low_watermark)
                {
                    /* Wake up filler */
                    P(filler_data[filler_index].mutex_watermark_condvar);
                    if((rc = pthread_cond_signal(&filler_data[filler_index].watermark_condvar)) != 0)
                    {
                        LogCrit(COMPONENT_MFSL, "Impossible to pthread_cond_signal to filler: %d.", rc);
                    }
                    V(filler_data[filler_index].mutex_watermark_condvar);
                }
            }
            V(filler_data[filler_index].precreated_object_pool.mutex_files_lru);
            break;

        case FSAL_TYPE_DIR:
            LogDebug(COMPONENT_MFSL, "Getting a precreated directory.");
            /* Get first available entry */
            P(filler_data[filler_index].precreated_object_pool.mutex_dirs_lru);
            LogDebug(COMPONENT_MFSL, "%d precreated directories remaining.",
                     (filler_data[filler_index].precreated_object_pool.dirs_lru->nb_entry
                      - filler_data[filler_index].precreated_object_pool.dirs_lru->nb_invalid));
            for(current_lru_entry=filler_data[filler_index].precreated_object_pool.dirs_lru->LRU;
                    (current_lru_entry->valid_state != LRU_ENTRY_VALID) && (current_lru_entry != NULL);
                    current_lru_entry=current_lru_entry->next);

            if(current_lru_entry->valid_state != LRU_ENTRY_VALID)
            {
                /* fill with one and alert administrators */
                LogWarn(COMPONENT_MFSL, "We ran out of precreated directories. Creating one on the fly.");
                if(MFSL_async_filler_fill_directories(filler_index, 1, object) != 0)
                {
                    V(filler_data[filler_index].precreated_object_pool.mutex_dirs_lru);
                    LogCrit(COMPONENT_MFSL, "Impossible to fill directories.");
                    MFSL_return(ERR_FSAL_NOENT, 0);
                }

                /* Warn filler to wake up */
                P(filler_data[filler_index].mutex_watermark_condvar);
                if((rc = pthread_cond_signal(&filler_data[filler_index].watermark_condvar)) != 0)
                {
                    LogCrit(COMPONENT_MFSL, "Impossible to pthread_cond_signal to filler: %d.", rc);
                }
                V(filler_data[filler_index].mutex_watermark_condvar);
            }
            else
            {
                /* Entry found */
                *object = (mfsl_precreated_object_t *) current_lru_entry->buffdata.pdata;

                if(LRU_invalidate(filler_data[filler_index].precreated_object_pool.dirs_lru, current_lru_entry) != LRU_LIST_SUCCESS)
                    LogCrit(COMPONENT_MFSL, "Impossible to invalidate current_lru_entry in dispatcher_lru.");

                if((filler_data[filler_index].precreated_object_pool.dirs_lru->nb_entry
                            - filler_data[filler_index].precreated_object_pool.dirs_lru->nb_invalid)
                        < mfsl_param->AFT_low_watermark)
                {
                    LogDebug(COMPONENT_MFSL, "Filler %d, low matermark reached. Waking up filler.", filler_index);
                    /* Wake filler up */
                    P(filler_data[filler_index].mutex_watermark_condvar);
                    if((rc = pthread_cond_signal(&filler_data[filler_index].watermark_condvar)) != 0)
                    {
                        LogCrit(COMPONENT_MFSL, "Impossible to pthread_cond_signal to filler: %d.", rc);
                    }
                    V(filler_data[filler_index].mutex_watermark_condvar);
                }
            }
            V(filler_data[filler_index].precreated_object_pool.mutex_dirs_lru);
            break;
        default:
            MFSL_return(ERR_FSAL_INVAL, 0);
    }

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
} /* MFSL_async_get_precreated_object */


/**
 * mfsl_async_filler_thread: thread used for objects creation management.
 *
 * This thread is used for managing objects creation
 *
 * @param arg   the index for the thread
 *
 * @return Pointer to the result (but this function will mostly loop forever).
 *
 */
void * mfsl_async_filler_thread(void * arg)
{
    mfsl_filler_data_t   * my_filler_data = (mfsl_filler_data_t *) arg;
    fsal_status_t          fsal_status;
    char                   namestr[64]; /* name of the function */
    int                    rc=0;        /* Used by BuddyInit */
    int                    i;
    fsal_export_context_t  fsal_export_context;
    LRU_status_t           lru_status;

    int remaining_dirs;
    int remaining_files;

    sprintf(namestr, "mfsl_async_filler_thread #%d", my_filler_data->index);
    SetNameFunction(namestr);

    /*   Initialisation   *
     **********************/
#ifndef _NO_BUDDY_SYSTEM
    /* Initialize Memory */
    if((rc = BuddyInit(NULL)) != BUDDY_SUCCESS)
    {
        /* Failed init */
        LogCrit(COMPONENT_MFSL, "Memory manager could not be initialized, exiting...");
        exit(1);
    }
    LogEvent(COMPONENT_MFSL, "Memory manager successfully initialized.");
#endif

    LogEvent(COMPONENT_MFSL, "Filler initialisation.");

    /* Data initialisation
     *********************/
    LogDebug(COMPONENT_MFSL, "Initialisation of filler number %d.", my_filler_data->index);

    if(pthread_cond_init(&filler_data[my_filler_data->index].watermark_condvar, NULL)        != 0)
    {
        LogCrit(COMPONENT_MFSL, "Impossible to initialize watermark_condvar for filler %d.",
                my_filler_data->index);
        exit(1);
    }

    if(pthread_mutex_init(&filler_data[my_filler_data->index].mutex_watermark_condvar, NULL) != 0)
    {
        LogCrit(COMPONENT_MFSL, "Impossible to initialize mutex_watermark_condvar for filler %d.",
                my_filler_data->index);
        exit(1);
    }

    if(pthread_mutex_init(&filler_data[my_filler_data->index].precreated_object_pool.mutex_dirs_lru, NULL)     != 0)
    {
        LogCrit(COMPONENT_MFSL, "Impossible to initialize mutex_dirs_lru for filler %d.",
                my_filler_data->index);
        exit(1);
    }

    if(pthread_mutex_init(&filler_data[my_filler_data->index].precreated_object_pool.mutex_files_lru, NULL)     != 0)
    {
        LogCrit(COMPONENT_MFSL, "Impossible to initialize mutex_files_lru for filler %d.",
                my_filler_data->index);
        exit(1);
    }

    if((filler_data[my_filler_data->index].precreated_object_pool.dirs_lru = LRU_Init(mfsl_param->lru_param, &lru_status)) == NULL)
    {
        LogCrit(COMPONENT_MFSL, "Impossible to initialize dirs_lru for filler %d. lru_status: %d",
                my_filler_data->index, (int) lru_status);
        exit(1);
    }

    if((filler_data[my_filler_data->index].precreated_object_pool.files_lru = LRU_Init(mfsl_param->lru_param, &lru_status)) == NULL)
    {
        LogCrit(COMPONENT_MFSL, "Impossible to initialize files_lru for filler %d. lru_status: %d",
                my_filler_data->index, (int) lru_status);
        exit(1);
    }

    /* create the pool fro mfsl_precreated_object_t */
    MakePool(&filler_data[my_filler_data->index].precreated_object_pool.pool_precreated_objects,
             (mfsl_param->nb_pre_create_dirs + mfsl_param->nb_pre_create_files),
             mfsl_precreated_object_t, NULL, NULL);


    /* Get root credentials
     **********************/
    fsal_status = FSAL_BuildExportContext(&fsal_export_context, NULL, NULL);
    if(FSAL_IS_ERROR(fsal_status))
    {
        /* Failed init */
        LogCrit(COMPONENT_MFSL,"MFSL filler could not build export context. Status: (%u.%u).",
                fsal_status.major, fsal_status.minor);
        exit(1);
    }

    fsal_status = FSAL_InitClientContext(&filler_data[my_filler_data->index].precreated_object_pool.root_fsal_context);
    if(FSAL_IS_ERROR(fsal_status))
    {
        /* Failed init */
        LogCrit(COMPONENT_MFSL,"MFSL Filler could not build thread context. Status: (%u.%u).",
                fsal_status.major, fsal_status.minor);
        exit(1);
    }

    fsal_status = FSAL_GetClientContext(&filler_data[my_filler_data->index].precreated_object_pool.root_fsal_context,
                    &fsal_export_context, 0, 0, NULL, 0);
    if(FSAL_IS_ERROR(fsal_status))
    {
        /* Failed init */
        LogCrit(COMPONENT_MFSL,"MFSL Filler could not build client context. Status: (%u.%u).",
                fsal_status.major, fsal_status.minor);
        exit(1);
    }

    /* Synchronize */
    rc = pthread_barrier_wait(&filler_barrier);
    if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        LogCrit(COMPONENT_MFSL, "MFSL_filler could not synchronize: %d.", rc);
        exit(1);
    }

    if((rc = pthread_once(&filler_init_once, (void *) MFSL_async_filler_init_objects)) != 0)
    {
        LogCrit(COMPONENT_MFSL, "MFSL_filler could not init directories and objects.");
        exit(1);
    }


    /*   Work   *
     ************/
    LogEvent(COMPONENT_MFSL, "Filler %d starts.", my_filler_data->index);

    /* Filler loop. */
    while(!end_of_mfsl)
    {
        /* Compute remaining objects */
        P(my_filler_data->precreated_object_pool.mutex_files_lru);
        remaining_files = (my_filler_data->precreated_object_pool.files_lru->nb_entry - my_filler_data->precreated_object_pool.files_lru->nb_invalid);
        V(my_filler_data->precreated_object_pool.mutex_files_lru);

        P(my_filler_data->precreated_object_pool.mutex_dirs_lru);
        remaining_dirs  = (my_filler_data->precreated_object_pool.dirs_lru->nb_entry  - my_filler_data->precreated_object_pool.dirs_lru->nb_invalid);
        V(my_filler_data->precreated_object_pool.mutex_dirs_lru);

        LogDebug(COMPONENT_MFSL, "Remaining files: %d. Remaining directories: %d.", remaining_files, remaining_dirs);

        /* Wait for a signal from MFSL_get_precreated_object */
        P(my_filler_data->mutex_watermark_condvar);
        while(   (remaining_dirs  > mfsl_param->AFT_low_watermark )
              && (remaining_files > mfsl_param->AFT_low_watermark ))
        {
            rc = pthread_cond_wait(&my_filler_data->watermark_condvar, &my_filler_data->mutex_watermark_condvar);
            if(rc != 0)
                LogMajor(COMPONENT_MFSL, "pthread_cond_wait failed: %d.", rc);
        }
        V(my_filler_data->mutex_watermark_condvar);

        if(remaining_dirs < mfsl_param->AFT_low_watermark){
            /* We're below watermark, fill directories */
            rc = MFSL_async_filler_fill_directories(my_filler_data->index, mfsl_param->AFT_nb_fill_critical, NULL);
            if(rc != 0)
            {
                LogCrit(COMPONENT_MFSL, "Impossible to fill directories for filler #%d.", my_filler_data->index);
            }
        
            /* Garbage collect */
            if(LRU_gc_invalid(my_filler_data->precreated_object_pool.dirs_lru, NULL) != LRU_LIST_SUCCESS)
                LogCrit(COMPONENT_MFSL, "Impossible to garbage collect dirs_lru in filler %d.", my_filler_data->index);
        }

        if(remaining_files < mfsl_param->AFT_low_watermark){
            /* We're below watermark, fill files */
            rc = MFSL_async_filler_fill_files(my_filler_data->index, mfsl_param->AFT_nb_fill_critical, NULL);
            if(rc != 0)
            {
                LogCrit(COMPONENT_MFSL, "Impossible to fill files for filler #%d.", my_filler_data->index);
            }

            /* Garbage collect */
            if(LRU_gc_invalid(my_filler_data->precreated_object_pool.files_lru, NULL) != LRU_LIST_SUCCESS)
                LogCrit(COMPONENT_MFSL, "Impossible to garbage collect files_lru in filler %d.", my_filler_data->index);
        }
        
    } /* end_of_mfsl */

    LogEvent(COMPONENT_MFSL, "Filler %d Ends.", my_filler_data->index);

    return NULL;
} /* mfsl_async_filler_thread */


/**
 *
 * MFSL_async_filler_init: initializes filler threads.
 *
 * Initializes filler threads
 *
 * @param *arg   Not used
 *
 */
fsal_status_t MFSL_async_filler_init(void * arg)
{
    fsal_status_t  fsal_status;
    pthread_attr_t filler_thread_attr;
    int            i=0;
    int            rc=0;

    SetNameFunction("MFSL_async_filler_init");

    LogEvent(COMPONENT_MFSL, "Fillers creation.");

    pthread_attr_init(&filler_thread_attr);
    pthread_attr_setscope(&filler_thread_attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&filler_thread_attr, PTHREAD_CREATE_JOINABLE);

    /* Thread Array */
    if((filler_thread = (pthread_t *)
                Mem_Alloc(mfsl_param->nb_synclet * sizeof(pthread_t))) == NULL)
        MFSL_return(ERR_FSAL_NOMEM, errno);

    /* Data Array */
    if((filler_data = (mfsl_filler_data_t *)
                Mem_Alloc(mfsl_param->nb_synclet * sizeof(mfsl_filler_data_t))) == NULL)
    {
        LogCrit(COMPONENT_MFSL, "Impossible to allocate filler_data. errno: %d.", errno);
        exit(1);
    }

    /* Initializes barrier */
    if((rc = pthread_barrier_init(&filler_barrier, NULL, mfsl_param->nb_synclet)) != 0)
    {
        LogCrit(COMPONENT_MFSL, "Impossible to create barrier to synchronize filers: %d.", rc);
        exit(1);
    }

    /* Start fillers
     ***************/
    /** \todo change parameter name */
    for(i=0; i < mfsl_param->nb_synclet; i++)
    {
        LogDebug(COMPONENT_MFSL, "Creation of filler number %d.", i);

        filler_data[i].index = i;

        if((rc = pthread_create(&filler_thread[i],
                        &filler_thread_attr,
                        mfsl_async_filler_thread,
                        (void *) &filler_data[i]))     < 0)
            MFSL_return(ERR_FSAL_SERVERFAULT, -rc);
    }

    LogDebug(COMPONENT_MFSL, "All fillers start working.");

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
} /* MFSL_async_filler_init */

#endif /* _USE_SWIG */
