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
	if(strlen(name) >= 3 && name[0] == 'r' && name[1] == '.') {
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
	return INPUT(r, rf->resultset)->get_field_value_as_string(INPUT(r, rf->resultset), r->results[rf->resultset].result , rf->field);
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
			if(!isdigit(*result_name))
				rlogit("rlib_resolve_namevalue: INVALID RESULT SET %s, name was [%s]\n", result_name, name);
		}
		g_free(result_name);
	}
	*rtn_field = INPUT(r, resultset)->resolve_field_pointer(INPUT(r, resultset), r->results[resultset].result, name);
	
	if(*rtn_field != NULL)
		found = TRUE;
	*rtn_resultset = resultset;
	return found;
}

static gint getalign(char *align) {
	if(align == NULL)
		return RLIB_ALIGN_LEFT;

	if(!strcmp(align, "right"))
		return RLIB_ALIGN_RIGHT;
	else if(!strcmp(align, "center"))
		return RLIB_ALIGN_CENTER;
	else
		return RLIB_ALIGN_LEFT;

}

static void rlib_field_resolve_pcode(rlib *r, struct report_field *rf) {
	rf->code = rlib_infix_to_pcode(r, rf->value);
	rf->format_code = rlib_infix_to_pcode(r, rf->format);
	rf->link_code = rlib_infix_to_pcode(r, rf->link);
	rf->color_code = rlib_infix_to_pcode(r, rf->color);
	rf->bgcolor_code = rlib_infix_to_pcode(r, rf->bgcolor);
	rf->col_code = rlib_infix_to_pcode(r, rf->col);
	rf->wrapchars_code = rlib_infix_to_pcode(r, rf->xml_wrapchars);
	rf->maxlines_code = rlib_infix_to_pcode(r, rf->xml_maxlines);
	
	rf->width = -1;
	if(rf->xml_width != NULL)
		rf->width = atol(rf->xml_width);

	rf->align = getalign(rf->xml_align);

/*rlogit("DUMPING PCODE FOR [%s]\n", rf->value);
rlib_pcode_dump(rf->code,0);	
rlogit("\n\n");*/
}

static void rlib_text_resolve_pcode(rlib *r, struct report_text *rt) {
	rt->color_code = rlib_infix_to_pcode(r, rt->color);
	rt->bgcolor_code = rlib_infix_to_pcode(r, rt->bgcolor);
	rt->col_code = rlib_infix_to_pcode(r, rt->col);

	rt->width = -1;
	if(rt->xml_width != NULL)
		rt->width = atol(rt->xml_width);

	rt->align = getalign(rt->xml_align);

}

static void rlib_break_resolve_pcode(rlib *r, struct break_fields *bf) {
	if(bf->value == NULL)
		rlogit("RLIB ERROR: BREAK FIELD VALUE CAN NOT BE NULL\n");
	bf->code = rlib_infix_to_pcode(r, bf->value);
}

static void rlib_variable_resolve_pcode(rlib *r, struct report_variable *rv) {
	rv->code = rlib_infix_to_pcode(r, rv->value);
/*rlogit("DUMPING PCODE FOR [%s]\n", rv->value);
rlib_pcode_dump(rv->code,0);	
rlogit("\n\n");*/
}

