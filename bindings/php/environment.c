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
 
#include <php.h>

#include "ralloc.h"
#include "rlib.h"

static char * rlib_php_resolve_memory_variable(char *name) {
	zval **data; 
	if (zend_hash_find(&EG(symbol_table),name,strlen(name),(void **)&data)==FAILURE) { 
		return NULL;
	} else {
		return Z_STRVAL_PP(data);
	}
}

static int rlib_php_write_output(char *data, int len) {
	return php_write(data, len TSRMLS_CC);
}

void rlib_php_free(rlib *r) {
	rfree(ENVIRONMENT(r));
}


struct environment_filter * rlib_php_new_environment() {
	struct environment_filter *ef;
	ef = emalloc(sizeof(struct environment_filter));

	ef->rlib_resolve_memory_variable = rlib_php_resolve_memory_variable;
	ef->rlib_write_output = rlib_php_write_output;
	ef->free = rlib_php_free;
}
