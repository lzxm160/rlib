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
#include <ctype.h>

#include "config.h"
#include "rlib.h"
#include "pcode.h"
#include "rlib_input.h"

/*
	RLIB needs to find direct pointer to result sets and fields ahead of time so
	during executing time things are fast.. The following functions will resolve
	fields and variables into almost direct pointers

	Rlib variables are internal variables to rlib that it exports to you to use in reports
	Right now I only export pageno, value (which is a pointer back to the field value resolved), lineno, detailcnt
	Rlib variables should be referecned as r.whatever
	In this case r.pageno

*/
gint rlib_resolve_rlib_variable(rlib *r, gchar *name) {
	if(r_bytecount(name) >= 3 && name[0] == 'r' && name[1] == '.') {
		name += 2;
		if(!strcmp(name, "pageno"))
			return RLIB_RLIB_VARIABLE_PAGENO;
		else if(!strcmp(name, "value"))
			return RLIB_RLIB_VARIABLE_VALUE;
		else if(!strcmp(name, "lineno"))
			return RLIB_RLIB_VARIABLE_LINENO;
		else if(!strcmp(name, "detailcnt"))
			return RLIB_RLIB_VARIABLE_DETAILCNT;
	}
	return 0;
}

gchar * rlib_resolve_field_value(rlib *r, struct rlib_resultset_field *rf) {
	struct input_filter *rs = INPUT(r, rf->resultset);
#if DISABLE_UTF8
	return g_strdup(rs->get_field_value_as_string(rs, r->results[rf->resultset].result, rf->field));
#else
	rlib_char_encoder *enc = (rs->info.encoder)? rs->info.encoder : r->db_encoder;
	return g_strdup((gchar *) rlib_char_encoder_encode(enc, 
			rs->get_field_value_as_string(rs, r->results[rf->resultset].result , rf->field)));
#endif
}

gint rlib_lookup_result(rlib *r, gchar *name) {
	gint i;
	for(i=0;i<r->queries_count;i++) {
		if(!strcmp(r->results[i].name, name))
			return i;
	}
	return -1;
}

gint rlib_resolve_resultset_field(rlib *r, char *name, void **rtn_field, gint *rtn_resultset) {
	gint resultset=0;
	gint found = FALSE;
	gchar *right_side = NULL, *result_name = NULL;

	resultset = r->current_result;
//TODO:localize this so utf8 names work too.
	right_side = memchr(name, '.', strlen(name));
	if(right_side != NULL) {
		gint t;
		result_name = g_malloc(strlen(name) - strlen(right_side) + 1);
		memcpy(result_name, name, strlen(name) - strlen(right_side));
		result_name[strlen(name) - strlen(right_side)] = '\0';
		right_side++;
		name = right_side;
		t = rlib_lookup_result(r, result_name);
		if(t >= 0) {
			found = TRUE;
			resultset = t;
		} else {
			if(!isdigit((int)*result_name))
				rlogit("rlib_resolve_namevalue: INVALID RESULT SET %s, name was [%s]\n", result_name, name);
		}
		g_free(result_name);
	}
	*rtn_field = INPUT(r, resultset)->resolve_field_pointer(INPUT(r, resultset), r->results[resultset].result, name);
	
	if(*rtn_field != NULL)
		found = TRUE;
	else
		r_error("The field [%s.%s] does not exist", result_name, name);
	*rtn_resultset = resultset;
	return found;
}


static void rlib_field_resolve_pcode(rlib *r, struct rlib_report_field *rf) {
	rf->code = rlib_infix_to_pcode(r, rf->value);
	rf->format_code = rlib_infix_to_pcode(r, rf->format);
	rf->link_code = rlib_infix_to_pcode(r, rf->link);
	rf->color_code = rlib_infix_to_pcode(r, rf->color);
	rf->bgcolor_code = rlib_infix_to_pcode(r, rf->bgcolor);
	rf->col_code = rlib_infix_to_pcode(r, rf->col);
	rf->width_code = rlib_infix_to_pcode(r, rf->xml_width);
	rf->align_code = rlib_infix_to_pcode(r, rf->xml_align);
	rf->wrapchars_code = rlib_infix_to_pcode(r, rf->xml_wrapchars);
	rf->maxlines_code = rlib_infix_to_pcode(r, rf->xml_maxlines);
	rf->width = -1;
//rlogit("DUMPING PCODE FOR [%s]\n", rf->value);
//rlib_pcode_dump(rf->code,0);	
//rlogit("\n\n");
}

