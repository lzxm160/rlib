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
 
#define TRUE	1
#define FALSE	0

struct input_filter {
	void *private;
	int (*input_close)(void *);
	void * (*new_result_from_query)(void *, char *);
	int (*free)(void *);
	
	int (*first)(void *, void *);
	int (*next)(void *, void *);
	int (*previous)(void *, void *);
	int (*last)(void *, void *);
	int (*isdone)(void *, void *);

	char * (*get_field_value_as_string)(void *, void *, void *);	

	void * (*resolve_field_pointer)(void *, void *, char *);

	void (*free_result)(void *, void *);	
};
