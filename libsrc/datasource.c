/*
 *  Copyright (C) 2003-2005 SICOM Systems, INC.
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

#include <config.h>
#include <string.h>
#include <gmodule.h>

#include "config.h"
#include "rlib.h"
#include "rlib_input.h"

typedef struct {
	union {
		gpointer ptr;
		gpointer (*new_input_filter)(void);
	} filter;
	union {
		gpointer ptr;
		gpointer (*mysql_connect)(gpointer, const gchar *, const gchar *, const gchar *, const gchar*, const gchar *);
		gpointer (*postgre_connect)(gpointer, const gchar *);
		gpointer (*odbc_connect)(gpointer, const gchar *, const gchar *, const gchar *);
	} connect;
} datasource_t;

gint rlib_add_datasource(rlib *r, const gchar *input_name, struct input_filter *input) {
	r->inputs[r->inputs_count].input = input;
	r->inputs[r->inputs_count].name = g_strdup(input_name);
	r->inputs[r->inputs_count].handle = NULL;
	r->inputs[r->inputs_count].input->info.encoder = NULL;
	r->inputs_count++;
	return 0;
}

static gint rlib_add_datasource_mysql_private(rlib *r, const gchar *input_name, const gchar *database_group, const gchar *database_host, 
const gchar *database_user, const gchar *database_password, const gchar *database_database) {
	GModule* handle;
	datasource_t	ds;
	gpointer mysql;

	handle = g_module_open("libr-mysql", 2);
	if (!handle) {
		rlogit("Could Not Load MYSQL Input [%s]\n", g_module_error());
		return -1;
	}

	g_module_symbol(handle, "rlib_mysql_new_input_filter", &ds.filter.ptr);
	g_module_symbol(handle, "rlib_mysql_real_connect", &ds.connect.ptr);
	r->inputs[r->inputs_count].input = ds.filter.new_input_filter();
	mysql = ds.connect.mysql_connect(r->inputs[r->inputs_count].input, database_group, database_host, database_user, 
		database_password, database_database);

	if(mysql == NULL) {
		rlogit("ERROR: Could not connect to MYSQL\n");
		return -1;
	}
	
	r->inputs[r->inputs_count].name = g_strdup(input_name);
	r->inputs[r->inputs_count].handle = handle;
	r->inputs[r->inputs_count].input->info.encoder = NULL;
	r->inputs_count++;

	return 0;	
	
	
}

gint rlib_add_datasource_mysql(rlib *r, const gchar *input_name, const gchar *database_host, const gchar *database_user, const gchar *database_password, 
const gchar *database_database) {
	return rlib_add_datasource_mysql_private(r, input_name, NULL, database_host, database_user, database_password, database_database);
}

gint rlib_add_datasource_mysql_from_group(rlib *r, const gchar *input_name, const gchar *group) {
	return rlib_add_datasource_mysql_private(r, input_name, group, NULL, NULL, NULL, NULL);
}

gint rlib_add_datasource_postgre(rlib *r, const gchar *input_name, const gchar *conn) {
	GModule* handle;
	datasource_t ds;
	gpointer postgre;

	handle = g_module_open("libr-postgre", 2);
	if (!handle) {
		rlogit("Could Not Load POSTGRE Input [%s]\n", g_module_error());
		return -1;
	}
	g_module_symbol(handle, "rlib_postgre_new_input_filter", &ds.filter.ptr);
	g_module_symbol(handle, "rlib_postgre_connect", &ds.connect.ptr);
	r->inputs[r->inputs_count].input = ds.filter.new_input_filter();
	postgre = ds.connect.postgre_connect(r->inputs[r->inputs_count].input, conn);
	if(postgre == NULL) {
		rlogit("ERROR: Could not connect to POSTGRE\n");
		return -1;
	}
	r->inputs[r->inputs_count].name = g_strdup(input_name);
	r->inputs[r->inputs_count].handle = handle;
	r->inputs[r->inputs_count].input->info.encoder = NULL;
	r->inputs_count++;
	return 0;
}

gint rlib_add_datasource_odbc(rlib *r, const gchar *input_name, const gchar *source, const gchar *user, const gchar *password) {
	GModule* handle;
	datasource_t ds;
	gpointer odbc;
	
	handle = g_module_open("libr-odbc", 2);
	if (!handle) {
		rlogit("Could Not Load ODBC Input [%s]\n", g_module_error());
		return -1;
	}
	g_module_symbol(handle, "rlib_odbc_new_input_filter", &ds.filter.ptr);
	g_module_symbol(handle, "rlib_odbc_connect", &ds.connect.ptr);
	r->inputs[r->inputs_count].input = ds.filter.new_input_filter();
	odbc = ds.connect.odbc_connect(r->inputs[r->inputs_count].input, source, user, password);
	r->inputs[r->inputs_count].name = g_strdup(input_name);
	if(odbc == NULL) {
		rlogit("ERROR: Could not connect to ODBC\n");
		return -1;
	}
	r->inputs[r->inputs_count].handle = handle;
	r->inputs[r->inputs_count].input->info.encoder = NULL;
	r->inputs_count++;
	return 0;
}

gint rlib_add_datasource_xml(rlib *r, const gchar *input_name) {
	gpointer xml;

	r->inputs[r->inputs_count].input = rlib_xml_new_input_filter();
	xml = rlib_xml_connect(r->inputs[r->inputs_count].input);
	r->inputs[r->inputs_count].name = g_strdup(input_name);
	r->inputs[r->inputs_count].handle = NULL;
	r->inputs[r->inputs_count].input->info.encoder = NULL;
	r->inputs_count++;
	return 0;
}
