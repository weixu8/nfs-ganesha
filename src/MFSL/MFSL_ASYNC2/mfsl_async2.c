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
 * \file    mfsl_async2.c
 * \author  $Author: leibovic $
 * \date    $Date: 2011/05/13 13:40:00 $
 * \version $Revision: 1.72 $
 * \brief   Asynchronous MFSL Layer.
 *
 */

#include "config.h"

/* fsal_types contains constants and type definitions for FSAL */
#include "fsal_types.h"
#include "fsal.h"
#include "mfsl_types.h"
#include "mfsl.h"
#include "common_utils.h"

/* We want to use memcpy() */
#include <string.h>

/******************************************************
 *              Common Filesystem calls.
 ******************************************************/

fsal_status_t MFSL_lookup(mfsl_object_t * parent_directory_handle,      /* IN */
                          fsal_name_t * p_filename,     /* IN */
                          fsal_op_context_t * p_context,        /* IN */
                          mfsl_context_t * p_mfsl_context,      /* IN */
                          mfsl_object_t * object_handle,        /* OUT */
                          fsal_attrib_list_t * object_attributes,        /* [ IN/OUT ] */
			  void * pextra
    )
{
  return FSAL_lookup(&parent_directory_handle->handle,
                     p_filename, p_context, &object_handle->handle, object_attributes);
}                               /* MFSL_lookup */

fsal_status_t MFSL_lookupPath(fsal_path_t * p_path,     /* IN */
                              fsal_op_context_t * p_context,    /* IN */
                              mfsl_context_t * p_mfsl_context,  /* IN */
                              mfsl_object_t * object_handle,    /* OUT */
                              fsal_attrib_list_t * object_attributes    /* [ IN/OUT ] */
    )
{
  return FSAL_lookupPath(p_path, p_context, &object_handle->handle, object_attributes);
}                               /* MFSL_lookupPath */

fsal_status_t MFSL_lookupJunction(mfsl_object_t * p_junction_handle,    /* IN */
                                  fsal_op_context_t * p_context,        /* IN */
                                  mfsl_context_t * p_mfsl_context,      /* IN */
                                  mfsl_object_t * p_fsoot_handle,       /* OUT */
                                  fsal_attrib_list_t * p_fsroot_attributes      /* [ IN/OUT ] */
    )
{
  return FSAL_lookupJunction(&p_junction_handle->handle,
                             p_context, &p_fsoot_handle->handle, p_fsroot_attributes);
}                               /* MFSL_lookupJunction */

fsal_status_t MFSL_access(mfsl_object_t * object_handle,        /* IN */
                          fsal_op_context_t * p_context,        /* IN */
                          mfsl_context_t * p_mfsl_context,      /* IN */
                          fsal_accessflags_t access_type,       /* IN */
                          fsal_attrib_list_t * object_attributes,        /* [ IN/OUT ] */
              		  void * pextra
    )
{
  return FSAL_access(&object_handle->handle, p_context, access_type, object_attributes);
}                               /* MFSL_access */

fsal_status_t MFSL_create(mfsl_object_t * parent_directory_handle,      /* IN */
                          fsal_name_t * p_filename,     /* IN */
                          fsal_op_context_t * p_context,        /* IN */
                          mfsl_context_t * p_mfsl_context,      /* IN */
                          fsal_accessmode_t accessmode, /* IN */
                          mfsl_object_t * object_handle,        /* OUT */
                          fsal_attrib_list_t * object_attributes,       /* [ IN/OUT ] */
                          fsal_attrib_list_t * parent_attributes,        /* [ IN/OUT ] */
			  void * pextra
    )
{
  return FSAL_create(&parent_directory_handle->handle,
                     p_filename,
                     p_context, accessmode, &object_handle->handle, object_attributes);
}                               /* MFSL_create */

