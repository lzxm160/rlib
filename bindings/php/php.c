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

#include "rlib.h"
#include "rlib_php.h"

/*
	here we define the PHP interface to rlib.  always assume no access to this source when making methods
	If you want to hack this read the "Extending PHP" section of the PHP Manual from php.net
*/


/* declaration of functions to be exported */
ZEND_FUNCTION(rlib_init);
#if HAVE_MYSQL
ZEND_FUNCTION(rlib_add_datasource_mysql);
#endif
#if HAVE_POSTGRE
ZEND_FUNCTION(rlib_add_datasource_postgre);
#endif
#if HAVE_ODBC
ZEND_FUNCTION(rlib_add_datasource_odbc);
#endif
ZEND_FUNCTION(rlib_add_query_as);
ZEND_FUNCTION(rlib_add_resultset_follower);
ZEND_FUNCTION(rlib_add_report);
ZEND_FUNCTION(rlib_set_output_format_from_text);
ZEND_FUNCTION(rlib_execute);
ZEND_FUNCTION(rlib_spool);
ZEND_FUNCTION(rlib_free);
ZEND_FUNCTION(rlib_get_content_type);
ZEND_FUNCTION(rlib_add_parameter);
ZEND_FUNCTION(rlib_set_locale);
ZEND_MODULE_STARTUP_D(rlib);

/*WRD: It appears we are thread safe here.. not sure yet*/
static int le_link;

/* compiled function list so Zend knows what's in this module */
zend_function_entry rlib_functions[] =
{
	 ZEND_FE(rlib_init, NULL)
#if HAVE_MYSQL
	 ZEND_FE(rlib_add_datasource_mysql, NULL)
#endif
#if HAVE_POSTGRE
	 ZEND_FE(rlib_add_datasource_postgre, NULL)
#endif
#if HAVE_ODBC
	 ZEND_FE(rlib_add_datasource_odbc, NULL)
#endif
	 ZEND_FE(rlib_add_query_as, NULL)
	 ZEND_FE(rlib_add_resultset_follower, NULL)
	 ZEND_FE(rlib_add_report, NULL)
	 ZEND_FE(rlib_set_output_format_from_text, NULL)
	 ZEND_FE(rlib_execute, NULL)
	 ZEND_FE(rlib_spool, NULL)
	 ZEND_FE(rlib_free, NULL)
	 ZEND_FE(rlib_get_content_type, NULL)
	 ZEND_FE(rlib_add_parameter, NULL)
	 ZEND_FE(rlib_set_locale, NULL)
    {NULL, NULL, NULL}
};

/* compiled module information */
zend_module_entry rlib_module_entry =
{
    STANDARD_MODULE_HEADER,
    "rlib",
    rlib_functions,
    ZEND_MODULE_STARTUP_N(rlib), 
    NULL, 
    NULL, 
    NULL, 
    NULL,
    NO_VERSION_YET,
    STANDARD_MODULE_PROPERTIES
};

static void _close_rlib_link(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	rlib_inout_pass *rip = (rlib_inout_pass *)rsrc->ptr;
	efree(rip);
}

ZEND_MODULE_STARTUP_D(rlib) {
	le_link = zend_register_list_destructors_ex(_close_rlib_link, NULL, LE_RLIB_NAME, module_number);
	return SUCCESS;
};

/* implement standard "stub" routine to introduce ourselves to Zend */
ZEND_GET_MODULE(rlib)

/*
	This will Make the connection to the SQL server and select the right database
*/
ZEND_FUNCTION(rlib_init) {
	rlib_inout_pass *rip;
	long resource_id;
	
	rip = emalloc(sizeof(rlib_inout_pass));
	bzero(rip, sizeof(rlib_inout_pass));
	
	rip->content_type = RLIB_CONTENT_TYPE_ERROR;

	rip->r = rlib_init_with_environment(rlib_php_new_environment());
	
	resource_id = ZEND_REGISTER_RESOURCE(return_value, rip, le_link);
	RETURN_RESOURCE(resource_id);
}

#if HAVE_MYSQL
ZEND_FUNCTION(rlib_add_datasource_mysql) {
	zval *z_rip = NULL;
	long datasource_length, sql_host_length, sql_user_length, sql_password_length, sql_database_length;
	char *datasource_name, *database_host, *database_user, *database_password, *database_database;
	rlib_inout_pass *rip;
	int id = -1;
	int result = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsssss", &z_rip,
		&datasource_name, &datasource_length,
		&database_host, &sql_host_length, 
		&database_user, &sql_user_length, 
		&database_password, &sql_password_length, 
		&database_database, &sql_database_length) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);
	
	result = rlib_add_datasource_mysql(rip->r, datasource_name, database_host, database_user, database_password, database_database);
	RETURN_LONG(result);
}
#endif

