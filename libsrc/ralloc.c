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

#include <glib.h>

#include "ralloc.h"


void *rmalloc(size_t size) {
	return g_malloc(size); 
}

gchar * rstrdup(const gchar *s) {
	return g_strdup(s);
}

gpointer rcalloc(size_t nmemb, size_t size) {
//TODO: this is not quite right but no good equivalent ... do something.
	return g_malloc0(nmemb * size);
}

void rfree(gpointer ptr) {
	g_free(ptr);
}

gpointer rrealloc(gpointer ptr, size_t size) {
	return g_realloc(ptr, size);
}

void ralloc_init_profiler() {
	g_mem_set_vtable(glib_mem_profiler_table);
}

int ralloc_getBadFrees() {
	return 0;
}

void ralloc_profile() {
	g_mem_profile();
}