fsal_status_t MFSL_mkdir(mfsl_object_t * parent_directory_handle,       /* IN */
                         fsal_name_t * p_dirname,       /* IN */
                         fsal_op_context_t * p_context, /* IN */
                         mfsl_context_t * p_mfsl_context,       /* IN */
                         fsal_accessmode_t accessmode,  /* IN */
                         mfsl_object_t * object_handle, /* OUT */
                         fsal_attrib_list_t * object_attributes,        /* [ IN/OUT ] */
                         fsal_attrib_list_t * parent_attributes, /* [ IN/OUT ] */
			 void * pextra
    )
{
  return FSAL_mkdir(&parent_directory_handle->handle,
                    p_dirname,
                    p_context, accessmode, &object_handle->handle, object_attributes);
}                               /* MFSL_mkdir */

fsal_status_t MFSL_truncate(mfsl_object_t * filehandle, /* IN */
                            fsal_op_context_t * p_context,      /* IN */
                            mfsl_context_t * p_mfsl_context,    /* IN */
                            fsal_size_t length, /* IN */
                            mfsl_file_t * file_descriptor,      /* INOUT */
                            fsal_attrib_list_t * object_attributes,      /* [ IN/OUT ] */
			    void * pextra
    )
{
  return FSAL_truncate(&filehandle->handle,
                       p_context, length, &file_descriptor->fsal_file, object_attributes);
}                               /* MFSL_truncate */

fsal_status_t MFSL_getattrs(mfsl_object_t * filehandle, /* IN */
                            fsal_op_context_t * p_context,      /* IN */
                            mfsl_context_t * p_mfsl_context,    /* IN */
                            fsal_attrib_list_t * object_attributes,      /* IN/OUT */
			    void * pextra
    )
{
  return FSAL_getattrs(&filehandle->handle, p_context, object_attributes);
}                               /* MFSL_getattrs */

fsal_status_t MFSL_setattrs(mfsl_object_t      * filehandle,        /* IN */
                            fsal_op_context_t  * p_context,         /* IN */
                            mfsl_context_t     * p_mfsl_context,    /* IN */
                            fsal_attrib_list_t * attrib_set,        /* IN */
                            fsal_attrib_list_t * object_attributes, /* [ IN/OUT ] */
			    void               * pextra
    )
{
	fsal_status_t fsal_status;
	fsal_status_t fsal_status2;
	fsal_status_t fsal_status3;

	fsal_attrib_list_t object_attributes_new;

	/* Sanity checks */
	if(!p_context || !object_attributes || !attrib_set)
		MFSL_return(ERR_FSAL_INVAL, 0);

	/* Check access localy */
	fsal_status2 = FSAL_setattr_access(p_context, attrib_set, object_attributes);

	/* Merge attributes */
	if(FSAL_IS_ERROR(fsal_status2))
	{
		/** @todo:
		MFSL_return(fsal_status2.major, 0);
		*/
	}
	else
	{
		/* Merge */
		fsal_status3 = FSAL_merge_attrs(object_attributes, attrib_set, &object_attributes_new);
		if(FSAL_IS_ERROR(fsal_status3))
			MFSL_return(fsal_status3.major, 0);
	}

	fsal_status = FSAL_setattrs(&filehandle->handle, p_context, attrib_set, object_attributes);

	/* Does status and attributes match? */
	if( (fsal_status.major != fsal_status2.major) || (fsal_status.minor != fsal_status2.minor) )
		LogCrit(COMPONENT_FSAL, "fsal_status don't match! (%u.%u) vs (%u.%u)",
				fsal_status.major, fsal_status.minor,
				fsal_status2.major, fsal_status2.minor
				);

	/** Check attributes validity */
	if(FSAL_TEST_MASK(attrib_set->asked_attributes, FSAL_ATTR_MODE) && (object_attributes_new.mode != object_attributes->mode))
		LogCrit(COMPONENT_FSAL, "Modes do not match.");

	if(FSAL_TEST_MASK(attrib_set->asked_attributes, FSAL_ATTR_OWNER) && (object_attributes_new.owner != object_attributes->owner))
		LogCrit(COMPONENT_FSAL, "Owners do not match.");

	if(FSAL_TEST_MASK(attrib_set->asked_attributes, FSAL_ATTR_GROUP) && (object_attributes_new.group != object_attributes->group))
		LogCrit(COMPONENT_FSAL, "Groups do not match.");

	if(FSAL_TEST_MASK(attrib_set->asked_attributes, FSAL_ATTR_ATIME)
			&& ( (object_attributes_new.atime.seconds != object_attributes->atime.seconds)
			     || (object_attributes_new.atime.nseconds != object_attributes->atime.nseconds) ))
		LogCrit(COMPONENT_FSAL, "Atimes do not match.");

	if(FSAL_TEST_MASK(attrib_set->asked_attributes, FSAL_ATTR_CREATION)
			&& ( (object_attributes_new.creation.seconds != object_attributes->creation.seconds)
			     || (object_attributes_new.creation.nseconds != object_attributes->creation.nseconds) ))
		LogCrit(COMPONENT_FSAL, "Creation times do not match.");

	if(FSAL_TEST_MASK(attrib_set->asked_attributes, FSAL_ATTR_MTIME)
			&& ( (object_attributes_new.mtime.seconds != object_attributes->mtime.seconds)
			     || (object_attributes_new.mtime.nseconds != object_attributes->mtime.nseconds) ))
		LogCrit(COMPONENT_FSAL, "Mtimes do not match.");

	if(FSAL_TEST_MASK(attrib_set->asked_attributes, FSAL_ATTR_CTIME)
			&& ( (object_attributes_new.ctime.seconds != object_attributes->ctime.seconds)
			     || (object_attributes_new.ctime.nseconds != object_attributes->ctime.nseconds) ))
		LogCrit(COMPONENT_FSAL, "Ctimes do not match.");

	return fsal_status;
}                               /* MFSL_setattrs */

