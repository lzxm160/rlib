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
 
#include <rlib.h>

int main(int argc, char **argv) {
	char *hostname, *username, *password, *database;
	rlib *r;

	if(argc != 5) {
		fprintf(stderr, "%s requires 4 arguments hostname username password database\n", argv[0]);
		fprintf(stderr, "You provided %d\n", argc-1);
		return -1;
	}
	
	hostname = argv[1];
	username = argv[2];
	password = argv[3];
	database = argv[4];

	r = rlib_init(NULL);
	rlib_add_datasource_mysql(r, hostname, username, password, database);
	rlib_add_query_as(r, "select * from example", "example");
	rlib_add_query_as(r, "select * from example order by name", "example2");
	rlib_add_query_as(r, "select * from example", "example3");
	rlib_add_report(r, "report.xml", NULL);
	rlib_add_report(r, "report.xml", "example2");
	rlib_add_report(r, "report2.xml", "example3");
	r->format = RLIB_FORMAT_PDF;
	rlib_execute(r);
	rlib_finalize(r);
	rlib_spool(r);
	rlib_free(r);
	return 0;
}