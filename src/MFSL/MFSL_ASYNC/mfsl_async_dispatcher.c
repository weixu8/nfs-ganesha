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
 * \file    mfsl_async_dispatcher.c
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

/* We want to use gettimeofday() */
#include <sys/time.h>


#ifndef _USE_SWIG

extern mfsl_parameter_t * mfsl_param;      /* MFSL parameters, from mfsl_async_init.c */
extern unsigned int       end_of_mfsl;     /* from mfsl_async_init.c */

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
    LRU_entry_t  * p_lru_entry = NULL;
    LRU_status_t   lru_status;

    SetNameFunction("MFSL_async_post");

    /* Sanitize */
    if(!p_operation_description)
        MFSL_return(ERR_FSAL_FAULT, 0);

    LogDebug(COMPONENT_MFSL, "Posting operation (%u, %s) to dispatcher.",
            p_operation_description->op_type,
            mfsl_async_op_name[p_operation_description->op_type]);

    P(dispatcher_lru_mutex);

    /* Get an entry in dispatcher_lru. */
    if((p_lru_entry = LRU_new_entry(dispatcher_lru, &lru_status)) == NULL)
    {
        LogMajor(COMPONENT_MFSL,"Impossible to post async operation in LRU dispatch list.");

        V(dispatcher_lru_mutex);

        MFSL_return(ERR_FSAL_SERVERFAULT, (int)lru_status);
    }

    /* Attaches the operation to it. */
    p_lru_entry->buffdata.pdata = (caddr_t) p_operation_description;
    p_lru_entry->buffdata.len   = sizeof(mfsl_async_op_desc_t);

    /* Make it valid */
    p_lru_entry->valid_state = LRU_ENTRY_VALID;

    V(dispatcher_lru_mutex);

    LogDebug(COMPONENT_MFSL, "Operation (%u, %s) posted to dispatcher.",
            p_operation_description->op_type,
            mfsl_async_op_name[p_operation_description->op_type]);

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
}

/**
 * mfsl_async_dispatcher_thread: this thread will assign asynchronous operation to the synclets.
 *
 * @param Arg (unused)
 *
 * @return Pointer to the result (but this function will mostly loop forever).
 *
 */