fsal_status_t MFSL_link(mfsl_object_t      * target_handle,  /* IN */
                        mfsl_object_t      * dir_handle,     /* IN */
                        fsal_name_t        * p_link_name,    /* IN */
                        fsal_op_context_t  * p_context,      /* IN */
                        mfsl_context_t     * p_mfsl_context, /* IN */
                        fsal_attrib_list_t * p_attr_obj,     /* [ IN/OUT ] */
			void               * pextra)         /* IN */
{
	fsal_status_t fsal_status;
	fsal_status_t fsal_status2;

	mfsl_dirs_attributes_t * dirs_attrs;
	fsal_attrib_list_t * p_attr_srcdir;  /* given source directory attributes */
	fsal_attrib_list_t * p_attr_destdir; /* given destination directory attributes */

	fsal_attrib_list_t attr_destdir_new; /* guessed destination directory attributes */
	fsal_attrib_list_t attr_destdir_old; /* syncly retrived destination directory attributes, just to check */
	fsal_attrib_list_t attr_obj_new;     /* guessed object attributes */

	/* Sanity checks */
	if(!p_context || !p_attr_destdir || !pextra)
		MFSL_return(ERR_FSAL_INVAL, 0);

	/* pextra contains destination and source directory attributes */
	dirs_attrs = (mfsl_dirs_attributes_t *) pextra;
	p_attr_destdir = dirs_attrs->src_dir_attrs;
	p_attr_srcdir = dirs_attrs->dest_dir_attrs;

	/* Sanity checks: are theses directories? */
	if( (p_attr_srcdir->type != FSAL_TYPE_DIR) || (p_attr_destdir->type != FSAL_TYPE_DIR) )
		MFSL_return(ERR_FSAL_NOTDIR, 0);

	/* copy destination directory attributes in a new structure */
	memcpy((void *) &attr_destdir_new, (void *) p_attr_destdir, sizeof(fsal_attrib_list_t));
	memcpy((void *) &attr_destdir_old, (void *) p_attr_destdir, sizeof(fsal_attrib_list_t));
	/* copy object attributes in a new structure */
	memcpy((void *) &attr_obj_new, (void *) p_attr_obj, sizeof(fsal_attrib_list_t));

	/* Async Check */
	fsal_status2 = FSAL_link_access(p_context, p_attr_destdir, p_attr_srcdir);

	/* Sync Check and do */
	fsal_status = FSAL_link(&target_handle->handle,
			        &dir_handle->handle,
				p_link_name,
				p_context,
				p_attr_obj
				);

	/* Guess destination directory attributes */
	attr_destdir_new.filesize      += 1;    /** todo: this is not true for all FSAL... */
	attr_destdir_new.ctime.seconds  = time(NULL);
	attr_destdir_new.ctime.nseconds = 0;
	attr_destdir_new.mtime.seconds  = attr_destdir_new.ctime.seconds;
	attr_destdir_new.mtime.nseconds = 0;
	/* Syncly retrieve destination directory attributes, just to check */
	FSAL_getattrs(&dir_handle->handle, p_context, &attr_destdir_old);
	/* Guess object attributes */
	attr_obj_new.numlinks      += 1;
	attr_obj_new.ctime.seconds  = attr_destdir_new.ctime.seconds;
	attr_obj_new.ctime.nseconds = attr_destdir_new.ctime.nseconds;

	/* Guessed destination directory attributes should match with sync ones */
	if(                (attr_destdir_old.filesize       != attr_destdir_new.filesize)
			|| (attr_destdir_old.ctime.seconds  != attr_destdir_new.ctime.seconds)
			|| (attr_destdir_old.ctime.nseconds != attr_destdir_new.ctime.nseconds)
			|| (attr_destdir_old.mtime.seconds  != attr_destdir_new.mtime.seconds)
			|| (attr_destdir_old.mtime.nseconds != attr_destdir_new.mtime.nseconds)
		)
		LogCrit(COMPONENT_FSAL, "Guessed destination directory attributes don't match with sync ones! Size: %llu vs %llu. Ctime: (%u.%u) vs (%u.%u). Mtime: (%u.%u) vs (%u.%u).",
				attr_destdir_old.filesize,         attr_destdir_new.filesize,
				attr_destdir_old.ctime.seconds,    attr_destdir_old.ctime.nseconds,
				attr_destdir_new.ctime.seconds,    attr_destdir_new.ctime.nseconds,
				attr_destdir_old.mtime.seconds,    attr_destdir_old.mtime.nseconds,
				attr_destdir_new.mtime.seconds,    attr_destdir_new.mtime.nseconds
				);

	/* Guessed object attributes should match with sync ones */
	if(                (attr_obj_new.numlinks       != p_attr_obj->numlinks)
			|| (attr_obj_new.ctime.seconds  != p_attr_obj->ctime.seconds)
			|| (attr_obj_new.ctime.nseconds != p_attr_obj->ctime.nseconds)
			)
		LogCrit(COMPONENT_FSAL, "Guessed object attributes don't match with sync ones! Numlinks: %lu vs %lu. Ctime: (%u.%u) vs (%u.%u).",
				p_attr_obj->numlinks,         attr_obj_new.numlinks,
				p_attr_obj->ctime.seconds,    p_attr_obj->ctime.nseconds,
				attr_obj_new.ctime.seconds, attr_obj_new.ctime.nseconds
				);

	/* Status should match */
	if((fsal_status.major != fsal_status2.major) || (fsal_status.minor != fsal_status2.minor))
		LogCrit(COMPONENT_FSAL, "Sync and Async status don't match! Sync: (%u,%u). Async: (%u,%u).",
				fsal_status.major,  fsal_status.minor,
				fsal_status2.major, fsal_status2.minor
				);

	return fsal_status;
}                               /* MFSL_link */

