/*
 * $Id$
 */
#ifndef RVECTOR_H
#define RVECTOR_H

#include <glib.h>


struct _rvector {
	void **data;
	gint maxentries;
	gint nentries;
	gint incr;
	gint (*comp)();
	gint noralloc;
	gint isSorted;
};
typedef struct _rvector RVector;


struct _rvector_iter {
	RVector *v;
	gint curidx;
	gint maxidx;
};
typedef struct _rvector_iter RVectorIterator;


RVector *RVector_newOpt(gint initsize, gint incr, gint (*comp)(), gint noralloc);
RVector *RVector_new(void);
RVector *RVector_newSpecial(void);
void RVector_add(RVector *v, gpointer object);
void RVector_free(RVector *v);
gpointer RVector_get(RVector *v, gint idx);
gint RVector_size(RVector *v);
void RVector_sort(RVector *v);
gint RVector_bsearch(RVector *v, const gpointer key);
gint RVector_find(RVector *v, const gpointer key);
gint RVector_isSorted(RVector *v);
void RVector_setCompareFunc(RVector *v, gint (*comp)());
gpointer RVector_deleteAt(RVector *v, gint idx);


RVectorIterator *RVectorIterator_new(RVector *v);
gint RVectorIterator_hasNext(RVectorIterator *vi);
gpointer RVectorIterator_next(RVectorIterator *vi);
void RVectorIterator_free(RVectorIterator *vi);

#endif
