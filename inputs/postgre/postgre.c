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

#include <glib.h>

#include "libpq-fe.h"
#include "rlib_input.h"

#define INPUT_PRIVATE(input) (((struct _private *)input->private))

struct rlib_postgre_results {
	PGresult *result;
	gchar *name;
	gint row;
	gint tot_rows;
	gint tot_fields;
	gint isdone;
	gint last_action;
	gint *fields;
};

struct _private {
	PGconn *conn;
};

gpointer rlib_postgre_connect(gpointer input_ptr, gchar *conninfo) {
	struct input_filter *input = input_ptr;
	PGconn *conn;

	conn = PQconnectdb(conninfo);
	if (PQstatus(conn) != CONNECTION_OK) {
		PQfinish(conn);
		conn = NULL;
	}

	INPUT_PRIVATE(input)->conn = conn;	
	return conn;
}

static gint rlib_postgre_input_close(gpointer input_ptr) {
	struct input_filter *input = input_ptr;
	PQfinish(INPUT_PRIVATE(input)->conn);
	INPUT_PRIVATE(input)->conn = NULL;
	return 0;
}

static PGresult * rlib_postgre_query(PGconn *conn, gchar *query) {
	PGresult *result = NULL;
	result = PQexec(conn, query);

	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		PQclear(result);
		PQfinish(conn);
	}
	return result;
}

static gint rlib_postgre_first(gpointer input_ptr, gpointer result_ptr) {
	struct rlib_postgre_results *result = result_ptr;
	result->row = 0;
	result->isdone = FALSE;
	return result != NULL ? TRUE : FALSE;
}

static gint rlib_postgre_next(gpointer input_ptr, gpointer result_ptr) {
	struct rlib_postgre_results *results = result_ptr;
	if(results->row+1 < results->tot_rows) {
		results->row++;
		results->isdone = FALSE;
		return TRUE;
	}
	results->isdone = TRUE;
	return FALSE;
}

static gint rlib_postgre_isdone(gpointer input_ptr, gpointer result_ptr) {
	struct rlib_postgre_results *result = result_ptr;
	return result->isdone;
}

static gint rlib_postgre_previous(gpointer input_ptr, gpointer result_ptr) {
	struct rlib_postgre_results *result = result_ptr;
	if(result->row-1 > 0) {
		result->row--;
		result->isdone = FALSE;
		return TRUE;
	}
	result->isdone = TRUE;
	return FALSE;
}

static gint rlib_postgre_last(gpointer input_ptr, gpointer result_ptr) {
	struct rlib_postgre_results *result = result_ptr;
	result->row = result->tot_rows-1;
	return TRUE;
}

static gchar * rlib_postgre_get_field_value_as_string(gpointer input_ptr, gpointer result_ptr, gpointer field_ptr) {
	struct rlib_postgre_results *results = result_ptr;
	gint field = (gint)field_ptr;
	field -= 1;
	return PQgetvalue(results->result, results->row, field);
}

static gpointer rlib_postgre_resolve_field_pointer(gpointer input_ptr, gpointer result_ptr, gchar *name) {
	struct rlib_postgre_results *results = result_ptr;
	gint i=0;
	for (i = 0; i < results->tot_fields; i++)
		if(!strcmp(PQfname(results->result, i), name)) {
			return (gpointer)results->fields[i];
		}
	return NULL;
}

gpointer postgre_new_result_from_query(gpointer input_ptr, gchar *query) {
	struct input_filter *input = input_ptr;
	struct rlib_postgre_results *results;
	PGresult *result;
	guint count,i;
	
	if(input_ptr == NULL)
		return NULL;
	
	result = rlib_postgre_query(INPUT_PRIVATE(input)->conn, query);
	if(result == NULL)
		return NULL;
	else {
		results = g_malloc(sizeof(struct rlib_postgre_results));
		results->result = result;
	}
	count = PQnfields(result);
	results->fields = g_malloc(sizeof(int) * count);
	for(i=0;i<count;i++) {
		results->fields[i] = i+1;
	}
	results->tot_fields = count;
	results->tot_rows = PQntuples(result);
	return results;
}

static void rlib_postgre_rlib_free_result(gpointer input_ptr, gpointer result_ptr) {
	struct rlib_postgre_results *results = result_ptr;
	PQclear(results->result);
	g_free(results->fields);
	g_free(results);
}

static gint rlib_postgre_free_input_filter(gpointer input_ptr) {
	struct input_filter *input = input_ptr;
	g_free(input->private);
	g_free(input);
	return 0;
}

gpointer rlib_postgre_new_input_filter() {
	struct input_filter *input;
	
	input = g_malloc(sizeof(struct input_filter));
	input->private = g_malloc(sizeof(struct _private));
	memset(input->private, 0, sizeof(struct _private));
	input->input_close = rlib_postgre_input_close;
	input->first = rlib_postgre_first;
	input->next = rlib_postgre_next;
	input->previous = rlib_postgre_previous;
	input->last = rlib_postgre_last;
	input->isdone = rlib_postgre_isdone;
	input->new_result_from_query = postgre_new_result_from_query;
	input->get_field_value_as_string = rlib_postgre_get_field_value_as_string;

	input->resolve_field_pointer = rlib_postgre_resolve_field_pointer;

	input->free = rlib_postgre_free_input_filter;
	input->free_result = rlib_postgre_rlib_free_result;
	return input;
}
