/*
 *  Copyright (C) 2003 SICOM Systems, INC.
 *
 *  Authors: Bob Doan <bdoan@sicompos.com>
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
 */
 
#include <stdlib.h>
#include <string.h>

#include "ralloc.h"

#if DEBUG_RLIB_MEMORY
static RVector *mem;
static int nBadFrees;
RVector *ralloc_getVector() {
	return mem;
}


int ralloc_getBadFrees() {
	return nBadFrees;
}


void ralloc_init() {
	if (mem) RVector_free(mem);
	mem = RVector_newSpecial();
	nBadFrees = 0;
}


void *rmalloc(size_t size) {
	void *p = malloc(size);
	if (mem && p) RVector_add(mem, p);
	return p; 
}

char *rstrdup(const char *s) {
	void *p = strdup(s);
	if (mem && p) RVector_add(mem, p);
	return p; 
}

void *rcalloc(size_t nmemb, size_t size) {
	void *p = calloc(nmemb, size);
	if (mem && p) RVector_add(mem, p);
	return p; 
}

void rfree(void *ptr) {
	if (mem) {
		int idx;
		idx = RVector_find(mem, ptr);
		if (idx >= 0) RVector_deleteAt(mem, idx);
		else ++nBadFrees;
	}
	free(ptr);
}

void *rrealloc(void *ptr, size_t size) {
	void *p = realloc(ptr, size);
	if (mem) {
		int idx;
		idx = RVector_find(mem, ptr);
		if (idx >= 0) RVector_deleteAt(mem, idx);
		else ++nBadFrees;
		if (p) RVector_add(mem, p);
	}
	return p;
}
#else
void *rmalloc(size_t size) {
	return malloc(size); 
}

char * rstrdup(const char *s) {
	return strdup(s);
}

void *rcalloc(size_t nmemb, size_t size) {
	return calloc(nmemb, size);
}

void rfree(void *ptr) {
	free(ptr);
}

void *rrealloc(void *ptr, size_t size) {
	return realloc(ptr, size);
}

RVector *ralloc_getVector() {
	return NULL;
}

void ralloc_init() {
}

int ralloc_getBadFrees() {
	return 0;
}
#endif
