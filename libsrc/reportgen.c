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
 *
 * $Id$
 * 
 * This module generates a report from the information stored in the current
 * report object.
 * The main entry point is called once at report generation time for each
 * report defined in the rlib object.
 *
 */
 
#include <stdlib.h>
#include <string.h>
#include <langinfo.h>

#include "config.h"
#include "rlib.h"
#include "pcode.h"
#include "rlib_input.h"

#define FONTPOINT 	10.0

//Not used: static struct rlib_rgb COLOR_BLACK = {0, 0, 0};

static gchar *orientations[] = {
	"",
	"portrait",
	"landscape",
	NULL
};

gchar *align_text(rlib *r, gchar *rtn, gint len, gchar *src, gint align, gint width) {
	g_strlcpy(rtn, src, len);

	if(!OUTPUT(r)->do_align)
		return rtn;

	if(align == RLIB_ALIGN_LEFT || width == -1) {
	} else {
		if(align == RLIB_ALIGN_RIGHT) {        
			gint x = width - r_charcount(src);
			if (x > (len - 1)) x = len - 1;
			if(x > 0) {
				memset(rtn, ' ', x);
				g_strlcpy(rtn+x, src, len - x);
			}
		}
		if(align == RLIB_ALIGN_CENTER) {
			gint x = (width - r_charcount(src))/2;
			if (x > (len - 1)) x = len -1;
			if(x > 0) {
				memset(rtn, ' ', x);
				g_strlcpy(rtn+x, src, len - x);
			}
		}
	}
	return rtn;
}
	
gint get_font_point(rlib *r, struct rlib_report_lines *rl) {
	if(rl->font_point == -1)
		return r->font_point;
	else
		return rl->font_point;
}	


#if 0
/**
 * break the string txt into a RVector of individual strings that will fit
 * the width. Use RVector_size to count the # of lines returned.
 */
RVector *wrap_memo_lines(gchar *txt, gint width, const gchar *wrapchars) {
	gint len;
	gchar *tptr, *endptr, *ptr;
	RVector *v = RVector_new();
	
	do {
		if (r_charcount(txt) < width) {
			RVector_add(v, g_strdup(txt));
			break;
		} else {
			endptr = ptr = txt + width;
			while (ptr > txt) {
				if ((tptr = strchr(wrapchars, *ptr))) {
					len = ptr - txt;
					tptr = g_malloc(len + 1);
					strncpy(tptr, txt, len);
					tptr[len] = '\0';
					RVector_add(v, tptr);
					endptr = ptr;
				}
				--ptr;
			}
			txt = endptr;
		}
	} while (TRUE);
	return v;
}


/**
 * Frees all memory allocated for a memo lines vector
 */
void free_memo_lines(RVector *v) {
	gint i, lim = RVector_size(v);

	for (i = 0; i < lim; ++i) {
		g_free(RVector_get(v, i));
	}
	RVector_free(v);
}
#endif


gint calc_memo_lines(struct rlib_report_lines *rl) {
	struct rlib_element *e;
//	int hasmemo;
	gint nlines = 0;
//	RVector *v;
	
	for (e = rl->e; e != NULL; e = e->next) {
		if (e->type == RLIB_ELEMENT_FIELD) {
			//
//			if (e->xml_maxlines)
//			v = wrapMemoLines(
//			if (e->maxlines != 1) {
//				hasmemo = TRUE;
//			}
		}
	}
	return nlines;
}

void rlib_handle_page_footer(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	gint i;

	for(i=0; i < report->pages_across; i++) {
		report->bottom_size[i] = get_outputs_size(r, report->page_footer, i);
		report->position_bottom[i] -= report->bottom_size[i];
	}

	rlib_layout_report_output(r, part, report, report->page_footer, TRUE);
	
	for(i=0; i<report->pages_across; i++)
		report->position_bottom[i] -= report->bottom_size[i];
}

/*void rlib_init_page(rlib *r, struct rlib_part *part, struct rlib_report *report, gchar report_header) {
	gint i;
	for(i=0;i<report->pages_across;i++)
		report->position_top[i] = report->top_margin;
	r->current_font_point = -1;
	OUTPUT(r)->start_new_page(r, part);
	OUTPUT(r)->set_font_point(r, r->font_point);
	if(report_header)
		rlib_layout_report_output(r, part, report, report->report_header, FALSE);	
	
	if(!(r->current_page_number == 1 && report->suppress_page_header_first_page == TRUE))
		rlib_layout_report_output(r, part, report, report->page_header, FALSE);
	rlib_layout_report_output(r, part, report, report->detail.textlines, FALSE);		
	rlib_handle_page_footer(r, part, report);

	OUTPUT(r)->init_end_page(r);
}*/

