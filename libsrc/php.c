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
ZEND_FUNCTION(rlib_add_datasource_mysql);
ZEND_FUNCTION(rlib_add_query_as);
ZEND_FUNCTION(rlib_add_report);
ZEND_FUNCTION(rlib_set_output_format);
ZEND_FUNCTION(rlib_execute);
ZEND_FUNCTION(rlib_spool);
ZEND_FUNCTION(rlib_finalize);
ZEND_FUNCTION(rlib_get_content_type);
ZEND_MODULE_STARTUP_D(rlib);

/*WRD: It appears we are thread safe here.. not sure yet*/
static int le_link;

/* compiled function list so Zend knows what's in this module */
zend_function_entry rlib_functions[] =
{
	 ZEND_FE(rlib_init, NULL)
	 ZEND_FE(rlib_add_datasource_mysql, NULL)
	 ZEND_FE(rlib_add_query_as, NULL)
	 ZEND_FE(rlib_add_report, NULL)
	 ZEND_FE(rlib_set_output_format, NULL)
	 ZEND_FE(rlib_execute, NULL)
	 ZEND_FE(rlib_spool, NULL)
	 ZEND_FE(rlib_finalize, NULL)
	 ZEND_FE(rlib_get_content_type, NULL)
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
	long sql_host_length, sql_user_length, sql_password_length, sql_database_length;
	rlib_inout_pass *rip;
	long resource_id;
	
	rip = emalloc(sizeof(rlib_inout_pass));
	bzero(rip, sizeof(rlib_inout_pass));
	
	rip->content_type = RLIB_CONTENT_TYPE_ERROR;

	rip->r = rlib_init();
	
	resource_id = ZEND_REGISTER_RESOURCE(return_value, rip, le_link);
	RETURN_RESOURCE(resource_id);
}

ZEND_FUNCTION(rlib_add_datasource_mysql) {
	zval *z_rip = NULL;
	long sql_host_length, sql_user_length, sql_password_length, sql_database_length;
	char *database_host, *database_user, *database_password, *database_database;
	rlib_inout_pass *rip;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rssss", &z_rip,
		&database_host, &sql_host_length, 
		&database_user, &sql_user_length, 
		&database_password, &sql_password_length, 
		&database_database, &sql_database_length) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);
	
	rlib_add_datasource_mysql(rip->r, database_host, database_user, database_password, database_database);
}

ZEND_FUNCTION(rlib_add_query_as) {
	zval *z_rip = NULL;
	long whatever;
	char *sql, *name;
	rlib_inout_pass *rip;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &z_rip, &sql, &whatever, &name, &whatever) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);	

	rlib_add_query_as(rip->r, estrdup(sql), estrdup(name));
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

ZEND_FUNCTION(rlib_set_output_format) {
	zval *z_rip = NULL;
	long whatever;
	char *name;
	rlib_inout_pass *rip;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &z_rip, &name, &whatever) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);	

	if(!strcasecmp(name, "PDF"))
		rip->format = RLIB_FORMAT_PDF;
	else if(!strcasecmp(name, "HTML"))
		rip->format = RLIB_FORMAT_HTML;
	else if(!strcasecmp(name, "TXT"))
		rip->format = RLIB_FORMAT_TXT;
	else if(!strcasecmp(name, "CSV"))
		rip->format = RLIB_FORMAT_CSV;
	else if(!strcasecmp(name, "XML"))
		rip->format = RLIB_FORMAT_XML;
	else
		zend_error(E_ERROR, "Valid Formats are PDF, HTML, TXT, CSV, or XML");
}

ZEND_FUNCTION(rlib_execute) {
	zval *z_rip = NULL;
	rlib_inout_pass *rip;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_rip) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);	
	rlib_execute(rip->r);
	if(rip->r == NULL) {
		zend_error(E_ERROR, "Could not load engine.. check logs");
	} else {
		rip->r->format = rip->format;
		if(rip->r != NULL) {
			make_report(rip->r);	
		}
	}
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
	
	rlib_input_close(rip->r);

}

ZEND_FUNCTION(rlib_finalize) {
	zval *z_rip = NULL;
	rlib_inout_pass *rip;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_rip) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);
	if(rip->r != NULL) {
		rlib_finalize(rip->r);
		if(rip->format == RLIB_FORMAT_PDF)
			rip->content_type = RLIB_CONTENT_TYPE_PDF;		
		else if(rip->format == RLIB_FORMAT_HTML)
			rip->content_type = RLIB_CONTENT_TYPE_HTML;
		else if(rip->format == RLIB_FORMAT_TXT)
			rip->content_type = RLIB_CONTENT_TYPE_TXT;
		else if(rip->format == RLIB_FORMAT_CSV)
			rip->content_type = RLIB_CONTENT_TYPE_CSV;
	}
}

#define CONTENT_TYPE_HTML "Content-Type: text/html\n"
#define CONTENT_TYPE_TEXT "Content-Type: text/plain\n"
#define CONTENT_TYPE_PDF "Content-Type: application/pdf"
#define CONTENT_TYPE_CSV "Content-type: application/octet-stream\nContent-Disposition: attachment; filename=report.csv\n"

ZEND_FUNCTION(rlib_get_content_type) {
	zval *z_rip = NULL;
	rlib_inout_pass *rip;
	int id = -1;
	static char buf[MAXSTRLEN];
	
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_rip) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(rip, rlib_inout_pass *, &z_rip, id, LE_RLIB_NAME, le_link);	

	if(rip->content_type == RLIB_CONTENT_TYPE_ERROR)
		sprintf(buf, "%s", CONTENT_TYPE_HTML);
	else if(rip->content_type == RLIB_CONTENT_TYPE_PDF) {
		sprintf(buf, "%s\n", CONTENT_TYPE_PDF);
		sprintf(buf, "%sContent-Length: %ld%c", buf, rip->r->length, 10);		
	} else if(rip->content_type == RLIB_CONTENT_TYPE_TXT) {
		sprintf(buf, "%s", CONTENT_TYPE_TEXT);
		sprintf(buf, "%sContent-Length: %ld%c", buf, rip->r->length, 10);		
	} else if(rip->content_type == RLIB_CONTENT_TYPE_HTML) {
		sprintf(buf, "%s", CONTENT_TYPE_HTML);
		sprintf(buf, "%sContent-Length: %ld%c", buf, rip->r->length, 10);		
	} else if(rip->content_type == RLIB_CONTENT_TYPE_CSV) {
		sprintf(buf, "%s", CONTENT_TYPE_CSV);
	}

	RETURN_STRING(buf, TRUE);
}

void rlib_write_output(char *buf, long length) {
	php_write(buf, length TSRMLS_CC);
}

char * rlib_php_resolve_memory_variable(char *name) {
	zval **data; 
	if(strlen(name) >= 3 && name[0] == 'm' && name[1] == '.') {
		name += 2;
		if (zend_hash_find(&EG(symbol_table),name,strlen(name)+1,(void **)&data)==FAILURE) { 
			debugf("rlib_resolve_memory_variable: could not resolve [%s], this could lead to more problems!\n", name);
			return NULL;
		} else {
	
			return Z_STRVAL_PP(data);
		}
	}
	return NULL;
}
