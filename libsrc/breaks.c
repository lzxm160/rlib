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

#include <stdlib.h>
#include <string.h>

#include "rlib.h"
#include "pcode.h"

static void rlib_print_break_header_lines(rlib *r, struct report_break *rb, struct report_output_array *roa, int backwards) {
	int blank = TRUE;
	int surpress = FALSE;

	if(!OUTPUT(r)->do_break)
		return;
		
	if(rb->surpressblank) {
		struct report_element *be;
		surpress = TRUE;
		for(be = rb->fields; be != NULL; be=be->next) {
			struct break_fields *bf = be->data;
			if((bf->rval == NULL || (RLIB_VALUE_IS_STRING(bf->rval) && !strcmp(RLIB_VALUE_GET_AS_STRING(bf->rval), ""))) && blank == TRUE)
				blank = TRUE;
			else
				blank = FALSE;
		}		
		
	}

	if(!surpress || (surpress && !blank)) {
		rb->didheader = TRUE;
		if(roa != NULL)
			print_detail_line(r, roa, backwards);
	} else {
		rb->didheader = FALSE;
	}
}

static void rlib_print_break_footer_lines(rlib *r, struct report_break *rb, struct report_output_array *roa, int backwards) {
	if(!OUTPUT(r)->do_break)
		return;

	if(rb->didheader)
		print_detail_line(r, roa, backwards);
}

void rlib_force_break_headers(rlib *r) {
	struct report_element *e;

	if(!OUTPUT(r)->do_break)
		return;
	
	if(r->reports[r->current_report]->breaks == NULL)
		return;
	
	for(e = r->reports[r->current_report]->breaks; e != NULL; e=e->next) {
		struct report_break *rb = e->data;
		if(rb->headernewpage) {
			rlib_print_break_header_lines(r, rb, rb->header, FALSE);
		}
				
	}
}

void rlib_handle_break_headers(rlib *r) {
	struct report_element *e;
	struct report_break *cache[100];
	int icache=0,i;
	float total=0;

	if(!OUTPUT(r)->do_break)
		return;
	
	if(r->reports[r->current_report]->breaks == NULL)
		return;
	
	for(e = r->reports[r->current_report]->breaks; e != NULL; e=e->next) {
		struct report_break *rb = e->data;
		struct report_element *be;
		int dobreak=1;
		for(be = rb->fields; be != NULL; be=be->next) {
			struct break_fields *bf = be->data;
			if(dobreak && bf->rval == NULL) {
				dobreak=1;
				bf->rval = rlib_execute_pcode(r, &bf->rval2, bf->code, NULL);
			} else {
				dobreak = 0;
			}
		}
		
		if(dobreak) {
			if(rb->header != NULL) {
				total += get_output_size(r, rb->header);
				cache[icache++] = rb;
			} else {
				rb->didheader = TRUE;
			}
		}
				
	}
	if(icache) {	
		if(!will_this_fit(r,total)) {
			OUTPUT(r)->rlib_end_page(r);
			rlib_force_break_headers(r);
		} else {
			for(i=0;i<icache;i++) {
				rlib_print_break_header_lines(r, cache[i], cache[i]->header, FALSE);	
			}
		}
	}
}

//TODO: Variables need to resolve the name into a number or something.. like break numbers for more efficient compareseon
void rlib_reset_variables_on_break(rlib *r, char *name) {
	struct report_element *e;

	if(!OUTPUT(r)->do_break)
		return;

	for(e = r->reports[r->current_report]->variables; e != NULL; e=e->next) {
		struct report_variable *rv = e->data;
		if(rv->resetonbreak != NULL && rv->resetonbreak[0] != '\0' && !strcmp(rv->resetonbreak, name)) {
			if(rv->type == REPORT_VARIABLE_COUNT) {
				RLIB_VARIABLE_CA(rv)->count = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->count, 0);
			} else if(rv->type == REPORT_VARIABLE_SUM) {
				RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
			} else if(rv->type == REPORT_VARIABLE_AVERAGE) {
				RLIB_VARIABLE_CA(rv)->count = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->count, 0);
				RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
			} else if(rv->type == REPORT_VARIABLE_LOWEST) {
				RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
			} else if(rv->type == REPORT_VARIABLE_HIGHEST) {
				RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
			}
		}
		if(rv->type == REPORT_VARIABLE_EXPRESSION)
			RLIB_VARIABLE_CA(rv)->count = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		
	}
}

void rlib_break_all_below_in_reverse_order(rlib *r, struct report_element *e) {
	struct report_break *rb;
	struct report_element *xxx, *be;
	int count=0,i=0,j=0;
	struct break_fields *bf;
	int do_endpage = FALSE;

	if(!OUTPUT(r)->do_break)
		return;

	for(xxx =e; xxx != NULL; xxx=xxx->next)
		count++;

	for(i=count;i > 0;i--) {
		xxx = e;
		for(j=0;j<i-1;j++)
			xxx = xxx->next;		
		rb = xxx->data;
		for(be = rb->fields; be != NULL; be=be->next) {
			bf = be->data;
			bf->rval = NULL;
		}
		rlib_end_page_if_line_wont_fit(r, rb->footer);
			
/*
	Fun little hack so break lines reflect the correct value.. not the next row
*/

		INPUT(r)->previous(INPUT(r), r->results[r->current_result].result);
		
		rlib_print_break_footer_lines(r, rb, rb->footer, FALSE);

		INPUT(r)->next(INPUT(r), r->results[r->current_result].result);

		rlib_reset_variables_on_break(r, rb->name);

		if(rb->newpage) {
			do_endpage = TRUE;
		}
	}
	if(do_endpage) {
		if(!INPUT(r)->isdone(INPUT(r), r->results[r->current_result].result)) {
			OUTPUT(r)->rlib_end_page(r);
			rlib_force_break_headers(r);
		}
	}
}

/*
	Footers are complicated.... I need to go in reverse order for footers... and if I find a match...
	I need to go back down the list and force breaks for everyone.. in reverse order.. ugh

*/
void rlib_handle_break_footers(rlib *r) {
	struct report_element *e;
	struct break_fields *bf;

	if(!OUTPUT(r)->do_break)
		return;
	
	if(r->reports[r->current_report]->breaks == NULL)
		return;
	
	for(e = r->reports[r->current_report]->breaks; e != NULL; e=e->next) {
		struct report_break *rb = e->data;
		struct report_element *be;
		int dobreak=1;
		for(be = rb->fields; be != NULL; be=be->next) {
			struct rlib_value rval_tmp;
			bf = be->data;
			if(dobreak && (INPUT(r)->isdone(INPUT(r), r->results[r->current_result].result) || rvalcmp(bf->rval, rlib_execute_pcode(r, &rval_tmp, bf->code, NULL)))) {
				dobreak=1;
			} else {
				dobreak = 0;
			}
		}
		
		if(dobreak) {
			rlib_break_all_below_in_reverse_order(r, e);
			break;
		}
				
	}
}