static void rlib_text_resolve_pcode(rlib *r, struct rlib_report_literal *rt) {
	rt->color_code = rlib_infix_to_pcode(r, rt->color);
	rt->bgcolor_code = rlib_infix_to_pcode(r, rt->bgcolor);
	rt->col_code = rlib_infix_to_pcode(r, rt->col);
	rt->width_code = rlib_infix_to_pcode(r, rt->xml_width);
	rt->align_code = rlib_infix_to_pcode(r, rt->xml_align);
	rt->width = -1;
}

static void rlib_break_resolve_pcode(rlib *r, struct rlib_break_fields *bf) {
	if(bf->value == NULL)
		rlogit("RLIB ERROR: BREAK FIELD VALUE CAN NOT BE NULL\n");
	bf->code = rlib_infix_to_pcode(r, bf->value);
}

static void rlib_variable_resolve_pcode(rlib *r, struct rlib_report_variable *rv) {
	rv->code = rlib_infix_to_pcode(r, rv->value);
/*rlogit("DUMPING PCODE FOR [%s]\n", rv->value);
rlib_pcode_dump(rv->code,0);	
rlogit("\n\n");*/
}

static void rlib_hr_resolve_pcode(rlib *r, struct rlib_report_horizontal_line * rhl) {
	if(rhl->size == NULL)
		rhl->realsize = 0;
	else
		rhl->realsize = atof(rhl->size);
	if(rhl->indent == NULL)
		rhl->realindent = 0;
	else
		rhl->realindent = atof(rhl->indent);
	if(rhl->length == NULL)
		rhl->reallength = 0;
	else
		rhl->reallength = atof(rhl->length);
	rhl->bgcolor_code = rlib_infix_to_pcode(r, rhl->bgcolor);
	rhl->suppress_code = rlib_infix_to_pcode(r, rhl->suppress);
}

static void rlib_image_resolve_pcode(rlib *r, struct rlib_report_image * ri) {
	ri->value_code = rlib_infix_to_pcode(r, ri->value);
	ri->type_code = rlib_infix_to_pcode(r, ri->type);
	ri->width_code = rlib_infix_to_pcode(r, ri->width);
	ri->height_code = rlib_infix_to_pcode(r, ri->height);
}

static void rlib_resolve_fields2(rlib *r, struct rlib_report_output_array *roa) {
	gint j;
	struct rlib_report_element *e;
	
	if(roa == NULL)
		return;
		
	if(roa->xml_page != NULL)
		roa->page = atol(roa->xml_page);
	else
		roa->page = -1;
	
	for(j=0;j<roa->count;j++) {
		struct rlib_report_output *ro = roa->data[j];
		
		if(ro->type == REPORT_PRESENTATION_DATA_LINE) {
			struct rlib_report_lines *rl = ro->data;	
			e = rl->e;
			rl->bgcolor_code = rlib_infix_to_pcode(r, rl->bgcolor);
			rl->color_code = rlib_infix_to_pcode(r, rl->color);
			rl->suppress_code = rlib_infix_to_pcode(r, rl->suppress);

			for(; e != NULL; e=e->next) {
				if(e->type == REPORT_ELEMENT_FIELD) {
					rlib_field_resolve_pcode(r, ((struct rlib_report_field *)e->data));
				} else if(e->type == REPORT_ELEMENT_LITERAL) {
					rlib_text_resolve_pcode(r, ((struct rlib_report_literal *)e->data));
				}
			}
		} else if(ro->type == REPORT_PRESENTATION_DATA_HR) {
			rlib_hr_resolve_pcode(r, ((struct rlib_report_horizontal_line *)ro->data));
		} else if(ro->type == REPORT_PRESENTATION_DATA_IMAGE) {
			rlib_image_resolve_pcode(r, ((struct rlib_report_image *)ro->data));
		}
	}
}

static void rlib_resolve_outputs(rlib *r, struct rlib_report_element *e) {
	struct rlib_report_output_array *roa;
	for(; e != NULL; e=e->next) {
		roa = e->data;
		rlib_resolve_fields2(r, roa);
	}			

}

/*
	Report variables are refereced as v.whatever
	but when created in the <variables/> section they use there normal name.. ie.. whatever
*/
struct rlib_report_variable *rlib_resolve_variable(rlib *r, gchar *name) {
	struct rlib_report_element *e;
//r_debug("Resolving variable [%s]", name);	
	if(r_bytecount(name) >= 3 && name[0] == 'v' && name[1] == '.') {
		name += 2;
		for(e = r->reports[r->current_report]->variables; e != NULL; e=e->next) {
			struct rlib_report_variable *rv = e->data;
		if(!strcmp(name, rv->name))
			return rv;
		}	
		rlogit("rlib_resolve_variable: Could not find [%s]\n", name);
	}
	return NULL;
}


