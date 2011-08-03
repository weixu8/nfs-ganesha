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

#ifndef _MFSL_ASYNC_TYPES_H
#define _MFSL_ASYNC_TYPES_H

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

#define CONF_LABEL_MFSL_ASYNC          "MFSL_ASYNC"

/* other includes */
#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>             /* for MAXNAMLEN */
#include "config_parsing.h"
#include "err_fsal.h"
#include "err_mfsl.h"
#include "LRU_List.h"


/* Misc
 ******/
typedef struct mfsl_parameter__
{
    unsigned int    nb_pre_async_op_desc;           /**< Number of preallocated Async Op descriptors      */
    unsigned int    nb_synclet;                     /**< Number of synclet to be used                     */
    unsigned int    nb_before_gc;                   /**< Numbers of calls before LRU invalide GC          */
    long int        async_window_sec;               /**< Asynchronous Task Dispatcher Window (seconds)    */
    long int        async_window_usec;              /**< Asynchronous Task Dispatcher Window (useconds)   */
    unsigned int    ADT_sleep_time;                 /**< Asynchronous Dispatcher Thread sleep time (usec) */
    unsigned int    nb_pre_create_dirs;             /**< The size of pre-created directories per synclet  */
    unsigned int    nb_pre_create_files;            /**< The size of pre-created files per synclet        */
    char            pre_create_obj_dir[MAXPATHLEN]; /**< Directory for pre-createed objects               */
    LRU_parameter_t lru_param;                      /**< Parameter to LRU for async op                    */
    /* Asynchronous Filler Thread */
    unsigned int    AFT_low_watermark;
    unsigned int    AFT_timeout;
    unsigned int    AFT_nb_fill_critical;
    unsigned int    AFT_nb_fill_timeout;
} mfsl_parameter_t;

typedef struct mfsl_context__
{
    struct prealloc_pool pool_async_op;     /* Asynchronous operations pool */
    pthread_mutex_t      lock;
    fsal_op_context_t    root_fsal_context; /* Credentials for root. Used by MFSL_create to manage precreated object. */
} mfsl_context_t;