fsal_status_t MFSL_opendir(mfsl_object_t * dir_handle,  /* IN */
                           fsal_op_context_t * p_context,       /* IN */
                           mfsl_context_t * p_mfsl_context,     /* IN */
                           fsal_dir_t * dir_descriptor, /* OUT */
                           fsal_attrib_list_t * dir_attributes,  /* [ IN/OUT ] */
			   void * pextra
    )
{
  return FSAL_opendir(&dir_handle->handle, p_context, dir_descriptor, dir_attributes);
}                               /* MFSL_opendir */

fsal_status_t MFSL_readdir(fsal_dir_t * dir_descriptor, /* IN */
                           fsal_cookie_t start_position,        /* IN */
                           fsal_attrib_mask_t get_attr_mask,    /* IN */
                           fsal_mdsize_t buffersize,    /* IN */
                           fsal_dirent_t * pdirent,     /* OUT */
                           fsal_cookie_t * end_position,        /* OUT */
                           fsal_count_t * nb_entries,   /* OUT */
                           fsal_boolean_t * end_of_dir, /* OUT */
                           mfsl_context_t * p_mfsl_context,      /* IN */
			   void * pextra
    )
{
  return FSAL_readdir(dir_descriptor,
                      start_position,
                      get_attr_mask,
                      buffersize, pdirent, end_position, nb_entries, end_of_dir);

}                               /* MFSL_readdir */

