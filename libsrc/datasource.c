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

#include <config.h>
#include <string.h>
#include <gmodule.h>

#include "rlib.h"
#include "rlib_input.h"

gint rlib_add_datasource(rlib *r, gchar *input_name, struct input_filter *input) {
	r->inputs[r->inputs_count].input = input;
	r->inputs[r->inputs_count].name = g_strdup(input_name);
	r->inputs[r->inputs_count].handle = NULL;
//	r->inputs[r->inputs_count].input->input_decoder = (iconv_t)-1;
	r->inputs_count++;
	return 0;
}

#if HAVE_MYSQL
static gint rlib_add_datasource_mysql_private(rlib *r, gchar *input_name, gchar *database_group, gchar *database_host, 
gchar *database_user, gchar *database_password, gchar *database_database) {
	GModule* handle;
	gpointer rlib_mysql_new_input_filter;
	gpointer rlib_mysql_real_connect;
	gpointer (*f1)();
	gpointer (*f2)(gpointer, gchar *, gchar *, gchar *, gchar*, gchar *);
	gpointer mysql;

	handle = g_module_open("libr-mysql", 2);
	if (!handle) {
		rlogit("Could Not Load MYSQL Input [%s]\n", g_module_error());
		return -1;
	}

	g_module_symbol(handle, "rlib_mysql_new_input_filter", (gpointer *)&rlib_mysql_new_input_filter);
	g_module_symbol(handle, "rlib_mysql_real_connect", (gpointer *)&rlib_mysql_real_connect);
	f1 = rlib_mysql_new_input_filter;															                                                                                                  
	f2 = rlib_mysql_real_connect;
	r->inputs[r->inputs_count].input = f1();
	mysql = f2(r->inputs[r->inputs_count].input, database_group, database_host, database_user, 
		database_password, database_database);

	if(mysql == NULL) {
		rlogit("ERROR: Could not connect to MYSQL\n");
		return -1;
	}
	
	r->inputs[r->inputs_count].name = g_strdup(input_name);
	r->inputs[r->inputs_count].handle = handle;
	r->inputs_count++;

	return 0;	
	
	
}

gint rlib_add_datasource_mysql(rlib *r, gchar *input_name, gchar *database_host, gchar *database_user, gchar *database_password, 
gchar *database_database) {
	return rlib_add_datasource_mysql_private(r, input_name, NULL, database_host, database_user, database_password, database_database);
}

gint rlib_add_datasource_mysql_from_group(rlib *r, gchar *input_name, gchar *group) {
	return rlib_add_datasource_mysql_private(r, input_name, group, NULL, NULL, NULL, NULL);
}
#endif

#if HAVE_POSTGRE
gint rlib_add_datasource_postgre(rlib *r, gchar *input_name, gchar *conn) {
	GModule* handle;
	gpointer rlib_postgre_new_input_filter;
	gpointer rlib_postgre_connect;//(gpointer, gchar *)
	gpointer postgre;
	gpointer (*f1)();
	gpointer (*f2)(gpointer, gchar *);

	handle = g_module_open("libr-postgre", 2);
	if (!handle) {
		rlogit("Could Not Load POSTGRE Input [%s]\n", g_module_error());
		return -1;
	}
	g_module_symbol(handle, "rlib_postgre_new_input_filter", &rlib_postgre_new_input_filter);
	g_module_symbol(handle, "rlib_postgre_connect", &rlib_postgre_connect);
	f1 = rlib_postgre_new_input_filter;
	f2 = rlib_postgre_connect;
	r->inputs[r->inputs_count].input = f1();
	postgre = f2(r->inputs[r->inputs_count].input, conn);
	r->inputs[r->inputs_count].name = g_strdup(input_name);
	if(postgre == NULL) {
		rlogit("ERROR: Could not connect to POSTGRE\n");
		return -1;
	}
	r->inputs[r->inputs_count].handle = handle;
	r->inputs_count++;
	return 0;
}
#endif

#if HAVE_ODBC
gint rlib_add_datasource_odbc(rlib *r, gchar *input_name, gchar *source, gchar *user, gchar *password) {
	GModule* handle;
	gpointer rlib_odbc_new_input_filter;
	gpointer rlib_odbc_connect;
	gpointer (*f1)();
	gpointer (*f2)(gpointer, gchar *, gchar *, gchar *);
	gpointer odbc;
	
	handle = g_module_open("libr-odbc", 2);
	if (!handle) {
		rlogit("Could Not Load ODBC Input [%s]\n", g_module_error());
		return -1;
	}
	g_module_symbol(handle, "rlib_odbc_new_input_filter", (gpointer *)&rlib_odbc_new_input_filter);
	g_module_symbol(handle, "rlib_odbc_connect", (gpointer *)&rlib_odbc_connect);
	f1 = rlib_odbc_new_input_filter;
	f2 = rlib_odbc_connect;
	r->inputs[r->inputs_count].input = f1();
	odbc = f2(r->inputs[r->inputs_count].input, source, user, password);
	r->inputs[r->inputs_count].name = g_strdup(input_name);
	if(odbc == NULL) {
		rlogit("ERROR: Could not connect to ODBC\n");
		return -1;
	}
	r->inputs[r->inputs_count].handle = handle;
	r->inputs_count++;
	return 0;
}
#endif