int is_true_str(const gchar *str) {
	if (str == NULL) return FALSE;
	return (!strcasecmp(str, "yes") || !strcasecmp(str, "true"))? TRUE : FALSE;
}


void rlib_resolve_fields(rlib *r) {
	struct rlib_report_element *e;
	struct rlib_report *thisreport = r->reports[r->current_report];

#if 0
	if(thisreport->xml_orientation == NULL)
		thisreport->orientation = RLIB_ORIENTATION_PORTRAIT;
	else if(!strcmp(thisreport->xml_orientation, "landscape"))
		thisreport->orientation = RLIB_ORIENTATION_LANDSCAPE;
	else
		thisreport->orientation = RLIB_ORIENTATION_PORTRAIT;
#endif
	
	thisreport->orientation = RLIB_ORIENTATION_PORTRAIT;
	thisreport->orientation_code = rlib_infix_to_pcode(r, thisreport->xml_orientation);
	thisreport->font_size = -1;
	thisreport->font_size_code = rlib_infix_to_pcode(r, thisreport->xml_font_size);
	thisreport->top_margin = DEFAULT_TOP_MARGIN;
	thisreport->top_margin_code = rlib_infix_to_pcode(r, thisreport->xml_top_margin);
	thisreport->left_margin = DEFAULT_LEFT_MARGIN;
	thisreport->left_margin_code = rlib_infix_to_pcode(r, thisreport->xml_left_margin);
	thisreport->bottom_margin = DEFAULT_BOTTOM_MARGIN;
	thisreport->bottom_margin_code = rlib_infix_to_pcode(r, thisreport->xml_bottom_margin);
	thisreport->paper = rlib_get_paper(r, RLIB_PAPER_LETTER);
	thisreport->paper_type_code = rlib_infix_to_pcode(r, thisreport->xml_paper_type);
	thisreport->pages_accross = 1;
	thisreport->pages_across_code = rlib_infix_to_pcode(r, thisreport->xml_pages_accross);
	thisreport->suppress_page_header_first_page = FALSE;
	thisreport->suppress_page_header_first_page_code = rlib_infix_to_pcode(r, thisreport->xml_suppress_page_header_first_page);
		
	thisreport->position_top = g_malloc(thisreport->pages_accross * sizeof(float));
	thisreport->position_bottom = g_malloc(thisreport->pages_accross * sizeof(float));
	thisreport->bottom_size = g_malloc(thisreport->pages_accross * sizeof(float));

	rlib_resolve_outputs(r, thisreport->report_header);
	rlib_resolve_outputs(r, thisreport->page_header);
	rlib_resolve_outputs(r, thisreport->page_footer);
	rlib_resolve_outputs(r, thisreport->report_footer);
	rlib_resolve_outputs(r, thisreport->detail.fields);
	rlib_resolve_outputs(r, thisreport->detail.textlines);
	rlib_resolve_outputs(r, thisreport->alternate.nodata);

	if(thisreport->breaks != NULL) {
		for(e = thisreport->breaks; e != NULL; e=e->next) {
			struct rlib_report_break *rb = e->data;
			struct rlib_report_element *be;
			
			rlib_resolve_outputs(r, rb->header);
			rlib_resolve_outputs(r, rb->footer);
			rb->newpage_code = rlib_infix_to_pcode(r, rb->xml_newpage);
			rb->headernewpage_code = rlib_infix_to_pcode(r, rb->xml_headernewpage);
			rb->suppressblank_code = rlib_infix_to_pcode(r, rb->xml_suppressblank);
			for(be = rb->fields; be != NULL; be=be->next) {
				struct rlib_break_fields *bf = be->data;
				rlib_break_resolve_pcode(r, bf);
			}		
		}
	}
	
	if(thisreport->variables != NULL) {
		for(e = thisreport->variables; e != NULL; e=e->next) {
			struct rlib_report_variable *rv = e->data;
			rlib_variable_resolve_pcode(r, rv);
		}
	}
}


gchar * rlib_resolve_memory_variable(rlib *r, gchar *name) {
	if(r_bytecount(name) >= 3 && name[0] == 'm' && name[1] == '.') {
		if (r->htParameters) {
			gchar *result = rlib_hashtable_lookup(r->htParameters, name + 2);
			if (result) return g_strdup(result);
		}
		return ENVIRONMENT(r)->rlib_resolve_memory_variable(name+2);
	}
	return NULL;
}
