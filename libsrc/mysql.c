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
#include <stdio.h>
#include <string.h>
#include <mysql.h>
 
#include "ralloc.h"
#include "input.h"

/*
	In case we have multiple servers later and we need to figure out which is the most avaialble one to hit up for a query
*/

#define RLIB_MAXIMUM_QUERIES	10

#define INPUT_PRIVATE(input) (((struct _private *)input->private))

struct rlib_mysql_results {
	MYSQL_RES *result;
	char *name;
	MYSQL_ROW this_row;
	MYSQL_ROW previous_row;
	MYSQL_ROW save_row;
	MYSQL_ROW last_row;
	int didprevious;
	int *fields;
};

struct _private {
	MYSQL *mysql;
};

void * rlib_mysql_real_connect(void * woot, char *host, char *user, char *password, char *database) {
	struct input_filter *input = woot;
	MYSQL *mysql;

	mysql = mysql_init(NULL);

	if (mysql_real_connect(mysql,host,user,password, database, 0, NULL, 0) == NULL) {
		return NULL;
	}
		
	if (mysql_select_db(mysql,database)) {
		return NULL;
	}

	INPUT_PRIVATE(input)->mysql = mysql;	
	return mysql;
}

static int rlib_mysql_input_close(void *woot) {
	struct input_filter *input = woot;
	mysql_close(INPUT_PRIVATE(input)->mysql);
	INPUT_PRIVATE(input)->mysql = NULL;
	
	return 0;
}

static MYSQL_RES * rlib_mysql_query(MYSQL *mysql, char *query) {
	MYSQL_RES *result = NULL;
	int rtn;
	rtn = mysql_query(mysql, query);
	if(rtn == 0) {
		result = mysql_store_result(mysql);
		return result;
	} 	
	return NULL;
}

static int rlib_mysql_first(void *input_ptr, void *result_ptr) {
	struct rlib_mysql_results *result = result_ptr;
	result->this_row = mysql_fetch_row(result->result);
	result->previous_row = NULL;
	result->last_row = NULL;
	result->didprevious = FALSE;
	return TRUE;
}

static int rlib_mysql_next(void *input_ptr, void *result_ptr) {
	struct rlib_mysql_results *result = result_ptr;
	MYSQL_ROW row;
	if(result->didprevious == TRUE) {
		result->didprevious = FALSE;
		result->this_row = result->save_row;
		return TRUE;
	} else {
		row = mysql_fetch_row(result->result);
		if(row != NULL) {
			result->previous_row = result->this_row;
			result->this_row = row;
			return TRUE;
		} else {
			result->previous_row = result->this_row;
			result->last_row = result->this_row;
			result->this_row = NULL;
			return FALSE;	
		}
	}
}

static int rlib_mysql_isdone(void *input_ptr, void *result_ptr) {
	struct rlib_mysql_results *result = result_ptr;
	if(result->this_row == NULL)
		return TRUE;
	else
		return FALSE;
}

static int rlib_mysql_previous(void *input_ptr, void *result_ptr) {
	struct rlib_mysql_results *result = result_ptr;
	result->save_row = result->save_row;
	result->this_row = result->previous_row;
	if(result->previous_row == NULL) {
		result->didprevious = FALSE;
		return FALSE;
	} else {
		result->didprevious = TRUE;
		return TRUE;
	}
}

static int rlib_mysql_last(void *input_ptr, void *result_ptr) {
	struct rlib_mysql_results *result = result_ptr;
	result->this_row = result->last_row;
	if(result->last_row == NULL)
		return FALSE;
	else
		return TRUE;
}

static char * rlib_mysql_get_field_value_as_string(void *input_ptr, void *result_ptr, void *field_ptr) {
	struct rlib_mysql_results *result = result_ptr;
	long field = (long)field_ptr;
	return result->this_row[field];
}

static void * rlib_mysql_resolve_field_pointer(void *input_ptr, void *result_ptr, char *name) {
	struct rlib_mysql_results *results = result_ptr;
	int x=0;
	MYSQL_FIELD *field;
	mysql_field_seek(results->result, 0);
	
	while((field = mysql_fetch_field(results->result))) {
		if(!strcmp(field->name, name)) {
			return (void *)results->fields[x];
		}
		x++;
	}
	return NULL;
}

void * mysql_new_result_from_query(void *input_ptr, char *query) {
	struct input_filter *input = input_ptr;
	struct rlib_mysql_results *results;
	MYSQL_RES *result;
	unsigned int count,i;
	result = rlib_mysql_query(INPUT_PRIVATE(input)->mysql, query);
	if(result == NULL)
		return NULL;
	else {
		results = rmalloc(sizeof(struct rlib_mysql_results));
		results->result = result;
	}
	count = mysql_field_count(INPUT_PRIVATE(input)->mysql);
	results->fields = rmalloc(sizeof(int) * count);
	for(i=0;i<count;i++) {
		results->fields[i] = i;
	}
	return results;
}

static void rlib_mysql_rlib_free_result(void *input_ptr, void *result_ptr) {
	struct rlib_mysql_results *result = result_ptr;
	mysql_free_result(result->result);
	rfree(result->fields);
}

static int rlib_mysql_free_input_filter(void *woot) {
	struct input_filter *input = woot;
	rfree(input->private);
	rfree(input);
	return 0;
}

void * rlib_mysql_new_input_filter() {
	struct input_filter *input;
	
	input = rmalloc(sizeof(struct input_filter));
	input->private = rmalloc(sizeof(struct _private));
	bzero(input->private, sizeof(struct _private));
	input->input_close = rlib_mysql_input_close;
	input->first = rlib_mysql_first;
	input->next = rlib_mysql_next;
	input->previous = rlib_mysql_previous;
	input->last = rlib_mysql_last;
	input->isdone = rlib_mysql_isdone;
	input->input_connect = rlib_mysql_real_connect;
	input->new_result_from_query = mysql_new_result_from_query;
	input->get_field_value_as_string = rlib_mysql_get_field_value_as_string;

	input->resolve_field_pointer = rlib_mysql_resolve_field_pointer;

	input->free = rlib_mysql_free_input_filter;
	input->rlib_free_result = rlib_mysql_rlib_free_result;
	return input;
}