fsal_status_t MFSL_closedir(fsal_dir_t * dir_descriptor,        /* IN */
                            mfsl_context_t * p_mfsl_context,     /* IN */
			    void * pextra
    )
{
  return FSAL_closedir(dir_descriptor);
}                               /* FSAL_closedir */

fsal_status_t MFSL_open(mfsl_object_t * filehandle,     /* IN */
                        fsal_op_context_t * p_context,  /* IN */
                        mfsl_context_t * p_mfsl_context,        /* IN */
                        fsal_openflags_t openflags,     /* IN */
                        mfsl_file_t * file_descriptor,  /* OUT */
                        fsal_attrib_list_t * file_attributes,    /* [ IN/OUT ] */
			void * pextra
    )
{
  return FSAL_open(&filehandle->handle,
                   p_context, openflags, &file_descriptor->fsal_file, file_attributes);
}                               /* MFSL_open */

fsal_status_t MFSL_open_by_name(mfsl_object_t * dirhandle,      /* IN */
                                fsal_name_t * filename, /* IN */
                                fsal_op_context_t * p_context,  /* IN */
                                mfsl_context_t * p_mfsl_context,        /* IN */
                                fsal_openflags_t openflags,     /* IN */
                                mfsl_file_t * file_descriptor,  /* OUT */
                                fsal_attrib_list_t * file_attributes, /* [ IN/OUT ] */ 
				void * pextra )
{
  return FSAL_open_by_name(&dirhandle->handle,
                           filename,
                           p_context, openflags, &file_descriptor->fsal_file, file_attributes);
}                               /* MFSL_open_by_name */

fsal_status_t MFSL_open_by_fileid(mfsl_object_t * filehandle,   /* IN */
                                  fsal_u64_t fileid,    /* IN */
                                  fsal_op_context_t * p_context,        /* IN */
                                  mfsl_context_t * p_mfsl_context,      /* IN */
                                  fsal_openflags_t openflags,   /* IN */
                                  mfsl_file_t * file_descriptor,        /* OUT */
                                  fsal_attrib_list_t * file_attributes, /* [ IN/OUT ] */ 
				  void * pextra )
{
  return FSAL_open_by_fileid(&filehandle->handle,
                             fileid,
                             p_context, openflags, &file_descriptor->fsal_file, file_attributes);
}                               /* MFSL_open_by_fileid */

fsal_status_t MFSL_read(mfsl_file_t * file_descriptor,  /*  IN  */
                        fsal_seek_t * seek_descriptor,  /* [IN] */
                        fsal_size_t buffer_size,        /*  IN  */
                        caddr_t buffer, /* OUT  */
                        fsal_size_t * read_amount,      /* OUT  */
                        fsal_boolean_t * end_of_file,   /* OUT  */
                        mfsl_context_t * p_mfsl_context, /* IN */
			void * pextra
    )
{
  return FSAL_read(&file_descriptor->fsal_file,
                   seek_descriptor, buffer_size, buffer, read_amount, end_of_file);
}                               /* MFSL_read */

