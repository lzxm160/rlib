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

 %module librlib
 %{
#include <rlib.h>
 %}
 
rlib * rlib_init();
int rlib_add_datasource_mysql(rlib *r, char *input_name, char *database_host, char *database_user, char *database_password, char *database_database);
int rlib_add_datasource_postgre(rlib *r, char *input_name, char *conn);
int rlib_add_query_as(rlib *r, char *input_source, char *sql, char *name);
int rlib_add_report(rlib *r, char *name, char *mainloop);
int rlib_execute(rlib *r);
int rlib_set_output_format_from_text(rlib *r, char * name);
int rlib_spool(rlib *r);
int rlib_free(rlib *r);
char *rlib_get_output(rlib *r);
long rlib_get_output_length(rlib *r);
int rlib_mysql_report(char *hostname, char *username, char *password, char *database, char *xmlfilename, char *sqlquery, char *outputformat);
int rlib_postgre_report(char *connstr, char *xmlfilename, char *sqlquery, char *outputformat);
