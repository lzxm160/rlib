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

/**
 * This module contains rlib wrappers for container objects
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "rlib.h"


/**
 * Creates a hashtable that saves both the key and the value by VALUE
 *  vs. by reference
 */
rlib_hashtable_ptr rlib_hashtable_new_copyboth() {
	rlib_hashtable_ptr ht = g_new0(rlib_hashtable, 1);
	if (ht != NULL) {
		ht->ht = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
		ht->dup_keys = TRUE;
 		ht->dup_vals = TRUE;
	}
	return ht;		
}


rlib_hashtable_ptr rlib_hashtable_new() {
	rlib_hashtable_ptr ht = g_new0(rlib_hashtable, 1);
	if (ht != NULL) {
		ht->ht = g_hash_table_new(g_str_hash, g_str_equal);
	}
	return ht;		
}


void rlib_hashtable_destroy(rlib_hashtable_ptr ht) {
	if (ht && ht->ht) g_hash_table_destroy(ht->ht);
	else rlogit("Invalid hashtable in rlib_hashtable_destroy");
}


void rlib_hashtable_insert(rlib_hashtable_ptr ht, gpointer key, gpointer val) {
	if (ht->dup_keys) key = g_strdup(key);
	if (ht->dup_vals) val = g_strdup(val);
	if (ht && ht->ht) g_hash_table_insert(ht->ht, key, val);
	else rlogit("Invalid hashtable in rlib_hashtable_insert");
}


gpointer rlib_hashtable_lookup(rlib_hashtable_ptr ht, gconstpointer lookup) {
	if (ht && ht->ht) return g_hash_table_lookup(ht->ht, lookup);
	else rlogit("Invalid hashtable in rlib_hashtable_lookup");
	return NULL;
}


gboolean rlib_hashtable_dupskey(rlib_hashtable_ptr ht) {
	return ht->dup_keys;
}

gboolean rlib_hashtable_dupsval(rlib_hashtable_ptr ht) {
	return ht->dup_vals;
}
