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
 
#include <string.h>
#include <dlfcn.h>

#include "ralloc.h"
#include "rlib.h"
#include "rlib_input.h"

gint rlib_add_datasource(rlib *r, gchar *input_name, struct input_filter *input) {
	r->inputs[r->inputs_count].input = input;
	r->inputs[r->inputs_count].name = input_name;
	r->inputs[r->inputs_count].handle = NULL;
	r->inputs_count++;
	return 0;
}

#if HAVE_MYSQL
gint rlib_add_datasource_mysql(rlib *r, gchar *input_name, gchar *database_host, gchar *database_user, gchar *database_password, 
gchar *database_database) {
	gpointer handle;
	gpointer (*rlib_mysql_new_input_filter)();
	gpointer (*rlib_mysql_real_connect)(void *, char *, char *, char*, char *);
	gpointer mysql;
	
	handle = dlopen ("libr-mysql.so", RTLD_LAZY);
	if (!handle) {
		rlogit("DLOPEN SAYS [%s]\n", dlerror());
		return -1;
	}
                                                                                                  
	rlib_mysql_new_input_filter = dlsym(handle, "rlib_mysql_new_input_filter");
	rlib_mysql_real_connect = dlsym(handle, "rlib_mysql_real_connect");
                                                                                                   
	r->inputs[r->inputs_count].input = rlib_mysql_new_input_filter();
	mysql = rlib_mysql_real_connect(r->inputs[r->inputs_count].input, database_host, database_user, database_password, database_database);
	r->inputs[r->inputs_count].name = input_name;
	if(mysql == NULL) {
		rlogit("ERROR: Could not connect to MYSQL\n");
		return -1;
	}
	
	r->inputs_count++;
	r->inputs[r->inputs_count].handle = handle;

	return 0;
}
#endif

#if HAVE_POSTGRE
gint rlib_add_datasource_postgre(rlib *r, gchar *input_name, gchar *conn) {
	gpointer handle;
	gpointer (*rlib_postgre_new_input_filter)();
	gpointer (*rlib_postgre_connect)(void *, char *);
	gpointer postgre;

	handle = dlopen ("libr-postgre.so", RTLD_LAZY);
	if (!handle)
		return -1;

	rlib_postgre_new_input_filter = dlsym(handle, "rlib_postgre_new_input_filter");
	rlib_postgre_connect = dlsym(handle, "rlib_postgre_connect");

	r->inputs[r->inputs_count].input = rlib_postgre_new_input_filter();
	postgre = rlib_postgre_connect(r->inputs[r->inputs_count].input, conn);
	r->inputs[r->inputs_count].name = input_name;
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
	gpointer handle;
	gpointer (*rlib_odbc_new_input_filter)();
	gpointer (*rlib_odbc_connect)(void *, char *, char *, char *);
	gpointer odbc;

	handle = dlopen ("libr-odbc.so", RTLD_LAZY);
	if (!handle)
		return -1;

	rlib_odbc_new_input_filter = dlsym(handle, "rlib_odbc_new_input_filter");
	rlib_odbc_connect = dlsym(handle, "rlib_odbc_connect");

	r->inputs[r->inputs_count].input = rlib_odbc_new_input_filter();
	odbc = rlib_odbc_connect(r->inputs[r->inputs_count].input, source, user, password);
	r->inputs[r->inputs_count].name = input_name;
	if(odbc == NULL) {
		rlogit("ERROR: Could not connect to ODBC\n");
		return -1;
	}
	r->inputs[r->inputs_count].handle = handle;
	
	r->inputs_count++;
	return 0;
}
#endif
