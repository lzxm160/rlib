/*
 *  Copyright (C) 2003 SICOM Systems, INC.
 *
 *  Authors: Bob Doan <bdoan@sicompos.com>, Chet Heilman <cheilman@sicompos.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rvector.h"
#include "rhashtable.h"


#include <rlib.h>


/**
 * Default search and sort functions.
 * inputs are pointers to the data or datastruct pointer.
 */
static int keycasecompare(const struct _ht_tuple **key, const struct _ht_tuple **htp) {
	return strcasecmp((*key)->key, (*htp)->key);
}


static int keycompare(const struct _ht_tuple **key, const struct _ht_tuple **htp) {
	const char *k1 = (*key)->key;
	const char *k2 = (*htp)->key;
//printf("k1=%s, k2=%s\n", k1, k2);
	return strcmp(k1, k2);
}


/**
 * Releases all memory allocated by the RHashtable object
 * This ht pointer should never be used again as it is now invalid;
 * Set the (RHashtable *) variable to NULL after calling.
 */
void RHashtable_free(RHashtable *ht) {
	if (ht != NULL) {
		RVectorIterator *vi = RVectorIterator_new(ht->data);
		if (vi != NULL) {
			while (RVectorIterator_hasNext(vi)) {
				rfree(RVectorIterator_next(vi));
			}
			RVectorIterator_free(vi);
		}
		RVector_free(ht->data);
		rfree(ht);
	}
}


/**
 * Returns a new hashtable with default sizes
 * A default hashtable has case SENSITIVE keys
 * A default hashtable stores value pointers, not values.
 */
RHashtable *RHashtable_new() {
	RHashtable *ht = (RHashtable *) rcalloc(1, sizeof(RHashtable));
	if (ht != NULL) {
		ht->data = RVector_new();
	}
	if (ht->data == NULL) {
		RHashtable_free(ht);
		ht = NULL;
	} else 
		RVector_setCompareFunc(ht->data, keycompare);
	return ht;
}


/**
 * Adds the key:value pair to the hashtable
 * returns 0 if OK, else non-0.
 * A duplicate key replaces the old value for that key with the new
 */
int RHashtable_put(RHashtable *ht, const char *key, const void *value) {
	struct _ht_tuple htt;
	int result = 1;
	struct _ht_tuple *t;
	RVector *v = ht->data;
	int idx;

	memset(&htt, 0, sizeof(htt));
	htt.key = key;
	idx = RVector_bsearch(v, &htt);
	if (idx >= 0) {
		t = RVector_get(v, idx);
		t->value = (void *) value;
		result = 0;
	} else {
		if (ht->storevalues)
			t = (struct _ht_tuple *) rcalloc(1, sizeof(struct _ht_tuple) + strlen(key) + strlen(value) + 1);
		else
			t = (struct _ht_tuple *) rcalloc(1, sizeof(struct _ht_tuple) + strlen(key));
		if (t != NULL) {
			strcpy(&t->keybuf, key);
			t->key = &t->keybuf;
			if (ht->storevalues) {
				t->value = &t->keybuf + strlen(key) + 1;
				strcpy(t->value, (char *) value);
			} else {
				t->value = (void *) value;
			}
			RVector_add(ht->data, t);
			result = 0;
		}
	}
	if (!RVector_isSorted(v)) RVector_sort(v);
	return result;
}


/**
 * returns the element associated with the key or NULL if not found;
 * if storevalues is true, the element is a CLONE of the element added,
 * not the actual same element.
 */
void *RHashtable_get(RHashtable *ht, const char *key) {
	struct _ht_tuple htt;
	RVector *v = ht->data;
	int idx;
	struct _ht_tuple *t;

	memset(&htt, 0, sizeof(htt));
	htt.key = key;
	idx = RVector_bsearch(v, &htt);
	if (idx >= 0) {
		t = RVector_get(v, idx);
		return t->value;
	}
	return NULL;
}


/**
 * Sets whether keys are case insensitive or not - default with _new is
 * CASE SENSITIVE keys.
 */
void RHashtable_setCaseInsensitive(RHashtable *ht, int yesorno) {
	if (yesorno)
		RVector_setCompareFunc(ht->data, keycasecompare);
	else
		RVector_setCompareFunc(ht->data, keycompare);
}


/**
 * If storevalues is true, both key and value data is stored in the hashtable.
 * if false, the value is stored as a POINTER - the key is still stored.
 */ 
int RHashtable_setStoreValues(RHashtable *ht, int yesorno) {
	int result = 1;
	
	if (RVector_size(ht->data) == 0) {
		ht->storevalues = yesorno;
		result = 0;
	}		
	return result;
}


/**
 * returns a RVector containing an array of name-value pairs (struct _ht_tuple *).
 * The vector is sorted according to the CompareFunc. To search the RVector, you
 * must pass a struct _ht_tuple ** so use RHashtable_get to do the same thing bue
 * get everything right for you.
 */
RVector *RHashtable_getDataRVector(RHashtable *ht) {
	return ht->data;
}