fsal_status_t MFSL_write(mfsl_file_t * file_descriptor, /* IN */
                         fsal_seek_t * seek_descriptor, /* IN */
                         fsal_size_t buffer_size,       /* IN */
                         caddr_t buffer,        /* IN */
                         fsal_size_t * write_amount,    /* OUT */
                         mfsl_context_t * p_mfsl_context,        /* IN */
			 void * pextra
    )
{
  return FSAL_write(&file_descriptor->fsal_file, seek_descriptor, buffer_size, buffer, write_amount);
}                               /* MFSL_write */

fsal_status_t MFSL_close(mfsl_file_t * file_descriptor, /* IN */
                         mfsl_context_t * p_mfsl_context,        /* IN */
			 void * pextra
    )
{
  return FSAL_close(&file_descriptor->fsal_file);
}                               /* MFSL_close */

fsal_status_t MFSL_sync(mfsl_file_t * file_descriptor /* IN */,
			 void * pextra)
{
   return FSAL_sync( &file_descriptor->fsal_file ) ;
}

fsal_status_t MFSL_close_by_fileid(mfsl_file_t * file_descriptor /* IN */ ,
                                   fsal_u64_t fileid, 
				   mfsl_context_t * p_mfsl_context,  /* IN */
				   void * pextra )
{
  return FSAL_close_by_fileid(&file_descriptor->fsal_file, fileid);
}                               /* MFSL_close_by_fileid */

fsal_status_t MFSL_readlink(mfsl_object_t * linkhandle, /* IN */
                            fsal_op_context_t * p_context,      /* IN */
                            mfsl_context_t * p_mfsl_context,    /* IN */
                            fsal_path_t * p_link_content,       /* OUT */
                            fsal_attrib_list_t * link_attributes,        /* [ IN/OUT ] */
			    void * pextra
    )
{
  return FSAL_readlink(&linkhandle->handle, p_context, p_link_content, link_attributes);
}                               /* MFSL_readlink */

fsal_status_t MFSL_symlink(mfsl_object_t * parent_directory_handle,     /* IN */
                           fsal_name_t * p_linkname,    /* IN */
                           fsal_path_t * p_linkcontent, /* IN */
                           fsal_op_context_t * p_context,       /* IN */
                           mfsl_context_t * p_mfsl_context,     /* IN */
                           fsal_accessmode_t accessmode,        /* IN (ignored); */
                           mfsl_object_t * link_handle, /* OUT */
                           fsal_attrib_list_t * link_attributes, /* [ IN/OUT ] */
			   void * pextra
    )
{
  return FSAL_symlink(&parent_directory_handle->handle,
                      p_linkname,
                      p_linkcontent,
                      p_context, accessmode, &link_handle->handle, link_attributes);
}                               /* MFSL_symlink */

