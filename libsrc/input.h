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
 
#define INPUT(r) (r->input)

struct input_filter {
	void *private;
	int (*rlib_input_close)(void *);
	int (*rlib_fetch_row_from_result)(void *, int);
	int (*set_row_pointer)(void *, int, void *);
	void * (*get_row_pointer)(void *, int);
	void * (*fetch_row)(void *, int);
	void * (*get_last_row_pointer)(void *, int);
	int (*set_last_row_pointer)(void *, int, void *);
	void * (*rlib_input_connect)(void *, char *, char *, char *, char *);
	void (*query_and_set_result)(void *, int, char *);
	void (*set_query_result_name)(void *, int, char *);
	void * (*get_result_pointer)(void *, int);
	char * (*get_row_value)(void *, int, int);	
	char * (*get_resultset_name)(void *, int);	
	void (*seek_field)(void *, int, int);
	void * (*fetch_field)(void *, int);
};