static void rlib_hr_resolve_pcode(rlib *r, struct report_horizontal_line * rhl) {
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

static void rlib_image_resolve_pcode(rlib *r, struct report_image * ri) {
	ri->value_code = rlib_infix_to_pcode(r, ri->value);
	ri->type_code = rlib_infix_to_pcode(r, ri->type);
	ri->width_code = rlib_infix_to_pcode(r, ri->width);
	ri->height_code = rlib_infix_to_pcode(r, ri->height);
}

static void rlib_resolve_fields2(rlib *r, struct report_output_array *roa) {
	gint j;
	struct report_element *e;
	
	if(roa == NULL)
		return;
		
	if(roa->xml_page != NULL)
		roa->page = atol(roa->xml_page);
	else
		roa->page = -1;
	
	for(j=0;j<roa->count;j++) {
		struct report_output *ro = roa->data[j];
		
		if(ro->type == REPORT_PRESENTATION_DATA_LINE) {
			struct report_lines *rl = ro->data;	
			e = rl->e;
			rl->bgcolor_code = rlib_infix_to_pcode(r, rl->bgcolor);
			rl->color_code = rlib_infix_to_pcode(r, rl->color);
			rl->suppress_code = rlib_infix_to_pcode(r, rl->suppress);

			for(; e != NULL; e=e->next) {
				if(e->type == REPORT_ELEMENT_FIELD) {
					rlib_field_resolve_pcode(r, ((struct report_field *)e->data));
				} else if(e->type == REPORT_ELEMENT_TEXT) {
					rlib_text_resolve_pcode(r, ((struct report_text *)e->data));
				}
			}
		} else if(ro->type == REPORT_PRESENTATION_DATA_HR) {
			rlib_hr_resolve_pcode(r, ((struct report_horizontal_line *)ro->data));
		} else if(ro->type == REPORT_PRESENTATION_DATA_IMAGE) {
			rlib_image_resolve_pcode(r, ((struct report_image *)ro->data));
		}
	}
}

static void rlib_resolve_outputs(rlib *r, struct report_element *e) {
	struct report_output_array *roa;
	for(; e != NULL; e=e->next) {
		roa = e->data;
		rlib_resolve_fields2(r, roa);
	}			

}

/*
	Report variables are refereced as v.whatever
	but when created in the <variables/> section they use there normal name.. ie.. whatever
*/
struct report_variable *rlib_resolve_variable(rlib *r, gchar *name) {
	struct report_element *e;
	if(strlen(name) >= 3 && name[0] == 'v' && name[1] == '.') {
		name += 2;
		for(e = r->reports[r->current_report]->variables; e != NULL; e=e->next) {
			struct report_variable *rv = e->data;
			if(!strcmp(name, rv->name))
				return rv;
		}	
		rlogit("rlib_resolve_variable: Could not find [%s]\n", name);
	}
	return NULL;
}

void rlib_resolve_fields(rlib *r) {
	struct report_element *e;
	if(r->reports[r->current_report]->xml_top_margin == NULL)
		r->reports[r->current_report]->top_margin = DEFAULT_TOP_MARGIN;
	else
		r->reports[r->current_report]->top_margin = atof(r->reports[r->current_report]->xml_top_margin);

	if(r->reports[r->current_report]->xml_orientation == NULL)
		r->reports[r->current_report]->orientation = RLIB_ORIENTATION_PORTRAIT;
	else if(!strcmp(r->reports[r->current_report]->xml_orientation, "landscape"))
		r->reports[r->current_report]->orientation = RLIB_ORIENTATION_LANDSCAPE;
	else
		r->reports[r->current_report]->orientation = RLIB_ORIENTATION_PORTRAIT;
	
	
	if(r->reports[r->current_report]->xml_font_size == NULL)
		r->reports[r->current_report]->font_size = -1;
	else
		r->reports[r->current_report]->font_size = atol(r->reports[r->current_report]->xml_font_size);

	if(r->reports[r->current_report]->xml_top_margin == NULL)
		r->reports[r->current_report]->top_margin = DEFAULT_TOP_MARGIN;
	else
		r->reports[r->current_report]->top_margin = atof(r->reports[r->current_report]->xml_top_margin);

	if(r->reports[r->current_report]->xml_left_margin == NULL)
		r->reports[r->current_report]->left_margin = DEFAULT_LEFT_MARGIN;
	else
		r->reports[r->current_report]->left_margin = atof(r->reports[r->current_report]->xml_left_margin);

	if(r->reports[r->current_report]->xml_bottom_margin == NULL)
		r->reports[r->current_report]->bottom_margin = DEFAULT_BOTTOM_MARGIN;
	else
		r->reports[r->current_report]->bottom_margin = atof(r->reports[r->current_report]->xml_bottom_margin);

	if(r->reports[r->current_report]->xml_pages_accross == NULL)
		r->reports[r->current_report]->pages_accross = 1;
	else
		r->reports[r->current_report]->pages_accross = atol(r->reports[r->current_report]->xml_pages_accross);

	r->reports[r->current_report]->suppress_page_header_first_page = FALSE;
	if(r->reports[r->current_report]->xml_suppress_page_header_first_page != NULL &&
		!strcmp(r->reports[r->current_report]->xml_suppress_page_header_first_page, "yes"))
		r->reports[r->current_report]->suppress_page_header_first_page = TRUE;
		
	r->reports[r->current_report]->position_top = g_malloc(r->reports[r->current_report]->pages_accross * sizeof(float));
	r->reports[r->current_report]->position_bottom = g_malloc(r->reports[r->current_report]->pages_accross * sizeof(float));
	r->reports[r->current_report]->bottom_size = g_malloc(r->reports[r->current_report]->pages_accross * sizeof(float));

	rlib_resolve_outputs(r, r->reports[r->current_report]->report_header);
	rlib_resolve_outputs(r, r->reports[r->current_report]->page_header);
	rlib_resolve_outputs(r, r->reports[r->current_report]->page_footer);
	rlib_resolve_outputs(r, r->reports[r->current_report]->report_footer);

	rlib_resolve_outputs(r, r->reports[r->current_report]->detail.fields);
	rlib_resolve_outputs(r, r->reports[r->current_report]->detail.textlines);

	if(r->reports[r->current_report]->breaks != NULL) {
		for(e = r->reports[r->current_report]->breaks; e != NULL; e=e->next) {
			struct report_break *rb = e->data;
			struct report_element *be;
			rlib_resolve_outputs(r, rb->header);
			rlib_resolve_outputs(r, rb->footer);
			rb->newpage = FALSE;
			rb->headernewpage = FALSE;
			rb->suppressblank = FALSE;
			if(rb->xml_newpage != NULL && rb->xml_newpage[0] != '\0' && !strcmp(rb->xml_newpage, "yes"))
				rb->newpage = TRUE;
			if(rb->xml_headernewpage != NULL && rb->xml_headernewpage[0] != '\0' && !strcmp(rb->xml_headernewpage, "yes"))
				rb->headernewpage = TRUE;
			if(rb->xml_suppressblank != NULL && rb->xml_suppressblank[0] != '\0' && !strcmp(rb->xml_suppressblank, "yes"))
				rb->suppressblank = TRUE;
			for(be = rb->fields; be != NULL; be=be->next) {
				struct break_fields *bf = be->data;
				rlib_break_resolve_pcode(r, bf);
			}		
		}
	}
	
	if(r->reports[r->current_report]->variables != NULL) {
		for(e = r->reports[r->current_report]->variables; e != NULL; e=e->next) {
			struct report_variable *rv = e->data;
			rlib_variable_resolve_pcode(r, rv);
		}
	}
}

gchar * rlib_resolve_memory_variable(rlib *r, gchar *name) {
	if(strlen(name) >= 3 && name[0] == 'm' && name[1] == '.') {
		if (r->htParameters) {
			gchar *result = rlib_hashtable_lookup(r->htParameters, name + 2);
			if (result) return result;
		}
		return ENVIRONMENT(r)->rlib_resolve_memory_variable(name+2);
	}
	return NULL;
}