fsal_status_t MFSL_rename(mfsl_object_t      * old_parentdir_handle, /* IN */
                          fsal_name_t        * p_old_name,           /* IN */
                          mfsl_object_t      * new_parentdir_handle, /* IN */
                          fsal_name_t        * p_new_name,           /* IN */
                          fsal_op_context_t  * p_context,            /* IN */
                          mfsl_context_t     * p_mfsl_context,       /* IN */
                          fsal_attrib_list_t * psrc_dir_attributes,  /* [ IN/OUT ] */
                          fsal_attrib_list_t * ptgt_dir_attributes,  /* [ IN/OUT ] */
                          void               * pextra                /* IN */
    )
{
	fsal_status_t fsal_status;
	fsal_status_t fsal_status2; /* Async status */

	fsal_attrib_list_t attr_new_srcdir;
	fsal_attrib_list_t attr_new_destdir;
	fsal_attrib_list_t * attr_old_obj;

	int samedirs;

	/* sanity checks */
	if(!psrc_dir_attributes || !ptgt_dir_attributes || !pextra)
		MFSL_return(ERR_FSAL_FAULT, 0);

	if( (psrc_dir_attributes->type != FSAL_TYPE_DIR) || (ptgt_dir_attributes->type != FSAL_TYPE_DIR) )
		MFSL_return(ERR_FSAL_NOTDIR, 0);

	/* get object attributes */
	attr_old_obj = pextra;

	/* check access asyncly */
	fsal_status2 = FSAL_rename_access(p_context, psrc_dir_attributes, ptgt_dir_attributes, attr_old_obj);

	/** @todo check if error and return it */

	/* guess source directory attributes */
	memcpy((void *) &attr_new_srcdir, (void *) psrc_dir_attributes, sizeof(fsal_attrib_list_t));
	attr_new_srcdir.ctime.seconds  = (fsal_uint_t) time(NULL);
	attr_new_srcdir.ctime.nseconds = 0;
	attr_new_srcdir.mtime.seconds  = attr_new_srcdir.ctime.seconds;
	attr_new_srcdir.mtime.nseconds = 0;

	/* guess destination directory attributes */
	memcpy((void *) &attr_new_destdir, (void *) ptgt_dir_attributes, sizeof(fsal_attrib_list_t));
	attr_new_destdir.ctime.seconds  = attr_new_srcdir.ctime.seconds;
	attr_new_destdir.ctime.nseconds = 0;
	attr_new_destdir.mtime.seconds  = attr_new_srcdir.ctime.seconds;
	attr_new_destdir.mtime.nseconds = 0;

	/* those directories' size change only if they're different */
	samedirs = FSAL_handlecmp(&old_parentdir_handle->handle, &new_parentdir_handle->handle, &fsal_status);

	if(FSAL_IS_ERROR(fsal_status))
		MFSL_return(fsal_status.major, 0);

	if(samedirs != 0)
	{
		/* different dirs */
		attr_new_srcdir.filesize  -= 1;
		attr_new_destdir.filesize += 1;
	}

	/* for the moment, rename syncly */
	fsal_status = FSAL_rename(&old_parentdir_handle->handle,
                     p_old_name,
                     &new_parentdir_handle->handle,
                     p_new_name, p_context, psrc_dir_attributes, ptgt_dir_attributes);

	/* for the moment, check if guessed source directory attributes is ok */
	if(                (attr_new_srcdir.ctime.seconds != psrc_dir_attributes->ctime.seconds)
			|| (attr_new_srcdir.mtime.seconds != psrc_dir_attributes->mtime.seconds)
			|| (attr_new_srcdir.filesize      != psrc_dir_attributes->filesize)
			)
		LogCrit(COMPONENT_FSAL, "Error, guessed source directory attributes don't match with real ones. \
				ctime: (%u.%u) vs (%u.%u); mtime: (%u.%u) vs (%u.%u); filesize: %llu vs %llu.",
				psrc_dir_attributes->ctime.seconds, psrc_dir_attributes->ctime.nseconds,
				attr_new_srcdir.ctime.seconds, attr_new_srcdir.ctime.nseconds,
				psrc_dir_attributes->mtime.seconds, psrc_dir_attributes->mtime.nseconds,
				attr_new_srcdir.mtime.seconds, attr_new_srcdir.mtime.nseconds,
				psrc_dir_attributes->filesize, attr_new_srcdir.filesize
				);

	/* for the moment, check if guessed destination directory attributes is ok */
	if(                (attr_new_destdir.ctime.seconds != ptgt_dir_attributes->ctime.seconds)
			|| (attr_new_destdir.mtime.seconds != ptgt_dir_attributes->mtime.seconds)
			|| (attr_new_destdir.filesize      != ptgt_dir_attributes->filesize)
			)
		LogCrit(COMPONENT_FSAL, "Error, guessed source directory attributes don't match with real ones. \
				ctime: (%u.%u) vs (%u.%u); mtime: (%u.%u) vs (%u.%u); filesize: %llu vs %llu.",
				ptgt_dir_attributes->ctime.seconds, ptgt_dir_attributes->ctime.nseconds,
				attr_new_destdir.ctime.seconds, attr_new_destdir.ctime.nseconds,
				ptgt_dir_attributes->mtime.seconds, ptgt_dir_attributes->mtime.nseconds,
				attr_new_destdir.mtime.seconds, attr_new_destdir.mtime.nseconds,
				ptgt_dir_attributes->filesize, attr_new_destdir.filesize
				);

	/* for the moment, return sync status anyway */
	return fsal_status;
}                               /* MFSL_rename */