#if HAVE_POSTGRE
ZEND_FUNCTION(rlib_add_datasource_postgre) {
	zval *z_rip = NULL;
	long datasource_length, conn_length;
	char *datasource_name, *conn;
	rlib_inout_pass *rip;
	int id = -1;
	int result = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &z_rip,
		&datasource_name, &datasource_length,
		&conn, &conn_length) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);
	
	result = rlib_add_datasource_postgre(rip->r, datasource_name, conn);
	RETURN_LONG(result);
}
#endif

#if HAVE_ODBC
ZEND_FUNCTION(rlib_add_datasource_odbc) {
	zval *z_rip = NULL;
	long datasource_length, sql_odbc_length, sql_user_length, sql_password_length;
	char *datasource_name, *database_odbc, *database_user, *database_password;
	rlib_inout_pass *rip;
	int id = -1;
	int result = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rssss", &z_rip,
		&datasource_name, &datasource_length,
		&database_odbc, &sql_odbc_length, 
		&database_user, &sql_user_length, 
		&database_password, &sql_password_length) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);
	
	result = rlib_add_datasource_odbc(rip->r, datasource_name, database_odbc, database_user, database_password);
	RETURN_LONG(result);
}
#endif


ZEND_FUNCTION(rlib_add_query_as) {
	zval *z_rip = NULL;
	long whatever;
	char *datasource_name, *sql, *name;
	rlib_inout_pass *rip;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsss", &z_rip, 
		&datasource_name, &whatever, &sql, &whatever, &name, &whatever) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);	

	rlib_add_query_as(rip->r, estrdup(datasource_name), estrdup(sql), estrdup(name));
}

ZEND_FUNCTION(rlib_add_resultset_follower) {
	zval *z_rip = NULL;
	long whatever;
	char *leader, *follower;
	rlib_inout_pass *rip;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &z_rip, &leader, &whatever, &follower, &whatever) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);	

	rlib_add_resultset_follower(rip->r, estrdup(leader), estrdup(follower));
}

ZEND_FUNCTION(rlib_add_report) {
	zval *z_rip = NULL;
	long whatever, mainloop_count;
	char *name, *mainloop;
	rlib_inout_pass *rip;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|s", &z_rip, &name, &whatever, &mainloop, &mainloop_count) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);	

	if(mainloop_count > 0)
		rlib_add_report(rip->r, estrdup(name), estrdup(mainloop));
	else
		rlib_add_report(rip->r, estrdup(name), NULL);
		
}

ZEND_FUNCTION(rlib_set_output_format_from_text) {
	zval *z_rip = NULL;
	long whatever;
	char *name;
	rlib_inout_pass *rip;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &z_rip, &name, &whatever) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);	

	rlib_set_output_format_from_text(rip->r, name);

}

ZEND_FUNCTION(rlib_execute) {
	zval *z_rip = NULL;
	rlib_inout_pass *rip;
	int id = -1;
	int result = 0;	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_rip) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);	
	result = rlib_execute(rip->r);
	RETURN_LONG(result);
}

ZEND_FUNCTION(rlib_spool) {
	zval *z_rip = NULL;
	rlib_inout_pass *rip;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_rip) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);	

	if(rip->r != NULL)
		rlib_spool(rip->r);
	else {
		zend_error(E_ERROR, "Unable to run report with requested data");
	}
	
}

ZEND_FUNCTION(rlib_free) {
	zval *z_rip = NULL;
	rlib_inout_pass *rip;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_rip) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);
	if(rip->r != NULL) {
		rlib_free(rip->r);
	}
}

ZEND_FUNCTION(rlib_get_content_type) {
	zval *z_rip = NULL;
	rlib_inout_pass *rip;
	int id = -1;
	static char buf[MAXSTRLEN];
	char *content_type;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_rip) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);	

	content_type = rlib_get_content_type_as_text(rip->r);

	sprintf(buf, "%s%c", content_type, 10);

	RETURN_STRING(buf, TRUE);
}

ZEND_FUNCTION(rlib_add_parameter) {
	zval *z_rip = NULL;
	long whatever;
	char *name, *value;
	rlib_inout_pass *rip;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &z_rip, &name, &whatever, &value, &whatever) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);	

	rlib_add_parameter(rip->r, name, value);
}

ZEND_FUNCTION(rlib_set_locale) {
	zval *z_rip = NULL;
	long whatever;
	char *locale;
	rlib_inout_pass *rip;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &z_rip, &locale, &whatever) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);	

	rlib_set_locale(rip->r, locale);
}
