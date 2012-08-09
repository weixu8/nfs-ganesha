/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
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
 * \file    9p_hash_functions.c
 * \author  $Author: deniel $
 * \brief   Implements the hash functions for 9P FIDs
 *
 * 9p_hash_functions.c :  Implements the hash functions for 9P FIDs
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _SOLARIS
#include "solaris_port.h"
#endif

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include "HashData.h"
#include "HashTable.h"
#include "nfs_core.h"
#include "log.h"
#include "cache_inode.h"
#include "fsal.h"
#include "9p.h"

/* Hashtable used to cache the hostname, accessed by their IP addess */
hash_table_t *ht_9pfids;

/**
 *
 * idmapper_rbt_hash_func: computes the hash value for the entry in id mapper stuff
 *
 * Computes the hash value for the entry in id mapper stuff. In fact, it just use addresse as value (identity function) modulo the size of the hash.
 * This function is called internal in the HasTable_* function
 *
 * @param hparam [IN] hash table parameter.
 * @param buffcleff[in] pointer to the hash key buffer
 *
 * @return the computed hash value.
 *
 * @see HashTable_Init
 *
 */
uint32_t _9p_hash_func(hash_parameter_t * p_hparam,
                       hash_buffer_t * buffclef)
{
  _9p_hash_key_t * pkey = (_9p_hash_key_t * )buffclef->pdata ;

  return (unsigned long)((pkey->peer_addr+pkey->fid) % p_hparam->index_size);
}                               /*  ip_name_value_hash_func */


/**
 *
 * _9p_hash_func: computes the rbt value for the entry in the id mapper stuff.
 *
 * Computes the rbt value for the entry in the  stuff.
 *
 * @param hparam [IN] hash table parameter.
 * @param buffcleff[in] pointer to the hash key buffer
 *
 * @return the computed rbt value.
 *
 * @see HashTable_Init
 *
 */
uint64_t _9p_hash_rbt(hash_parameter_t * p_hparam,
                                hash_buffer_t * buffclef)
{
  _9p_hash_key_t * pkey = (_9p_hash_key_t * )buffclef->pdata ;
  uint32_t part1 = 0 ;
  uint32_t part2 = 0 ;
  uint64_t result = 0LL ;
 
  part1 = pkey->peer_addr ;
  part2 = ( pkey->peer_port << 4 ) + pkey->fid ;
  result = (part1 << 8 ) + part2 ;

  return result;
}          


/**
 *
 * compare_idmapper: compares the values stored in the key buffers.
 *
 * Compares the values stored in the key buffers. This function is to be used as 'compare_key' field in
 * the hashtable storing the nfs duplicated requests.
 *
 * @param buff1 [IN] first key
 * @param buff2 [IN] second key
 *
 * @return 0 if keys are identifical, 1 if they are different.
 *
 */

int _9p_compare_hash_key(hash_buffer_t * buff1, hash_buffer_t * buff2)
{
  _9p_hash_key_t * pkey1 = (_9p_hash_key_t * )buff1->pdata ;
  _9p_hash_key_t * pkey2 = (_9p_hash_key_t * )buff2->pdata ;
 
  return ( (pkey1->peer_addr == pkey2->peer_addr ) &&
           (pkey1->peer_port == pkey2->peer_port ) &&
           (pkey1->fid == pkey2->fid ) ) ? 0 : 1 ;
}   

/**
 *
 * _9p_display_hash_key: displays the entry key stored in the buffer.
 *
 * Displays the entry key stored in the buffer. This function is to be used as 'val_to_str' field in
 * the hashtable storing the id mapper stuff
 *
 * @param buff1 [IN]  buffer to display
 * @param buff2 [OUT] output string
 *
 * @return number of character written.
 *
 */
