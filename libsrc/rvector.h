/*
 * $Id$
 */
#ifndef RVECTOR_H
#define RVECTOR_H


struct _rvector {
	void **data;
	int maxentries;
	int nentries;
	int incr;
	int (*comp)();
	int noralloc;
	int isSorted;
};
typedef struct _rvector RVector;


struct _rvector_iter {
	RVector *v;
	int curidx;
	int maxidx;
};
typedef struct _rvector_iter RVectorIterator;


RVector *RVector_newOpt(int initsize, int incr, int (*comp)(), int noralloc);
RVector *RVector_new(void);
RVector *RVector_newSpecial(void);
void RVector_add(RVector *v, void *object);
void RVector_free(RVector *v);
void *RVector_get(RVector *v, int idx);
int RVector_size(RVector *v);
void RVector_sort(RVector *v);
int RVector_bsearch(RVector *v, const void *key);
int RVector_find(RVector *v, const void *key);
int RVector_isSorted(RVector *v);
void RVector_setCompareFunc(RVector *v, int (*comp)());
void *RVector_deleteAt(RVector *v, int idx);


RVectorIterator *RVectorIterator_new(RVector *v);
int RVectorIterator_hasNext(RVectorIterator *vi);
void *RVectorIterator_next(RVectorIterator *vi);
void RVectorIterator_free(RVectorIterator *vi);

#endif
