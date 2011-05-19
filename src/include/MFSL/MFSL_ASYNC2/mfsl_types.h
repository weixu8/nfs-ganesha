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
 * \file    mfsl_types.h
 */

#ifndef _MFSL_ASYNC2_TYPES_H
#define _MFSL_ASYNC2_TYPES_H

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Some habits concerning mutex management */
#ifndef P
#define P( a ) pthread_mutex_lock( &a )
#endif

#ifndef V
#define V( a ) pthread_mutex_unlock( &a )
#endif

/*
 * labels in the config file
 */

#define CONF_LABEL_MFSL_ASYNC2          "MFSL_ASYNC"

/* other includes */
#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>             /* for MAXNAMLEN */
#include "config_parsing.h"
#include "err_fsal.h"
#include "err_mfsl.h"
#include "LRU_List.h"

typedef struct mfsl_parameter__
{
    unsigned int    nb_pre_async_op_desc; /**< Number of preallocated Async Op descriptors      */
    unsigned int    nb_synclet;           /**< Number of synclet to be used                     */
    unsigned int    nb_before_gc;         /**< Numbers of calls before LRU invalide GC          */
    long int        async_window_sec;     /**< Asynchronous Task Dispatcher Window (seconds)    */
    long int        async_window_usec;    /**< Asynchronous Task Dispatcher Window (useconds)   */
    unsigned int    ADT_sleep_time;       /**< Asynchronous Dispatcher Thread sleep time (usec) */
    LRU_parameter_t lru_param;            /**< Parameter to LRU for async op                    */
} mfsl_parameter_t;

typedef struct mfsl_context__
{
    struct prealloc_pool pool_async_op; /**< Asynchronous operations pool */
    pthread_mutex_t lock;
} mfsl_context_t;

typedef struct mfsl_object__
{
    fsal_handle_t handle;
} mfsl_object_t;

typedef struct mfsl_file__
{
    fsal_file_t fsal_file ;
} mfsl_file_t ;

/* this one is used in MFSL_link(). */
typedef struct mfsl_dirs_attributes__
{
    fsal_attrib_list_t * src_dir_attrs;
    fsal_attrib_list_t * dest_dir_attrs;
} mfsl_dirs_attributes_t;

/**
 * Dispatcher and synclets specific functions
 * */
typedef struct mfsl_synclet_context__
{
    pthread_mutex_t lock;
} mfsl_synclet_context_t;

typedef struct mfsl_synclet_data__
{
    unsigned int             index;             /* Index of the synclet related to this data */
    pthread_cond_t           op_condvar;        /**/
    pthread_mutex_t          mutex_op_condvar;  /**/
    fsal_op_context_t        root_fsal_context; /**/
    mfsl_synclet_context_t   synclet_context;   /**/
    pthread_mutex_t          mutex_op_lru;      /**/
    unsigned int             passcounter;       /**/
    LRU_list_t             * op_lru;            /**/
} mfsl_synclet_data_t;

fsal_status_t MFSL_async_dispatcher_init(void * arg);
fsal_status_t MFSL_async_synclet_init(void * arg);

/**
 * Operations management
 * */

/* unlink */
typedef struct mfsl_async_op_unlink_args__
{
    fsal_handle_t      * parentdir_handle;
    fsal_name_t        * p_object_name;
    fsal_op_context_t  * p_context;
    fsal_attrib_list_t * parentdir_attributes;
} mfsl_async_op_unlink_args_t;

typedef struct mfsl_async_op_unlink_res__
{
    fsal_attrib_list_t * parentdir_attributes;
} mfsl_async_op_unlink_res_t;

/* general */
typedef union mfsl_async_op_args__
{
    mfsl_async_op_unlink_args_t unlink;
} mfsl_async_op_args_t;

typedef union mfsl_async_op_res__
{
    mfsl_async_op_unlink_res_t  unlink;
} mfsl_async_op_res_t;

typedef enum mfsl_async_op_type__
{
    MFSL_ASYNC_OP_UNLINK   = 0,
    MFSL_ASYNC_OP_LINK     = 1,
    MFSL_ASYNC_OP_RENAME   = 2,
    MFSL_ASYNC_OP_SETATTR  = 3,
    MFSL_ASYNC_OP_CREATE   = 4,
    MFSL_ASYNC_OP_MKDIR    = 5,
    MFSL_ASYNC_OP_REMOVE   = 6,
    MFSL_ASYNC_OP_TRUNCATE = 7,
    MFSL_ASYNC_OP_SYMLINK  = 8
} mfsl_async_op_type_t;

static const char *mfsl_async_op_name[] =
{
    "MFSL_ASYNC_OP_UNLINK",
    "MFSL_ASYNC_OP_LINK",
    "MFSL_ASYNC_OP_RENAME",
    "MFSL_ASYNC_OP_SETATTR",
    "MFSL_ASYNC_OP_CREATE",
    "MFSL_ASYNC_OP_MKDIR",
    "MFSL_ASYNC_OP_REMOVE",
    "MFSL_ASYNC_OP_TRUNCATE",
    "MFSL_ASYNC_OP_SYMLINK"
};

typedef struct mfsl_async_op_desc__
{
    struct timeval         op_time;
    mfsl_async_op_type_t   op_type;
    mfsl_async_op_args_t   op_args;
    mfsl_async_op_res_t    op_res;
    mfsl_async_op_res_t    op_guessed;
    mfsl_object_t        * op_mobject;
    fsal_status_t (*op_func) (struct mfsl_async_op_desc__ *);
    fsal_op_context_t      fsal_op_context;
    caddr_t                ptr_mfsl_context;
    unsigned int           related_synclet_index;
} mfsl_async_op_desc_t;

/* functions */
fsal_status_t MFSL_async_post(mfsl_async_op_desc_t * p_operation_description);

unsigned int MFSL_async_choose_synclet(mfsl_async_op_desc_t * candidate_async_op);

#endif                          /* _MFSL_ASYNC2_TYPES_H */