typedef struct mfsl_object__
{
    fsal_handle_t  handle;
    /* asynchronism manageemnt */
    struct timeval last_op_time;
    int            last_synclet_index;
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


/* Dispatcher, fillers and synclets specific functions
 *****************************************************/
typedef struct mfsl_precreated_object__
{
  mfsl_object_t      mobject;
  fsal_name_t        filename;
  fsal_attrib_list_t object_attributes;
} mfsl_precreated_object_t;

typedef struct mfsl_synclet_context__
{
    pthread_mutex_t lock;
} mfsl_synclet_context_t;

typedef struct mfsl_synclet_data__
{
    unsigned int             index;                 /* Index of the synclet related to this data */
    pthread_cond_t           op_condvar;            /* The condition the synclet is waiting for before wake up. */
    pthread_mutex_t          mutex_op_condvar;      /* Mutex associated with previous condition. */
    mfsl_synclet_context_t   synclet_context;       /**/
    unsigned int             passcounter;           /* Number of loop made by the synclet. Used for garbage collect. */
    /* async_op_desc management */
    LRU_list_t             * op_lru;                /* This list contains the operation to be processed by the current synclet */
    pthread_mutex_t          mutex_op_lru;          /* Mutex that owns the operations_lru */
    LRU_list_t             * failed_op_lru;         /* This list contains the operation that failed to be processed by the current synclet */
    pthread_mutex_t          mutex_failed_op_lru;   /* Mutex that owns the failed_operations_lru */
    /* asynchronism management */
    struct timeval           last_op_time;
    pthread_mutex_t          last_op_time_mutex;
} mfsl_synclet_data_t;

typedef struct mfsl_filler_context__
{
    pthread_mutex_t lock;
} mfsl_filler_context_t;

typedef struct mfsl_filler_pool__
{
    LRU_list_t             * dirs_lru;                /* List of precreated directories */
    pthread_mutex_t          mutex_dirs_lru;          /* Mutex that owns the dirs_lru */
    LRU_list_t             * files_lru;               /* List of precreated directories */
    pthread_mutex_t          mutex_files_lru;         /* Mutex that owns the files_lru */
    fsal_op_context_t        root_fsal_context;       /* Credentials for root. Used by MFSL_create to manage precreated object. */
    pthread_mutex_t          mutex_pool_objects;      /* to lock the pool */
    struct prealloc_pool     pool_precreated_objects; /* Asynchronous operations pool */
    fsal_handle_t            filler_pool_handle;      /* handle of filler local pool */
    fsal_handle_t            dirs_pool_handle;        /* handle of precreated dirs */
    fsal_handle_t            files_pool_handle;       /* handle of precreated files */
} mfsl_filler_pool_t;

typedef struct mfsl_filler_data__
{
    unsigned int             index;                   /* Index of the filler related to this data */
    pthread_cond_t           watermark_condvar;       /* The condition the filler is waiting for before wake up. */
    pthread_mutex_t          mutex_watermark_condvar; /* Mutex associated with previous condition. */
    mfsl_filler_context_t    filler_context;          /**/
    /* precreated objects management */
    mfsl_filler_pool_t       precreated_object_pool;  /**/
} mfsl_filler_data_t;


fsal_status_t MFSL_async_dispatcher_init(void * arg);
fsal_status_t MFSL_async_filler_init(void * arg);
fsal_status_t MFSL_async_synclet_init(void * arg);


/* Operations management
 ***********************/
/* unlink */
typedef struct mfsl_async_op_unlink_args__
{
    fsal_handle_t      parentdir_handle;
    fsal_name_t        object_name;
    fsal_op_context_t  context;
    fsal_attrib_list_t parentdir_attributes;
} mfsl_async_op_unlink_args_t;

typedef struct mfsl_async_op_unlink_res__
{
    fsal_attrib_list_t parentdir_attributes;
} mfsl_async_op_unlink_res_t;

/* link */
typedef struct mfsl_async_op_link_args__
{
    fsal_handle_t      target_handle;
    fsal_handle_t      dir_handle;
    fsal_name_t        link_name;
    fsal_op_context_t  context;
    fsal_attrib_list_t linked_object_attributes;
} mfsl_async_op_link_args_t;

typedef struct mfsl_async_op_link_res__
{
    fsal_attrib_list_t linked_object_attributes;
} mfsl_async_op_link_res_t;

/* rename */
typedef struct mfsl_async_op_rename_args__
{
    fsal_handle_t      src_parentdir_handle;
    fsal_name_t        src_name;
    fsal_handle_t      tgt_parentdir_handle;
    fsal_name_t        tgt_name;
    fsal_op_context_t  context;
    fsal_attrib_list_t src_dir_attributes;
    fsal_attrib_list_t tgt_dir_attributes;
} mfsl_async_op_rename_args_t;

typedef struct mfsl_async_op_rename_res__
{
    fsal_attrib_list_t src_dir_attributes;
    fsal_attrib_list_t tgt_dir_attributes;
} mfsl_async_op_rename_res_t;

/* setattrs */
typedef struct mfsl_async_op_setattrs_args__
{
    fsal_handle_t      file_handle;
    fsal_op_context_t  context;
    fsal_attrib_list_t attrib_set;
    fsal_attrib_list_t object_attributes;
} mfsl_async_op_setattrs_args_t;

typedef struct mfsl_async_op_setattrs_res__
{
    fsal_attrib_list_t object_attributes;
} mfsl_async_op_setattrs_res_t;

/* mkdir */
typedef struct mfsl_async_op_mkdir_args__
{
    fsal_handle_t      old_parentdir_handle;
    fsal_name_t        old_dirname;
    fsal_handle_t      new_parentdir_handle;
    fsal_name_t        new_dirname;
    fsal_handle_t      new_dir_handle;
    fsal_op_context_t  context;
    fsal_attrib_list_t new_dir_attributes;
} mfsl_async_op_mkdir_args_t;

typedef struct mfsl_async_op_mkdir_res__
{
    fsal_handle_t      new_dir_handle;
    fsal_attrib_list_t new_dir_attributes;
    fsal_attrib_list_t new_parentdir_attributes;
} mfsl_async_op_mkdir_res_t;

/* create */
typedef struct mfsl_async_op_create_args__
{
    fsal_handle_t      old_parentdir_handle;
    fsal_name_t        old_filename;
    fsal_handle_t      new_parentdir_handle;
    fsal_name_t        new_filename;
    fsal_handle_t      new_file_handle;
    fsal_op_context_t  context;
} mfsl_async_op_create_args_t;

typedef struct mfsl_async_op_create_res__
{
    fsal_handle_t      new_file_handle;
    fsal_attrib_list_t new_file_attributes;
    fsal_attrib_list_t new_parentdir_attributes;
} mfsl_async_op_create_res_t;

/* general */
typedef union mfsl_async_op_args__
{
    mfsl_async_op_unlink_args_t   unlink;
    mfsl_async_op_link_args_t     link;
    mfsl_async_op_rename_args_t   rename;
    mfsl_async_op_setattrs_args_t setattrs;
    mfsl_async_op_mkdir_args_t    mkdir;
    mfsl_async_op_create_args_t   create;
} mfsl_async_op_args_t;

typedef union mfsl_async_op_res__
{
    mfsl_async_op_unlink_res_t   unlink;
    mfsl_async_op_link_res_t     link;
    mfsl_async_op_rename_res_t   rename;
    mfsl_async_op_setattrs_res_t setattrs;
    mfsl_async_op_mkdir_res_t    mkdir;
    mfsl_async_op_create_res_t   create;
} mfsl_async_op_res_t;

typedef enum mfsl_async_op_type__
{
    MFSL_ASYNC_OP_UNLINK   = 0,
    MFSL_ASYNC_OP_LINK     = 1,
    MFSL_ASYNC_OP_RENAME   = 2,
    MFSL_ASYNC_OP_SETATTRS = 3,
    MFSL_ASYNC_OP_MKDIR    = 4,
    MFSL_ASYNC_OP_CREATE   = 5,
    MFSL_ASYNC_OP_REMOVE   = 6,
    MFSL_ASYNC_OP_TRUNCATE = 7,
} mfsl_async_op_type_t;

static const char *mfsl_async_op_name[] =
{
    "MFSL_ASYNC_OP_UNLINK",
    "MFSL_ASYNC_OP_LINK",
    "MFSL_ASYNC_OP_RENAME",
    "MFSL_ASYNC_OP_SETATTRS",
    "MFSL_ASYNC_OP_MKDIR",
    "MFSL_ASYNC_OP_CREATE",
    "MFSL_ASYNC_OP_REMOVE",
    "MFSL_ASYNC_OP_TRUNCATE",
};

typedef struct mfsl_async_op_desc__
{
    struct timeval         op_time;    /* date of the operation */
    mfsl_async_op_type_t   op_type;    /* type of the operation */
    mfsl_async_op_args_t   op_args;    /* arguments to pass to operation function */
    mfsl_async_op_res_t    op_res;     /* will contain results of the operation */
    mfsl_async_op_res_t    op_guessed; /* what we computed, to check */
    fsal_status_t (*op_func) (struct mfsl_async_op_desc__ *); /* function to apply on the operation */
    fsal_op_context_t      fsal_op_context;
    caddr_t                ptr_mfsl_context; /* pointer tothe mfsl_context, used in synclet */
    unsigned int           related_synclet_index;
} mfsl_async_op_desc_t;

/* functions */
fsal_status_t MFSL_async_post(mfsl_async_op_desc_t * p_operation_description);

unsigned int MFSL_async_choose_synclet(mfsl_async_op_desc_t * candidate_async_op);

int MFSL_async_object_is_synchronous(mfsl_object_t * p_mfsl_object);

#define MFSL_OBJECT_NO_LAST_SYNCLET -1

/* Precreated files management
 *****************************/
/* Functions */
fsal_status_t MFSL_async_get_precreated_object(unsigned int filler_index,           /* IN */
                                               fsal_nodetype_t type,                /* IN */
                                               mfsl_precreated_object_t ** object); /* IN/OUT */



#endif                          /* _MFSL_ASYNC_TYPES_H */