fsal_status_t MFSL_mknode(mfsl_object_t * parentdir_handle,     /* IN */
                          fsal_name_t * p_node_name,    /* IN */
                          fsal_op_context_t * p_context,        /* IN */
                          mfsl_context_t * p_mfsl_context,      /* IN */
                          fsal_accessmode_t accessmode, /* IN */
                          fsal_nodetype_t nodetype,     /* IN */
                          fsal_dev_t * dev,     /* IN */
                          mfsl_object_t * p_object_handle,      /* OUT */
                          fsal_attrib_list_t * node_attributes,  /* [ IN/OUT ] */
			  void * pextra
    )
{
  return FSAL_mknode(&parentdir_handle->handle,
                     p_node_name,
                     p_context,
                     accessmode,
                     nodetype, dev, &p_object_handle->handle, node_attributes);
}                               /* MFSL_mknode */

fsal_status_t MFSL_rcp(mfsl_object_t * filehandle,      /* IN */
                       fsal_op_context_t * p_context,   /* IN */
                       mfsl_context_t * p_mfsl_context, /* IN */
                       fsal_path_t * p_local_path,      /* IN */
                       fsal_rcpflag_t transfer_opt,      /* IN */
		       void * pextra
    )
{
  return FSAL_rcp(&filehandle->handle, p_context, p_local_path, transfer_opt);
}                               /* MFSL_rcp */

fsal_status_t MFSL_rcp_by_fileid(mfsl_object_t * filehandle,    /* IN */
                                 fsal_u64_t fileid,     /* IN */
                                 fsal_op_context_t * p_context, /* IN */
                                 mfsl_context_t * p_mfsl_context,       /* IN */
                                 fsal_path_t * p_local_path,    /* IN */
                                 fsal_rcpflag_t transfer_opt,    /* IN */
				 void * pextra
    )
{
  return FSAL_rcp_by_fileid(&filehandle->handle,
                            fileid, p_context, p_local_path, transfer_opt);
}                               /* MFSL_rcp_by_fileid */

#ifndef _USE_SWIG

/******************************************************
 *                FSAL locks management.
 ******************************************************/
fsal_status_t MSFL_lock(mfsl_file_t * obj_handle,       /* IN */
                        fsal_lockdesc_t * ldesc,        /*IN/OUT */
                        fsal_boolean_t callback /* IN */
    )
{
  return FSAL_lock(&obj_handle->fsal_file, ldesc, callback );
}                               /* MFSL_lock */


fsal_status_t MFSL_changelock(fsal_lockdesc_t * lock_descriptor,        /* IN / OUT */
                              fsal_lockparam_t * lock_info      /* IN */
    )
{
  return FSAL_changelock(lock_descriptor, lock_info);
}                               /* MFSL_changelock */

fsal_status_t MFSL_unlock(mfsl_file_t * obj_handle,     /* IN */
                          fsal_lockdesc_t * ldesc       /*IN/OUT */
    )
{
  return FSAL_unlock(&obj_handle->fsal_file, ldesc);
}                               /* MFSL_unlock */

fsal_status_t MFSL_getlock(mfsl_file_t * obj_handle,    /* IN */
                           fsal_lockdesc_t * ldesc      /*IN/OUT */
    )
{
   return FSAL_getlock( &obj_handle->fsal_file, ldesc ) ;
}



#endif                          /* ! _USE_SWIG */
