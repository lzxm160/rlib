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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NO	0
#define YES (!NO)


#include "rvector.h"
#include "ralloc.h"

static void error(char *msg) {
}


static int comp(const void *x, const void *y) {
	return *(int *) x - *(int *) y;
}


/**
 * returns a new Vector of the specified initial size and increment.
 */
RVector *RVector_newOpt(int initsize, int incr, int (*compar)(), int noralloc) {
	RVector *v;
	v = (noralloc)? (RVector *) calloc(1, sizeof(RVector))
					: (RVector *) rcalloc(1, sizeof(RVector));
	if (v != NULL) {
		v->noralloc = noralloc;
		v->data = (noralloc)? (void **) calloc(initsize, sizeof(void *))
							: (void **) rcalloc(initsize, sizeof(void *));
		v->maxentries = initsize;
		v->nentries = 0;
		v->incr = incr;
		v->comp = compar;
	} else {
		error("Cannot allocate rvector"); 
	}
	return v;
}


/**
 * returns a new Vector of the default initial and incremental size.
 * Using the default compare (integer)
 * memory is allocated directly from the O/S vs. ralloc
 */
RVector *RVector_newSpecial() {
	return RVector_newOpt(512, 512, comp, 1);
}


/**
 * returns a new Vector of the default initial and incremental size.
 * Using the default compare (integer)
 */
RVector *RVector_new() {
	return RVector_newOpt(512, 512, comp, 0);
}


/**
 * Adds the passed object to the Vector. The Vector is grown as needed to 
 * accomodate an indefinite number of elements.
 */
void RVector_add(RVector *v, void *object) {
	int newsize = v->maxentries + v->incr;
	
	if (v->nentries >= v->maxentries) {
		void **t = (v->noralloc)? (void **) realloc(v->data, newsize * sizeof(void*))
								: (void **) rrealloc(v->data, newsize * sizeof(void*));
		if (t != NULL) {
			v->data = t;
			v->maxentries = newsize;
		} else {
			error("Cannot realloc vector");
		}
	}
	v->data[v->nentries++] = object;		
	v->isSorted = NO;
}



/**
 * Releases all resources.
 */
void RVector_free(RVector *v) {
	if (v->noralloc) {
		if (v->data) free(v->data);
		free(v);
	} else {
		if (v->data) rfree(v->data);
		rfree(v);
	}
}


inline static int isValidIdx(int idx, int max) {
	return ((idx >= 0) && (idx < max));
}


/**
 * Returns the element at the specified index in the array
 */
void *RVector_get(RVector *v, int idx) {
	if (isValidIdx(idx, v->nentries)) return v->data[idx];
	return NULL;
}


/**
 * returns the number of entries actively in the array
 */
int RVector_size(RVector *v) {
	return v->nentries;
}


void RVector_setCompareFunc(RVector *v, int (*comp)()) {
	v->comp = comp;
}


/**
 * Sorts the Vector into order based on the 'compar' function.
 */
void RVector_sort(RVector *v) {
	qsort(v->data, v->nentries, sizeof(void *), v->comp);
	v->isSorted = YES;
}


/**
 * Performs a Binary search on the Vector v, using the comparison algorithm
 * 'keycompare' looking for key.
 * Returns the index of the sought element or -1 if not found.
 */
int RVector_bsearch(RVector *v, const void *key) {
	void **bresult = (void **) bsearch(&key, v->data, RVector_size(v), sizeof(void *), v->comp);
	if (bresult != NULL) return bresult - v->data;
	return -1;
}


/**
 * Find using a linear search, using the current comparison algorithm
 * Returns the index of the sought element or -1 if not found.
 */
int RVector_find(RVector *v, const void *key) {
	int i, lim;
	
	for (i = 0, lim = v->nentries; i < lim; ++i) {
		if (!v->comp(&v->data[i], &key)) return i;
	}
	return -1;
}

/**
 * returns TRUE if the vector is in a sorted order or false if something
 * has been added since the last sort.
 */
int RVector_isSorted(RVector *v) {
	return v->isSorted;
}


/**
 * Deletes the indexed entry in vector.
 * returns the value of the entry if successful, else NULL
 */
void *RVector_deleteAt(RVector *v, int idx) {
	void *result = NULL;
	
	if (isValidIdx(idx, v->nentries)) {
		result = v->data[idx];
		memcpy(&v->data[idx], &v->data[idx + 1], (v->nentries - idx) * sizeof(void*));
		--v->nentries;
	}
	return result;
}


/**
 * Returns the Vectors data pointer array. 
 * Do not use this function unless there is a really good reason.
 * Maybe the new functionality should be added to the Vector class.
 */
void **RVector_getArray(RVector *v) {
	return v->data;
}



/**
 * A 'class' to iterate a vector
 */
/**
 * Creates a new VectorIterator to iterate the passed Vector
 */
RVectorIterator *RVectorIterator_new(RVector *v) {
	RVectorIterator *vi = (RVectorIterator *) rcalloc(1, sizeof(RVectorIterator));
	if (vi != NULL) {
		vi->v = v;
		vi->curidx = 0;
		vi->maxidx = RVector_size(v);
	}
	return vi;
}


/**
 * returns TRUE if the iterator has another entry. i.e. a call to 
 * VectorIterator_next WILL SUCCEED.
 */
int RVectorIterator_hasNext(RVectorIterator *vi) {
	return vi->curidx < vi->maxidx;
}


/**
 * returns the next element of the Vector or NULL if there is none.
 */
void *RVectorIterator_next(RVectorIterator *vi) {
	return (RVectorIterator_hasNext(vi))?
				RVector_get(vi->v, vi->curidx++)
				: NULL;
}


/**
 * Releases all resources.
 */
void RVectorIterator_free(RVectorIterator *vi) {
	if (vi != NULL) rfree(vi);
}