gfloat get_output_size(rlib *r, struct rlib_report_output_array *roa) {
	gint j;
	gfloat total=0;
	for(j=0;j<roa->count;j++) {
		struct rlib_report_output *rd = roa->data[j];
		if(rd->type == REPORT_PRESENTATION_DATA_LINE) {
			struct rlib_report_lines *rl = rd->data;
			total += RLIB_GET_LINE(get_font_point(r, rl));
			
//Here to adjust size of memo field output.			
		} else if(rd->type == REPORT_PRESENTATION_DATA_HR) {
			struct rlib_report_horizontal_line *rhl = rd->data;
			total += RLIB_GET_LINE(rhl->size);		
		}
	}
	return total;
}

gfloat get_outputs_size(rlib *r, struct rlib_element *e, gint page) {
	gfloat total=0;
	struct rlib_report_output_array *roa;

	for(; e != NULL; e=e->next) {
		roa = e->data;
		if(roa->page == -1 || roa->page == page || roa->page == -1)
			total += get_output_size(r, roa);
	}			

	return total;
}


gint will_this_fit(rlib *r, struct rlib_part *part, struct rlib_report *report, gfloat total, gint page) {
	if(OUTPUT(r)->paginate == FALSE)
		return TRUE;
	if(report->position_top[page-1]+total > report->position_bottom[page-1])
		return FALSE;
	else
		return TRUE;
}

gint will_outputs_fit(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_element *e, gint page) {
	gfloat size = 0;
	struct rlib_report_output_array *roa;

	if(OUTPUT(r)->paginate == FALSE)
		return TRUE;
	if(e == NULL)
		return TRUE;
	for(; e != NULL; e=e->next) {
		roa = e->data;
		if(page == -1 || page == roa->page || roa->page == -1)
			size += get_output_size(r, roa);
	}			
	return will_this_fit(r, part, report, size, page);
}

void rlib_set_report_from_part(rlib *r, struct rlib_part *part, struct rlib_report *report, gfloat top_margin_offset) {
	gint i;
	for(i=0;i<report->pages_across;i++) {
		report->position_top[i] = report->top_margin + part->position_top[0] + top_margin_offset;
		report->bottom_size[i] = part->bottom_size[0];
		report->position_bottom[i] = part->position_bottom[0];
	}

}

gint rlib_end_page_if_line_wont_fit(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_element *e) {
	gint i, fits=TRUE;	
	for(i=0;i<report->pages_across;i++) {
		if(!will_outputs_fit(r,part, report, e, i+1))
			fits=FALSE;
	}
	if(!fits) {
		if(report->raw_page_number < r->current_page_number) {
			OUTPUT(r)->end_page_again(r, part, report);
			report->raw_page_number++;
			OUTPUT(r)->set_raw_page(r, part, report->raw_page_number);
		} else {
			OUTPUT(r)->end_page(r, part);
			rlib_layout_init_part_page(r, part);
			report->raw_page_number++;
		}
		rlib_set_report_from_part(r, part, report, 0);
		rlib_layout_init_report_page(r, part, report);
	}
	return !fits;
}

gint rlib_fetch_first_rows(rlib *r) {
	gint i;
	gint result = TRUE;
	for(i=0;i<r->queries_count;i++) {
		if(INPUT(r,i)->first(INPUT(r,i), r->results[i].result) == FALSE) {
			result = FALSE;
		}
	}
	return result;
}