int _9p_display_hash_key(hash_buffer_t * pbuff, char *str)
{
  _9p_hash_key_t * pkey = (_9p_hash_key_t * )pbuff->pdata ;
  return sprintf( str, "0x%x=%d.%d.%d.%d:%d",
                  pkey->peer_addr,
                  (pkey->peer_addr & 0xFF000000) >> 24,
                  (pkey->peer_addr & 0x00FF0000) >> 16,
                  (pkey->peer_addr & 0x0000FF00) >> 8,
                  pkey->peer_addr & 0x000000FF,
                  pkey->peer_port );

  return sprintf(str, "%s", (char *)(pbuff->pdata));
}

/**
 *
 * _9p_display_hash_val: displays the entry val stored in the buffer.
 *
 * Displays the entry val stored in the buffer. This function is to be used as 'val_to_str' field in
 * the hashtable storing the id mapper stuff
 *
 * @param buff1 [IN]  buffer to display
 * @param buff2 [OUT] output string
 *
 * @return number of character written.
 *
 */
int _9p_display_hash_val(hash_buffer_t * pbuff, char *str)
{
  _9p_fid_t * pfid = (_9p_fid_t *) pbuff->pdata ;
  return sprintf(str, "FID:%u",  (unsigned int)pfid->fid ) ;
}                         

/**
 *
 * _9p_hash_fid_init: Inits the hashtable for UID mapping.
 *
 * Inits the hashtable for UID mapping.
 *
 * @param param [IN] parameter used to init the uid map cache
 *
 * @return 0 if successful, -1 otherwise
 *
 */
int _9p_hash_fid_init( _9p_parameter_t param)
{
  if((ht_9pfids = HashTable_Init(&param._9p_hash_param)) == NULL)
    {
      LogCrit(COMPONENT_9P,
              "9P_INIT: Cannot init 9P FIDs cache");
      return -1;
    }

  return 0 ;
}                               /* _9p_hash_fid__init */


int _9p_hash_fid_get( _9p_conn_t * pconn, u32 fid, _9p_fid_t **  ppfid )
{
  hash_buffer_t buffkey;
  hash_buffer_t buffval;
  _9p_hash_key_t key  ;

  key.peer_addr = pconn->peer_addr ;
  key.peer_port = pconn->peer_port ;
  key.fid = fid ;

  if(HashTable_Get(ht_9pfids, &buffkey, &buffval) != HASHTABLE_SUCCESS)
    return -1 ;

  *ppfid = (_9p_fid_t *)buffval.pdata ;

  return 0 ;
}

int _9p_hash_fid_del( _9p_conn_t * pconn, u32 fid )
{
  hash_buffer_t buffkey;
  _9p_hash_key_t key  ;

  key.peer_addr = pconn->peer_addr ;
  key.peer_port = pconn->peer_port ;
  key.fid = fid ;

  buffkey.pdata = (char *)&key ;
  buffkey.len = sizeof( _9p_hash_key_t ) ;

  if( HashTable_Del( ht_9pfids, &buffkey, NULL, NULL ) != HASHTABLE_SUCCESS )
   return -1 ;

  return 0 ;
}

int _9p_hash_fid_set( _9p_conn_t * pconn, u32 fid, _9p_fid_t * pfid )
{
  hash_buffer_t buffkey;
  hash_buffer_t buffdata;
  _9p_hash_key_t * pkey = NULL ;

  if( ( pkey  = gsh_malloc( sizeof( _9p_hash_key_t ) ) ) == NULL )
    return -1 ;

  pkey->peer_addr = pconn->peer_addr ;
  pkey->peer_port = pconn->peer_port ;
  pkey->fid = fid ;

  buffkey.pdata = (char *)pkey ;
  buffkey.len = sizeof( _9p_hash_key_t ) ;

  buffdata.pdata = (char *)pfid ;
  buffdata.len = sizeof( _9p_fid_t ) ;

  if( HashTable_Test_And_Set(ht_9pfids, &buffkey, &buffdata,
                              HASHTABLE_SET_HOW_SET_OVERWRITE ) != HASHTABLE_SUCCESS )
    return -1 ;

  return 0 ;
}


