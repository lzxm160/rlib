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

#include "config.h"
#include "rlib.h"
#include "pcode.h"
#include "rlib_input.h"

static void rlib_print_break_header_output(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_report_break *rb, struct rlib_element *e, gint backwards) {
	gint blank = TRUE;
	gint suppress = FALSE;

	if(!OUTPUT(r)->do_break)
		return;
		
	if(rb->suppressblank) {
		struct rlib_element *be;
		suppress = TRUE;
		for(be = rb->fields; be != NULL; be=be->next) {
			struct rlib_break_fields *bf = be->data;
			if((bf->rval == NULL || (RLIB_VALUE_IS_STRING(bf->rval) && !strcmp(RLIB_VALUE_GET_AS_STRING(bf->rval), ""))) && blank == TRUE)
				blank = TRUE;
			else
				blank = FALSE;
		}		
		
	}
	if(!suppress || (suppress && !blank)) {
		rb->didheader = TRUE;
		if(e != NULL)
			rlib_layout_report_output(r, part, report, e, backwards);
	} else {
		rb->didheader = FALSE;
	}
}

static void rlib_print_break_footer_output(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_report_break *rb, struct rlib_element *e, gint backwards) {
	if(!OUTPUT(r)->do_break)
		return;

	if(rb->didheader)
		rlib_layout_report_output(r, part, report, e, backwards);
}

void rlib_force_break_headers(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	struct rlib_element *e;

	if(!OUTPUT(r)->do_break)
		return;
	
	if(report->breaks == NULL)
		return;
	
	for(e = report->breaks; e != NULL; e=e->next) {
		gint dobreak=1;
		struct rlib_report_break *rb = e->data;
		struct rlib_element *be;
		for(be = rb->fields; be != NULL; be=be->next) {
			struct rlib_break_fields *bf = be->data;
			if(dobreak && bf->rval == NULL) {
				dobreak=1;
				rlib_value_free(bf->rval);
				bf->rval = rlib_execute_pcode(r, &bf->rval2, bf->code, NULL);
			} else {
				dobreak = 0;
			}
		}	
	}
	
	for(e = report->breaks; e != NULL; e=e->next) {
		struct rlib_report_break *rb = e->data;
		if(rb->headernewpage) {
			rlib_print_break_header_output(r, part, report, rb, rb->header, FALSE);
		}
				
	}
}

void rlib_handle_break_headers(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	gint icache=0,page,i;
	gfloat total[RLIB_MAXIMUM_PAGES_ACROSS];
	struct rlib_element *e;
	struct rlib_report_break *cache[100];

	if(report->breaks == NULL)
		return;
	
	for(i=0;i<RLIB_MAXIMUM_PAGES_ACROSS;i++) 
		total[i] = 0;

	for(e = report->breaks; e != NULL; e=e->next) {
		gint dobreak=1;
		struct rlib_report_break *rb = e->data;
		struct rlib_element *be;
		for(be = rb->fields; be != NULL; be=be->next) {
			struct rlib_break_fields *bf = be->data;
			if(dobreak && bf->rval == NULL) {
				dobreak=1;
				rlib_value_free(bf->rval);
				bf->rval = rlib_execute_pcode(r, &bf->rval2, bf->code, NULL);
			} else {
				dobreak = 0;
			}
		}
		
		if(dobreak) {
			if(rb->header != NULL) {
				cache[icache++] = rb;
			} else {
				rb->didheader = TRUE;
			}
			for(page=0;page<report->pages_across;page++) {
				total[page] += get_outputs_size(r, rb->header, page);
			}
		}
				
	}
	if(icache && OUTPUT(r)->do_break) {	
		gint allfit = TRUE;
		for(page=0;page<report->pages_across;page++) {
			if(!will_this_fit(r, part, report, total[page], page+1))
				allfit = FALSE;
		}
		if(!allfit) {
			OUTPUT(r)->end_page(r, part);
			rlib_force_break_headers(r, part, report);
		} else {
			for(i=0;i<icache;i++) {
				rlib_print_break_header_output(r, part, report, cache[i], cache[i]->header, FALSE);	
			}
		}
	}
}

//TODO: Variables need to resolve the name into a number or something.. like break numbers for more efficient compareseon
void rlib_reset_variables_on_break(rlib *r, struct rlib_part *part, struct rlib_report *report, gchar *name) {
	struct rlib_element *e;

	for(e = report->variables; e != NULL; e=e->next) {
		struct rlib_report_variable *rv = e->data;
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

//		if(rv->type == REPORT_VARIABLE_EXPRESSION)
//			RLIB_VARIABLE_CA(rv)->count = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		
	}
}

void rlib_break_all_below_in_reverse_order(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_element *e) {
	gint count=0,i=0,j=0;
	gint do_endpage = FALSE;
	struct rlib_report_break *rb;
	struct rlib_element *xxx, *be;
	struct rlib_break_fields *bf;

	for(xxx =e; xxx != NULL; xxx=xxx->next)
		count++;

	for(i=count;i > 0;i--) {
		xxx = e;
		for(j=0;j<i-1;j++)
			xxx = xxx->next;		
		rb = xxx->data;
		for(be = rb->fields; be != NULL; be=be->next) {
			bf = be->data;
			rlib_value_free(bf->rval);
			bf->rval = NULL;
		}
		if(OUTPUT(r)->do_break) {
			rlib_end_page_if_line_wont_fit(r, part, report, rb->footer);
			if(!INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result))
				rlib_navigate_previous(r, r->current_result);
			rlib_print_break_footer_output(r, part, report, rb, rb->footer, FALSE);
			if(!INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result))
				rlib_navigate_next(r, r->current_result);
		}

		rlib_reset_variables_on_break(r, part, report, rb->name);
		rlib_process_expression_variables(r, report);
		if(rb->newpage) {
			do_endpage = TRUE;
		}
	}
	if(do_endpage && OUTPUT(r)->do_break) {
		if(!INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result)) {
			OUTPUT(r)->end_page(r, part);
			rlib_force_break_headers(r, part, report);
		}
	}
}

/*
	Footers are complicated.... I need to go in reverse order for footers... and if I find a match...
	I need to go back down the list and force breaks for everyone.. in reverse order.. ugh

*/
void rlib_handle_break_footers(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	struct rlib_element *e;
	struct rlib_break_fields *bf;

	if(report->breaks == NULL)
		return;
	for(e = report->breaks; e != NULL; e=e->next) {
		gint dobreak=1;
		struct rlib_report_break *rb = e->data;
		struct rlib_element *be;
		for(be = rb->fields; be != NULL; be=be->next) {
			struct rlib_value rval_tmp;
			RLIB_VALUE_TYPE_NONE(&rval_tmp);
			bf = be->data;
			if(dobreak && (INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result) 
				|| rvalcmp(bf->rval, rlib_execute_pcode(r, &rval_tmp, bf->code, NULL)))) {
				dobreak=1;
			} else {
				dobreak = 0;
			}
			rlib_value_free(&rval_tmp);
		}
		
		if(dobreak) {
			rlib_break_all_below_in_reverse_order(r, part, report, e);
			break;
		}
				
	}
}

