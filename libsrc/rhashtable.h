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

#ifndef RHASHTABLE_H
#define RHASHTABLE_H

#include "rvector.h"


struct _ht_tuple {
	const char *key;
	char *value;
	char keybuf;
};


struct rhashtable {
	RVector *data;
	int storevalues;
	int (* keycompare)(const struct _ht_tuple **key, const struct _ht_tuple **ht);
};
typedef struct rhashtable RHashtable;

void RHashtable_free(RHashtable *ht);
RHashtable *RHashtable_new();
int RHashtable_put(RHashtable *ht, const char *key, const void *value);
void *RHashtable_get(RHashtable *ht, const char *key);
void RHashtable_setCaseInsensitive(RHashtable *ht, int yesorno);
int RHashtable_setStoreValues(RHashtable *ht, int yesorno);

#endif
