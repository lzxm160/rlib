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
#include <locale.h>

#include "ralloc.h"
#include "rlib.h"

rlib * rlib_init() {
	rlib *r;
	
	setlocale (LC_NUMERIC, "en_US");
	init_signals();

	r = rmalloc(sizeof(rlib));
	bzero(r, sizeof(rlib));

	r->input = rlib_mysql_new_input_filter();

	return r;
}

int rlib_add_datasource_mysql(rlib *r, char *database_host, char *database_user, char *database_password, char *database_database) {
	void *mysql;

	mysql = INPUT(r)->rlib_input_connect(INPUT(r), database_host, database_user, database_password, database_database);

	if(mysql == NULL) {
		debugf("ERROR: Could not connect to MYSQL\n");
		return -1;
	}
}

int rlib_add_query_as(rlib *r, char *sql, char *name) {
	if(r->queries_count > (RLIB_MAXIMUM_QUERIES-1)) {
		return -1;
	}

	r->queries[r->queries_count].sql = sql;
	r->queries[r->queries_count].name = name;
	r->queries_count++;
	return r->queries_count;
}

int rlib_add_report(rlib *r, char *name, char *mainloop) {
	if(r->reports_count > (RLIB_MAXIMUM_REPORTS-1)) {
		return - 1;
	}
	
	r->reportstorun[r->reports_count].name = name;
	r->reportstorun[r->reports_count].query = mainloop;
	r->reports_count++;
	return r->reports_count;
}

int rlib_execute(rlib *r) {
	int i,j;
	void *mysql;

	for(i=0;i<r->queries_count;i++) {
		INPUT(r)->query_and_set_result(INPUT(r), i, r->queries[i].sql);
		if(INPUT(r)->get_result_pointer(INPUT(r), i) == NULL) {
			rfree(r);
			debugf("Failed To Run A Query!\n");			
			return -1;
		}
		INPUT(r)->set_query_result_name(INPUT(r), i, r->queries[i].name);
	}
//TODO: this is stupid
	r->results_count = r->queries_count;

	LIBXML_TEST_VERSION

	xmlKeepBlanksDefault(0);
	for(i=0;i<r->reports_count;i++) {
		r->reports[i] = parse_report_file(r->reportstorun[i].name);
		r->reports[i]->mainloop_query = -1;
		if(r->reportstorun[i].query != NULL) {
			for(j=0;j<r->queries_count;j++) {
				if(!strcmp(r->queries[j].name, r->reportstorun[i].query)) {
					r->reports[i]->mainloop_query = j;
					break;
				}					
			}
		}
		xmlCleanupParser();		
		if(r->reports[i] == NULL) {
			//TODO:FREE REPORT AND ALL ABOVE REPORTS
			debugf("Failed to run a Report\n");
			return -1;
		}
	}
	
	return 0;
}