static void rlib_init_variables(rlib *r, struct rlib_report *report) {
	struct rlib_element *e;
	for(e = report->variables; e != NULL; e=e->next) {
		struct rlib_report_variable *rv = e->data;
		if(rv->type == REPORT_VARIABLE_EXPRESSION) {
			rv->data = g_malloc(sizeof(struct rlib_count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_COUNT) {
			rv->data = g_malloc(sizeof(struct rlib_count_amount));
			RLIB_VARIABLE_CA(rv)->count = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->count, 0);
		} else if(rv->type == REPORT_VARIABLE_SUM) {
			rv->data = g_malloc(sizeof(struct rlib_count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_AVERAGE) {
			rv->data = g_malloc(sizeof(struct rlib_count_amount));
			RLIB_VARIABLE_CA(rv)->count = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->count, 0);
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_LOWEST) {
			rv->data = g_malloc(sizeof(struct rlib_count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_HIGHEST) {
			rv->data = g_malloc(sizeof(struct rlib_count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		}
	}
	
}

static void rlib_process_variables(rlib *r, struct rlib_report *report) {
	struct rlib_element *e;
	for(e = report->variables; e != NULL; e=e->next) {
		struct rlib_report_variable *rv = e->data;
		struct rlib_value *count = &RLIB_VARIABLE_CA(rv)->count;
		struct rlib_value *amount = &RLIB_VARIABLE_CA(rv)->amount;
		struct rlib_value execute_result, *er = &execute_result;
		if(rv->code != NULL)
			 rlib_execute_pcode(r, &execute_result, rv->code, NULL);
		if(rv->type == REPORT_VARIABLE_COUNT) {
			RLIB_VALUE_GET_AS_NUMBER(count) += RLIB_DECIMAL_PRECISION;
		} else if(rv->type == REPORT_VARIABLE_EXPRESSION) {
			if(RLIB_VALUE_IS_NUMBER(er)) {
				rlib_value_free(amount);
				rlib_value_new_number(amount, RLIB_VALUE_GET_AS_NUMBER(er));
			} else if (RLIB_VALUE_IS_STRING(er)) {
				rlib_value_free(amount);
				rlib_value_new_string(amount, RLIB_VALUE_GET_AS_STRING(er));
			} else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER OR STRING FOR REPORT_VARIABLE_EXPRESSION\n");
		} else if(rv->type == REPORT_VARIABLE_SUM) {
			if(RLIB_VALUE_IS_NUMBER(er))
				RLIB_VALUE_GET_AS_NUMBER(amount) += RLIB_VALUE_GET_AS_NUMBER(er);
			else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_SUM\n");
		} else if(rv->type == REPORT_VARIABLE_AVERAGE) {
			RLIB_VALUE_GET_AS_NUMBER(count) += RLIB_DECIMAL_PRECISION;
			if(RLIB_VALUE_IS_NUMBER(er))
				RLIB_VALUE_GET_AS_NUMBER(amount) += RLIB_VALUE_GET_AS_NUMBER(er);
			else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_AVERAGE\n");
		} else if(rv->type == REPORT_VARIABLE_LOWEST) {
			if(RLIB_VALUE_IS_NUMBER(er)) {
				if(RLIB_VALUE_GET_AS_NUMBER(er) < RLIB_VALUE_GET_AS_NUMBER(amount) || RLIB_VALUE_GET_AS_NUMBER(amount) == 0) //TODO: EVIL HACK
					RLIB_VALUE_GET_AS_NUMBER(amount) = RLIB_VALUE_GET_AS_NUMBER(er);
			} else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_LOWEST\n");
		} else if(rv->type == REPORT_VARIABLE_HIGHEST) {
			if(RLIB_VALUE_IS_NUMBER(er)) {
				if(RLIB_VALUE_GET_AS_NUMBER(er) > RLIB_VALUE_GET_AS_NUMBER(amount) || RLIB_VALUE_GET_AS_NUMBER(amount) == 0) //TODO: EVIL HACK
					RLIB_VALUE_GET_AS_NUMBER(amount) = RLIB_VALUE_GET_AS_NUMBER(er);
			} else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_HIGHEST\n");
		}
	}
	
}

static void rlib_evaluate_report_attributes(rlib *r, struct rlib_report *report) {
	gint t;
	gfloat f;
	
	if (rlib_execute_as_int_inlist(r, report->orientation_code, &t, orientations))
		if ((t == RLIB_ORIENTATION_PORTRAIT) || (t == RLIB_ORIENTATION_LANDSCAPE))
			report->orientation = t;
	if (rlib_execute_as_int(r, report->font_size_code, &t))
		report->font_size = t;
	if(report->is_the_only_report) {
		report->top_margin = 0;
		report->bottom_margin = 0;
		report->left_margin = 0;	
	} else {
		if (rlib_execute_as_float(r, report->top_margin_code, &f))
			report->top_margin = f;
		if (rlib_execute_as_float(r, report->left_margin_code, &f))
			report->left_margin = f;
		if (rlib_execute_as_float(r, report->bottom_margin_code, &f))
			report->bottom_margin = f;
	}
	if (rlib_execute_as_int(r, report->pages_across_code, &t))
		report->pages_across = t;
	if (rlib_execute_as_int(r, report->suppress_page_header_first_page_code, &t))
		report->suppress_page_header_first_page = t;
}

static void rlib_evaluate_break_attributes(rlib *r, struct rlib_report *report) {
	struct rlib_report_break *rb;
	struct rlib_element *e;
	gint t;
	
	for(e = report->breaks; e != NULL; e = e->next) {
		rb = e->data;
		if (rlib_execute_as_boolean(r, rb->newpage_code, &t))
			rb->newpage = t;
		if (rlib_execute_as_boolean(r, rb->headernewpage_code, &t))
			rb->headernewpage = t;
		if (rlib_execute_as_boolean(r, rb->suppressblank_code, &t))
			rb->suppressblank = t;
	}
}

void rlib_process_expression_variables(rlib *r, struct rlib_report *report) {
	struct rlib_element *e;
	for(e = report->variables; e != NULL; e=e->next) {
		struct rlib_report_variable *rv = e->data;
		struct rlib_value *amount = &RLIB_VARIABLE_CA(rv)->amount;
		struct rlib_value execute_result, *er = &execute_result;
		if(rv->code != NULL)
			 rlib_execute_pcode(r, &execute_result, rv->code, NULL);
		if(rv->type == REPORT_VARIABLE_EXPRESSION) {
			if(RLIB_VALUE_IS_NUMBER(er)) {
				rlib_value_free(amount);
				rlib_value_new_number(amount, RLIB_VALUE_GET_AS_NUMBER(er));
			} else if (RLIB_VALUE_IS_STRING(er)) {
				rlib_value_free(amount);
				rlib_value_new_string(amount, RLIB_VALUE_GET_AS_STRING(er));
			} else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER OR STRING FOR REPORT_VARIABLE_EXPRESSION\n");
		}
	}
	
}

static void rlib_evaluate_part_attributes(rlib *r, struct rlib_part *part) {
	gint t;
	gfloat f;
	char buf[MAXSTRLEN];
	
	if (rlib_execute_as_int_inlist(r, part->orientation_code, &t, orientations))
		if ((t == RLIB_ORIENTATION_PORTRAIT) || (t == RLIB_ORIENTATION_LANDSCAPE))
			part->orientation = t;
	if (rlib_execute_as_int(r, part->font_size_code, &t))
		part->font_size = t;
	if (rlib_execute_as_float(r, part->top_margin_code, &f))
		part->top_margin = f;
	if (rlib_execute_as_float(r, part->left_margin_code, &f))
		part->left_margin = f;
	if (rlib_execute_as_float(r, part->bottom_margin_code, &f))
		part->bottom_margin = f;
	if (rlib_execute_as_float(r, part->pages_across_code, &f))
		part->pages_across = f;
	if (rlib_execute_as_string(r, part->paper_type_code, buf, MAXSTRLEN)) {
		struct rlib_paper *paper = rlib_layout_get_paper_by_name(r, buf);
		if(paper != NULL)
			part->paper = paper;
	}
}

void rlib_layout_report(rlib *r, struct rlib_part *part, struct rlib_report *report, gfloat left_margin_offset, gfloat top_margin_offset) {
	gint processed_variables;
	gint i;
	char query[MAXSTRLEN];
	gint report_percent;
	gfloat at_least = 0.0, origional_position_top;
				
	report->query_code = rlib_infix_to_pcode(r, report, report->xml_query);
	if(report->query_code != NULL) {
		rlib_execute_as_string(r, report->query_code, query, MAXSTRLEN);
		for(i=0;i<r->queries_count;i++) {
			if(!strcmp(r->results[i].name, query)) {
				r->current_result = i;		
				break;
			}
		}
	} else {
		r->current_result = 0;
	}

	rlib_resolve_report_fields(r, report);
	rlib_init_variables(r, report);
	rlib_process_variables(r, report);
	processed_variables = TRUE;
	rlib_evaluate_report_attributes(r, report);

	rlib_set_report_from_part(r, part, report, top_margin_offset);

	report->left_margin += left_margin_offset + part->left_margin;
rlogit("LEFT MARGIN IS NOW %f\n", report->left_margin);

	if(report->font_size != -1)
		r->font_point = report->font_size;

	if(rlib_execute_as_int(r, report->height_code, &report_percent)) {
		at_least = (part->position_bottom[0] - part->position_top[0]) * ((gfloat)report_percent/100);					
	}

	origional_position_top = report->position_top[0];

	rlib_layout_report_output(r, part, report, report->report_header, FALSE);
	rlib_layout_init_report_page(r, part, report);
	r->detail_line_count = 0;
/*		if (!first_result) {
			rlib_layout_report_output(r, rr->alternate.nodata, FALSE);
*/			
	INPUT(r,r->current_result)->first(INPUT(r,r->current_result), r->results[r->current_result].result);

	if(!INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result)) {
		while (1) {
			gint output_count = 0;

			if(!processed_variables) {
				rlib_process_variables(r, report);
				processed_variables = TRUE;
			}
			rlib_evaluate_break_attributes(r, report);
			rlib_handle_break_headers(r, part, report);

			if(rlib_end_page_if_line_wont_fit(r, part, report, report->detail.fields))
				rlib_force_break_headers(r, part, report);

			if(OUTPUT(r)->do_break)
				output_count = rlib_layout_report_output(r, part, report, report->detail.fields, FALSE);
			else
				output_count = rlib_layout_report_output_with_break_headers(r, part, report);

			if(output_count > 0)
				r->detail_line_count++;

			if(rlib_navigate_next(r, r->current_result) == FALSE) {
				rlib_navigate_last(r, r->current_result);
				rlib_handle_break_footers(r, part, report);
				break;
			} 

			rlib_evaluate_break_attributes(r, report);
			rlib_handle_break_footers(r, part, report);
			processed_variables = FALSE;
		}
	}
	rlib_navigate_last(r, r->current_result);
	rlib_layout_report_footer(r, part, report);

	if(at_least > 0) {
		gfloat used = (report->position_bottom[0]-origional_position_top)-(report->position_bottom[0]-report->position_top[0]);
		if(used < at_least) {
			for(i=0;i<report->pages_across;i++)
				report->position_top[i] += (at_least-used);
		}
	}

}

struct rlib_report_position {
	long page;
	gfloat position_top;
};

void rlib_layout_part_td(rlib *r, struct rlib_part *part, struct rlib_element *e_td, long page_number, gfloat position_top, struct rlib_report_position *rrp) {
	struct rlib_element *e, *td_contents;
	
	gfloat paper_width = rlib_layout_get_page_width(r, part) - (part->left_margin * 2);
	gfloat running_left_margin = 0;
	
	for(e=e_td;e != NULL;e=e->next) {
		gfloat running_top_margin = 0;
		struct rlib_part_td *td = e->data;
		gint width, height, border_width;
		gchar border_color[MAXSTRLEN];
		struct rlib_rgb bgcolor;
		
		if(!rlib_execute_as_int(r, td->width_code, &width))
			width = 100;

		if(!rlib_execute_as_int(r, td->height_code, &height))
			height = 0;

		if(!rlib_execute_as_int(r, td->border_width_code, &border_width))
			border_width = 0;
			
		if(!rlib_execute_as_string(r, td->border_color_code, border_color, MAXSTRLEN))
			border_color[0] = 0;

		parsecolor(&bgcolor, border_color);
		
		OUTPUT(r)->start_td(r, part, running_left_margin+part->left_margin, rlib_layout_get_next_line(r, part, running_top_margin+position_top+part->position_top[0], 0), width,  height, border_width, border_color[0] == 0 ? NULL : &bgcolor);

		for(td_contents=td->e;td_contents != NULL;td_contents=td_contents->next) {
			if(td_contents->type == RLIB_ELEMENT_REPORT) {
				struct rlib_report *report = td_contents->data;
				
				report->page_width = (((gfloat)width/100) * paper_width);
				OUTPUT(r)->set_raw_page(r, part, page_number);
				report->raw_page_number = page_number;
				rlib_layout_report(r, part, report, running_left_margin, running_top_margin+position_top);
				running_top_margin = report->position_top[0] - part->position_top[0];
				if(report->raw_page_number > rrp->page) {
					rrp->page = report->raw_page_number;
					rrp->position_top = report->position_top[0];				
				} else if(report->raw_page_number == rrp->page) {
					if(report->position_top[0] > rrp->position_top)
						rrp->position_top = report->position_top[0];
				}
			} else {
				r_error("UNKNOWN ELEMENT IN TD\n");
			}
		}
		rlogit("========== TD WIDTH WAS %f\n", (((gfloat)width/100) * paper_width));
		running_left_margin += (((gfloat)width/100) * paper_width);
		OUTPUT(r)->end_td(r);
	}	
}

void rlib_layout_part_tr(rlib *r, struct rlib_part *part, struct rlib_element *e_tr) {
	struct rlib_report_position rrp;
	struct rlib_element *e;
	char buf[MAXSTRLEN];
	
	bzero(&rrp, sizeof(rrp));

	for(e=e_tr;e != NULL;e=e->next) {
		struct rlib_part_tr *tr = e->data;
		gfloat save_position_top = 0;
		long save_page_number;
		gint newpage; 

		if(rlib_execute_as_boolean(r, tr->newpage_code, &newpage)) {
			if(newpage && OUTPUT(r)->paginate) {
				OUTPUT(r)->end_page(r, part);
				rlib_layout_init_part_page(r, part);
				bzero(&rrp, sizeof(rrp));
			}
		}
		
		OUTPUT(r)->start_tr(r);
		save_page_number = r->current_page_number;
		
		if(rrp.position_top > 0)
			save_position_top = rrp.position_top - part->position_top[0];

		tr->layout = RLIB_LAYOUT_FLOW;
		if(rlib_execute_as_string(r, tr->layout_code, buf, MAXSTRLEN) && strcmp(buf, "fixed") == 0)
			tr->layout = RLIB_LAYOUT_FIXED;

		rlogit("  TR [%f] [%s]\n", rrp.position_top, tr->layout == RLIB_LAYOUT_FIXED ? "fixed" : "flow");

		bzero(&rrp, sizeof(rrp));

		rlib_layout_part_td(r, part, tr->e, save_page_number, save_position_top, &rrp);
		OUTPUT(r)->end_tr(r);
		
	}	
}

gint make_report(rlib *r) {
	gint i = 0;
//	gint processed_variables = FALSE;
	gint first_result;
	if(r->format == RLIB_FORMAT_HTML)
		rlib_html_new_output_filter(r);
	else if(r->format == RLIB_FORMAT_TXT)
		rlib_txt_new_output_filter(r);
	else if(r->format == RLIB_FORMAT_CSV)
		rlib_csv_new_output_filter(r);
#ifdef HAVE_LIBCPDF
	else
		rlib_pdf_new_output_filter(r);
#endif
	r->current_font_point = -1;

	OUTPUT(r)->set_fg_color(r, -1, -1, -1);
	OUTPUT(r)->set_bg_color(r, -1, -1, -1);

	r->current_page_number = 1;
	r->current_result = 0;
	r->start_of_new_report = TRUE;

	OUTPUT(r)->init_output(r);
	first_result = rlib_fetch_first_rows(r);

	r->current_output_encoder = NULL;
	for(i=0;i<r->parts_count;i++) {
		struct rlib_part *part = r->parts[i];

		rlib_resolve_part_fields(r, part);
		rlib_evaluate_part_attributes(r, part);
		OUTPUT(r)->start_report(r, part);
		if(part->font_size != -1)
			r->font_point = part->font_size;

		rlib_layout_init_part_page(r, part);
		rlib_layout_part_tr(r, part, part->tr_elements);
		OUTPUT(r)->end_report(r, part);
	}
	
/*
		if(report+1 < r->reports_count) {
			r->current_page_number++;
			r->start_of_new_report = TRUE;
			r->current_line_number = 1;
			r->detail_line_count = 0;
			r->font_point = FONTPOINT;
		}
		rlib_char_encoder_destroy(&rr->output_encoder); //Destroy if was one.
		rlib_char_encoder_destroy(&rr->db_encoder); //Destroy if was one.
		rlib_char_encoder_destroy(&rr->param_encoder); //Destroy if was one.
	}
	*/
	return 0;
}

gint rlib_finalize(rlib *r) {
	OUTPUT(r)->finalize_private(r);
	return 0;
}

