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

/*
	In case we have multiple servers later and we need to figure out which is the most avaialble one to hit up for a query
*/
MYSQL * rlib_mysql_real_connect(char *host, char *user, char *password, char *database) {
	MYSQL *mysql;
	mysql = emalloc(sizeof(MYSQL));

	mysql_init(mysql);

	if (mysql_real_connect(mysql,host,user,password, database, 0, NULL, 0) == NULL) {
		debugf("Could Not Real Connect To MySQL\n");
		return NULL;
	}
		
	if (mysql_select_db(mysql,database)) {
		debugf("Could Not Select Database\n");
		return NULL;
	}
	return mysql;
}

int rlib_mysql_close(MYSQL *mysql) {
	mysql_close(mysql);
	return 0;
}

MYSQL_RES * rlib_mysql_query(MYSQL *mysql, char *query) {
	MYSQL_RES *result = NULL;
	int rtn;
	
	rtn = mysql_query(mysql, query);
	if(rtn == 0) {
		result = mysql_store_result(mysql);
		return result;
	} else  {
		debugf("SQL ERROR %d\n", rtn);
	}		
	return NULL;
}
