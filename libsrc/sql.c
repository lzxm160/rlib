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
#include <mysql.h>
 
#include "ralloc.h"
#include "input.h"

/*
	In case we have multiple servers later and we need to figure out which is the most avaialble one to hit up for a query
*/

#define RLIB_MAXIMUM_QUERIES	10

#define INPUT_PRIVATE(input) (((struct _private *)input->private))

struct rlib_results {
	MYSQL_RES *result;
	char *name;
	MYSQL_ROW row;
	MYSQL_ROW last_row;
};

struct _private {
	struct rlib_results results[RLIB_MAXIMUM_QUERIES]; 
	MYSQL *mysql;	
};


void * rlib_mysql_real_connect(void * woot, char *host, char *user, char *password, char *database) {
	struct input_filter *input = woot;
	MYSQL *mysql;
	mysql = rmalloc(sizeof(MYSQL));

	mysql_init(mysql);

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
	return 0;
}

MYSQL_RES * rlib_mysql_query(MYSQL *mysql, char *query) {
	MYSQL_RES *result = NULL;
	int rtn;
	
	rtn = mysql_query(mysql, query);
	if(rtn == 0) {
		result = mysql_store_result(mysql);
		return result;
	} 	
	return NULL;
}

static int rlib_mysql_fetch_row_from_result(void *woot, int i) {
	struct input_filter *input = woot;
	INPUT_PRIVATE(input)->results[i].row = mysql_fetch_row(INPUT_PRIVATE(input)->results[i].result);
	return 1;
}

static int mysql_set_row_pointer(void *woot, int i, void *data) {
	struct input_filter *input = woot;
	INPUT_PRIVATE(input)->results[i].row = data;
	return 1;
}

static void * mysql_get_row_pointer(void *woot, int i) {
	struct input_filter *input = woot;
	return INPUT_PRIVATE(input)->results[i].row;
}

static void * xxmysql_fetch_row(void *woot, int i) {
	struct input_filter *input = woot;
	MYSQL_ROW row;
	row = mysql_fetch_row(INPUT_PRIVATE(input)->results[i].result);
	return (void *)row;
}

static char * mysql_get_row_value(void *woot, int resultset, int i) {
	struct input_filter *input = woot;
	return INPUT_PRIVATE(input)->results[resultset].row[i];
}

static char * mysql_get_resultset_name(void *woot, int resultset) {
	struct input_filter *input = woot;
	return INPUT_PRIVATE(input)->results[resultset].name;
}

static void * mysql_get_last_row_pointer(void *woot, int i) {
	struct input_filter *input = woot;
	return INPUT_PRIVATE(input)->results[i].last_row;
}

void mysql_seek_field(void *woot, int i, int offset) {
	struct input_filter *input = woot;
	mysql_field_seek(INPUT_PRIVATE(input)->results[i].result, offset);
	return;
}

int mysql_set_last_row_pointer(void *woot, int i, void *set) {
	struct input_filter *input = woot;
	INPUT_PRIVATE(input)->results[i].last_row = set;
	return 1;
}

void mysql_query_and_set_result(void *woot, int i, char *query) {
	struct input_filter *input = woot;
	INPUT_PRIVATE(input)->results[i].result = rlib_mysql_query(INPUT_PRIVATE(input)->mysql, query);
	return;
}

void mysql_set_query_result_name(void *woot, int i, char *name) {
	struct input_filter *input = woot;
	INPUT_PRIVATE(input)->results[i].name = name;
	return;
}

static void * mysql_get_result_pointer(void *woot, int i) {
	struct input_filter *input = woot;
	return INPUT_PRIVATE(input)->results[i].result;
}

static void * xxmysql_fetch_field(void *woot, int i) {
	struct input_filter *input = woot;
	return mysql_fetch_field(INPUT_PRIVATE(input)->results[i].result);
}

static void * xxmysql_fetch_field_name(void *woot, void *xfield) {
	struct input_filter *input = woot;
	MYSQL_FIELD *field = xfield;
	return field->name;
}

static int rlib_mysql_free_input_filter(void *woot) {
	struct input_filter *input = woot;
	rfree(input->private);
	rfree(input);
debugf("************** SET ME FREE	***************\n");
}

void * rlib_mysql_new_input_filter() {
	struct input_filter *input;
	
	input = rmalloc(sizeof(struct input_filter));
	input->private = rmalloc(sizeof(struct _private));
	bzero(input->private, sizeof(struct _private));
	input->rlib_input_close = rlib_mysql_input_close;
	input->rlib_fetch_row_from_result = rlib_mysql_fetch_row_from_result;
	input->set_row_pointer = mysql_set_row_pointer;
	input->get_row_pointer = mysql_get_row_pointer;
	input->fetch_row = xxmysql_fetch_row;
	input->get_last_row_pointer = mysql_get_last_row_pointer;
	input->set_last_row_pointer = mysql_set_last_row_pointer;
	input->rlib_input_connect = rlib_mysql_real_connect;
	input->query_and_set_result = mysql_query_and_set_result;
	input->set_query_result_name = mysql_set_query_result_name;
	input->get_result_pointer = mysql_get_result_pointer;
	input->get_row_value = mysql_get_row_value;
	input->get_resultset_name = mysql_get_resultset_name;
	input->seek_field = mysql_seek_field;
	input->fetch_field = xxmysql_fetch_field;
	input->fetch_field_name = xxmysql_fetch_field_name;
	input->free = rlib_mysql_free_input_filter;
	return input;
}
