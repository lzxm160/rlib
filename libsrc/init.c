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

/*
	We need to connect to the SQL server, run the quereries, and parse
	the xml fileds.. if any of that goes wrong we are screwed and wee
	need to tell the user that
*/

/*
	Accepts Stuff from the PHP side of things and returns a struct rlib
	if things work out.......  All this is done before the main loop
*/
rlib * rlib_init(rlib_inout_pass *rip) {
	int i,j;
	void *mysql;
	rlib *r;
	
	setlocale (LC_NUMERIC, "en_US");

	r = rmalloc(sizeof(rlib));
	bzero(r, sizeof(rlib));

	r->input = rlib_mysql_new_input_filter();
	mysql = INPUT(r)->rlib_input_connect(INPUT(r), rip->database_host, rip->database_user, rip->database_password, rip->database_database);

	if(mysql == NULL) {
		debugf("Could not connect to MYSQL\n");
		return NULL;
	}

	if(rip->queries_count <= 0) {
		debugf("You need to specify at least one query\n");
		return NULL;
	}

	if(rip->reports_count <= 0) {
		debugf("You need to specify at least one report\n");
		return NULL;
	}
	
	for(i=0;i<rip->queries_count;i++) {
		INPUT(r)->query_and_set_result(INPUT(r), i, rip->queries[i].sql);
		if(INPUT(r)->get_result_pointer(INPUT(r), i) == NULL) {
			rfree(r);
			debugf("Failed To Run A Query!\n");			
			return NULL;
		}
		INPUT(r)->set_query_result_name(INPUT(r), i, rip->queries[i].name);
	}
	r->results_count = rip->queries_count;

	LIBXML_TEST_VERSION
	xmlKeepBlanksDefault(0);
	for(i=0;i<rip->reports_count;i++) {
		r->reports[i] = parse_report_file(rip->reports[i].name);
		r->reports[i]->mainloop_query = -1;
		if(rip->reports[i].query != NULL) {
			for(j=0;j<rip->queries_count;j++) {
				if(!strcmp(rip->queries[j].name, rip->reports[i].query)) {
					r->reports[i]->mainloop_query = j;
					break;
				}					
			}
		}
		xmlCleanupParser();		
		if(r->reports[i] == NULL) {
			//TODO:FREE REPORT AND ALL ABOVE REPORTS
			debugf("Failed to run a Report\n");
			return NULL;
		}
	}
	r->reports_count = rip->reports_count;
	
	rip->content_type = RLIB_CONTENT_TYPE_PDF;
	return r;
}