void * mfsl_async_dispatcher_thread(void * arg)
{
    mfsl_async_op_desc_t * current_async_operation;
    LRU_entry_t          * current_lru_entry;
    LRU_entry_t          * synclet_lru_entry;
    LRU_status_t           lru_status;             /* Used when adding an entry in a LRU */
    unsigned int           chosen_synclet;
    struct timeval         current_time;
    struct timeval         delta_time;
    int                    passcounter = 0;
    int                    rc = 0;                 /* Used for BuddyInit */

    SetNameFunction("mfsl_async_dispatcher_thread");

    /*************************
     *    Initialisation
     *************************/
    LogEvent(COMPONENT_MFSL, "Dispatcher initialisation.");

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

    /* Initialize LRU */
    if(pthread_mutex_init(&dispatcher_lru_mutex, NULL) != 0)
    {
        LogCrit(COMPONENT_MFSL, "Impossible to initialize dispatcher_lru_mutex.");
        exit(1);
    }

    if((dispatcher_lru = LRU_Init(mfsl_param->lru_param, &lru_status)) == NULL)
    {
        LogCrit(COMPONENT_MFSL, "Impossible to initialize dispatcher_lru.");
        exit(1);
    }

    /*************************
     *         Work
     *************************/
    LogEvent(COMPONENT_MFSL, "Dispatcher starts.");

    /* Dispatcher loop. */
    while(!end_of_mfsl)
    {
        /* Sleep some time */
        sleep(mfsl_param->ADT_sleep_time);

        /* Get current time */
        if(gettimeofday(&current_time, NULL) != 0)
        {
            /* Could'not get time of day... */
            LogCrit(COMPONENT_MFSL, "Cannot get time of day...");
            exit(1);
        }

        /* Loop on dispatcher_lru */
        P(dispatcher_lru_mutex);
        for(current_lru_entry=dispatcher_lru->LRU ; current_lru_entry != NULL ;
                current_lru_entry=current_lru_entry->next)
        {
            /* ignore invalid entries */
            if(current_lru_entry->valid_state == LRU_ENTRY_VALID)
            {
                /* Get current asynchronous operation to perform. */
                current_async_operation = (mfsl_async_op_desc_t *) (current_lru_entry->buffdata.pdata);

                /* We only perform operations out of the asynchronous window.
                 * Operations are in chronological order.
                 * */
                timersub(&current_time, &(current_async_operation->op_time), &delta_time);

                if(delta_time.tv_sec < mfsl_param->async_window_sec)
                    break;
                if(delta_time.tv_usec < mfsl_param->async_window_sec)
                    break;

                LogDebug(COMPONENT_MFSL, "Found an operation to process: %p.", (caddr_t) current_async_operation);

                /* Retrieve previouslychosen synclet */
                chosen_synclet = current_async_operation->related_synclet_index;

                /* Insert the operation in this synclet's lru */
                P(synclet_data[chosen_synclet].mutex_op_lru);

                synclet_lru_entry = LRU_new_entry(synclet_data[chosen_synclet].op_lru, &lru_status);
                if(!synclet_lru_entry)
                {
                    LogCrit(COMPONENT_MFSL, "Impossible to get an entry in synclet %d.", chosen_synclet);
                    V(synclet_data[chosen_synclet].mutex_op_lru);
                    continue;   /* Go for next entry in dispatcher_lru */
                }

                synclet_lru_entry->buffdata.pdata = current_lru_entry->buffdata.pdata;
                synclet_lru_entry->buffdata.len   = current_lru_entry->buffdata.len;
                synclet_lru_entry->valid_state    = LRU_ENTRY_VALID;

                V(synclet_data[chosen_synclet].mutex_op_lru);

                /* Notify this synclet */
                P(synclet_data[chosen_synclet].mutex_op_condvar);
                if(pthread_cond_signal(&synclet_data[chosen_synclet].op_condvar) == -1)
                    LogCrit(COMPONENT_MFSL, "Impossible to pthread_cond_signal to synclet %d.", chosen_synclet);
                V(synclet_data[chosen_synclet].mutex_op_condvar);

                /* Invalidate the operation in the dispatcher's lru */
                if(LRU_invalidate(dispatcher_lru, current_lru_entry) != LRU_LIST_SUCCESS)
                    LogCrit(COMPONENT_MFSL, "Impossible to invalidate current_lru_entry in dispatcher_lru.");
            }
        }

        /* We just made one another loop */
        passcounter += 1;

        /* Garbage collect */
        if(passcounter >= mfsl_param->nb_before_gc)
        {
            if(LRU_gc_invalid(dispatcher_lru, NULL) != LRU_LIST_SUCCESS)
                LogCrit(COMPONENT_MFSL, "Impossible to garbage collect dispatcher_lru. passcounter: %d.", passcounter);

            passcounter = 0;
        }

        V(dispatcher_lru_mutex);
    } /* end_of_mfsl */

    LogEvent(COMPONENT_MFSL, "Dispatcher Ends.");

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
    pthread_attr_t dispatcher_thread_attr; /* Thread attributes */
    int            rc = 0;

    SetNameFunction("MFSL_async_dispatcher_init");

    LogEvent(COMPONENT_MFSL, "Dispatcher creation.");

    pthread_attr_init(&dispatcher_thread_attr);
    pthread_attr_setscope(&dispatcher_thread_attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&dispatcher_thread_attr, PTHREAD_CREATE_JOINABLE);

    if((rc = pthread_create(&dispatcher_thread,
                            &dispatcher_thread_attr,
                            mfsl_async_dispatcher_thread,
                            (void *)NULL)   ) != 0)
        MFSL_return(ERR_FSAL_SERVERFAULT, -rc);

    MFSL_return(ERR_FSAL_NO_ERROR, 0);
}

#endif /* _USE_SWIG */
