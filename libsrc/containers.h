/*
 *  Copyright (C) 2003 SICOM Systems, INC.
 *
 *  Authors: Bob Doan <bdoan@sicompos.com>
 *           Chet Heilman <cheilman@sicompos.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

struct rlib_hashtable {
	GHashTable *ht;
	gboolean dup_keys;
	gboolean dup_vals;
};
typedef struct rlib_hashtable rlib_hashtable;
typedef rlib_hashtable * rlib_hashtable_ptr;


rlib_hashtable_ptr rlib_hashtable_new_copyboth(void);
rlib_hashtable_ptr rlib_hashtable_new(void);
void rlib_hashtable_destroy(rlib_hashtable_ptr ht);
void rlib_hashtable_insert(rlib_hashtable_ptr ht, gpointer key, gpointer val);
gpointer rlib_hashtable_lookup(rlib_hashtable_ptr ht, gconstpointer lookup);
gboolean rlib_hashtable_dupskey(rlib_hashtable_ptr ht);
gboolean rlib_hashtable_dupsval(rlib_hashtable_ptr ht);
