/*
 *  Copyright (C) 2003-2004 SICOM Systems, INC.
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
		else if(!strcmp(name, "format"))
			return RLIB_RLIB_VARIABLE_FORMAT;
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


static void rlib_field_resolve_pcode(rlib *r, struct rlib_report *report, struct rlib_report_field *rf) {
	rf->code = rlib_infix_to_pcode(r, report, rf->value, TRUE);
	rf->format_code = rlib_infix_to_pcode(r, report, rf->xml_format, TRUE);
	rf->link_code = rlib_infix_to_pcode(r, report, rf->xml_link, TRUE);
	rf->color_code = rlib_infix_to_pcode(r, report, rf->xml_color, TRUE);
	rf->bgcolor_code = rlib_infix_to_pcode(r, report, rf->xml_bgcolor, TRUE);
	rf->col_code = rlib_infix_to_pcode(r, report, rf->xml_col, TRUE);
	rf->width_code = rlib_infix_to_pcode(r, report, rf->xml_width, TRUE);
	rf->bold_code = rlib_infix_to_pcode(r, report, rf->xml_bold, TRUE);
	rf->italics_code = rlib_infix_to_pcode(r, report, rf->xml_italics, TRUE);
	rf->align_code = rlib_infix_to_pcode(r, report, rf->xml_align, TRUE);
	rf->memo_code = rlib_infix_to_pcode(r, report, rf->xml_memo, TRUE);
	rf->memo_height_code = rlib_infix_to_pcode(r, report, rf->xml_memo_height, TRUE);
	rf->memo_wrap_chars_code = rlib_infix_to_pcode(r, report, rf->xml_memo_wrap_chars, TRUE);
	rf->width = -1;
//rlogit("DUMPING PCODE FOR [%s]\n", rf->value);
//rlib_pcode_dump(rf->code,0);	
//rlogit("\n\n");
}

static void rlib_literal_resolve_pcode(rlib *r, struct rlib_report *report, struct rlib_report_literal *rt) {
	rt->color_code = rlib_infix_to_pcode(r, report, rt->xml_color, TRUE);
	rt->bgcolor_code = rlib_infix_to_pcode(r, report, rt->xml_bgcolor, TRUE);
	rt->col_code = rlib_infix_to_pcode(r, report, rt->xml_col, TRUE);
	rt->width_code = rlib_infix_to_pcode(r, report, rt->xml_width, TRUE);
	rt->bold_code = rlib_infix_to_pcode(r, report, rt->xml_bold, TRUE);
	rt->italics_code = rlib_infix_to_pcode(r, report, rt->xml_italics, TRUE);
	rt->align_code = rlib_infix_to_pcode(r, report, rt->xml_align, TRUE);
	rt->width = -1;
//rlogit("DUMPING PCODE FOR [%s]\n", rt->value);
//rlib_pcode_dump(rf->code,0);	
//rlogit("\n\n");
}

static void rlib_break_resolve_pcode(rlib *r, struct rlib_report *report, struct rlib_break_fields *bf) {
	if(bf->xml_value == NULL)
		r_error("RLIB ERROR: BREAK FIELD VALUE CAN NOT BE NULL\n");
	bf->code = rlib_infix_to_pcode(r, report, bf->xml_value, TRUE);
}

static void rlib_variable_resolve_pcode(rlib *r, struct rlib_report *report, struct rlib_report_variable *rv) {
	rv->code = rlib_infix_to_pcode(r, report, rv->xml_value, TRUE);
/*rlogit("DUMPING PCODE FOR [%s]\n", rv->value);
rlib_pcode_dump(rv->code,0);	
rlogit("\n\n");*/
}

static void rlib_hr_resolve_pcode(rlib *r, struct rlib_report *report, struct rlib_report_horizontal_line * rhl) {
	rhl->size = 0;

	if(rhl->xml_indent == NULL)
		rhl->indent = 0;
	else
		rhl->indent = atof(rhl->xml_indent);
	if(rhl->xml_length == NULL)
		rhl->length = 0;
	else
		rhl->length = atof(rhl->xml_length);
	rhl->bgcolor_code = rlib_infix_to_pcode(r, report, rhl->xml_bgcolor, TRUE);
	rhl->suppress_code = rlib_infix_to_pcode(r, report, rhl->xml_suppress, TRUE);
	rhl->size_code = rlib_infix_to_pcode(r, report, rhl->xml_size, TRUE);
}

static void rlib_image_resolve_pcode(rlib *r, struct rlib_report *report, struct rlib_report_image * ri) {
	ri->value_code = rlib_infix_to_pcode(r, report, ri->xml_value, TRUE);
	ri->type_code = rlib_infix_to_pcode(r, report, ri->xml_type, TRUE);
	ri->width_code = rlib_infix_to_pcode(r, report, ri->xml_width, TRUE);
	ri->height_code = rlib_infix_to_pcode(r, report, ri->xml_height, TRUE);
}

static void rlib_resolve_fields2(rlib *r, struct rlib_report *report, struct rlib_report_output_array *roa) {
	gint j;
	struct rlib_element *e;
	
	if(roa == NULL)
		return;
		
	if(roa->xml_page != NULL)
		roa->page = atol(roa->xml_page);
	else
		roa->page = -1;
	
	for(j=0;j<roa->count;j++) {
		struct rlib_report_output *ro = roa->data[j];
		
		if(ro->type == RLIB_REPORT_PRESENTATION_DATA_LINE) {
			struct rlib_report_lines *rl = ro->data;	
			e = rl->e;
			rl->bgcolor_code = rlib_infix_to_pcode(r, report, rl->xml_bgcolor, TRUE);
			rl->color_code = rlib_infix_to_pcode(r, report, rl->xml_color, TRUE);
			rl->suppress_code = rlib_infix_to_pcode(r, report, rl->xml_suppress, TRUE);
			rl->bold_code = rlib_infix_to_pcode(r, report, rl->xml_bold, TRUE);
			rl->italics_code = rlib_infix_to_pcode(r, report, rl->xml_italics, TRUE);

			for(; e != NULL; e=e->next) {
				if(e->type == RLIB_ELEMENT_FIELD) {
					rlib_field_resolve_pcode(r, report, ((struct rlib_report_field *)e->data));
				} else if(e->type == RLIB_ELEMENT_LITERAL) {
					rlib_literal_resolve_pcode(r, report, ((struct rlib_report_literal *)e->data));
				}
			}
		} else if(ro->type == RLIB_REPORT_PRESENTATION_DATA_HR) {
			rlib_hr_resolve_pcode(r, report, ((struct rlib_report_horizontal_line *)ro->data));
		} else if(ro->type == RLIB_REPORT_PRESENTATION_DATA_IMAGE) {
			rlib_image_resolve_pcode(r, report, ((struct rlib_report_image *)ro->data));
		}
	}
}

static void rlib_resolve_outputs(rlib *r, struct rlib_report *report, struct rlib_element *e) {
	struct rlib_report_output_array *roa;
	for(; e != NULL; e=e->next) {
		roa = e->data;
		rlib_resolve_fields2(r, report, roa);
	}			

}

/*
	Report variables are refereced as v.whatever
	but when created in the <variables/> section they use there normal name.. ie.. whatever
*/
struct rlib_report_variable *rlib_resolve_variable(rlib *r, struct rlib_report *report, gchar *name) {
	struct rlib_element *e;
//r_debug("Resolving variable [%s]", name);	
	if(r_bytecount(name) >= 3 && name[0] == 'v' && name[1] == '.') {
		name += 2;
		for(e = report->variables; e != NULL; e=e->next) {
			struct rlib_report_variable *rv = e->data;
		if(!strcmp(name, rv->xml_name))
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

void rlib_resolve_graph(rlib *r, struct rlib_report *report, struct rlib_graph *graph) {
	struct rlib_graph_plot *plot;
	GSList *list;
	
	graph->type_code = rlib_infix_to_pcode(r, report, graph->xml_type, TRUE);
	graph->subtype_code = rlib_infix_to_pcode(r, report, graph->xml_subtype, TRUE);
	graph->width_code = rlib_infix_to_pcode(r, report, graph->xml_width, TRUE);
	graph->height_code = rlib_infix_to_pcode(r, report, graph->xml_height, TRUE);
	graph->title_code = rlib_infix_to_pcode(r, report, graph->xml_title, TRUE);
	graph->x_axis_title_code = rlib_infix_to_pcode(r, report, graph->xml_x_axis_title, TRUE);
	graph->y_axis_title_code = rlib_infix_to_pcode(r, report, graph->xml_y_axis_title, TRUE);

	for(list=graph->plots;list != NULL; list = g_slist_next(list)) {
		plot = list->data;
		plot->axis_code = rlib_infix_to_pcode(r, report, plot->xml_axis, TRUE);
		plot->field_code = rlib_infix_to_pcode(r, report, plot->xml_field, TRUE);
	}
}

void rlib_resolve_report_fields(rlib *r, struct rlib_report *report) {
	struct rlib_element *e;
	gfloat f;
	report->orientation = RLIB_ORIENTATION_PORTRAIT;
	report->orientation_code = rlib_infix_to_pcode(r, report, report->xml_orientation, TRUE);
	report->font_size = -1;
	report->font_size_code = rlib_infix_to_pcode(r, report, report->xml_font_size, TRUE);
	report->top_margin = RLIB_DEFAULT_TOP_MARGIN;
	report->top_margin_code = rlib_infix_to_pcode(r, report, report->xml_top_margin, TRUE);
	report->left_margin = RLIB_DEFAULT_LEFT_MARGIN;
	report->left_margin_code = rlib_infix_to_pcode(r, report, report->xml_left_margin, TRUE);
	report->bottom_margin = RLIB_DEFAULT_BOTTOM_MARGIN;
	report->bottom_margin_code = rlib_infix_to_pcode(r, report, report->xml_bottom_margin, TRUE);
	report->pages_across = 1;
	report->pages_across_code = rlib_infix_to_pcode(r, report, report->xml_pages_across, TRUE);
	report->suppress_page_header_first_page = FALSE;
	report->suppress_page_header_first_page_code = rlib_infix_to_pcode(r, report, report->xml_suppress_page_header_first_page, TRUE);
	report->height_code = rlib_infix_to_pcode(r, report, report->xml_height, TRUE);
	report->iterations = 1;
	report->iterations_code = rlib_infix_to_pcode(r, report, report->xml_iterations, TRUE);

	if (rlib_execute_as_float(r, report->pages_across_code, &f))
		report->pages_across = f;
	if (rlib_execute_as_float(r, report->iterations_code, &f))
		report->iterations = f;
		
	report->position_top = g_malloc(report->pages_across * sizeof(float));
	report->position_bottom = g_malloc(report->pages_across * sizeof(float));
	report->bottom_size = g_malloc(report->pages_across * sizeof(float));

	rlib_resolve_outputs(r, report, report->report_header);
	rlib_resolve_outputs(r, report, report->page_header);
	rlib_resolve_outputs(r, report, report->page_footer);
	rlib_resolve_outputs(r, report, report->report_footer);
	rlib_resolve_outputs(r, report, report->detail.fields);
	rlib_resolve_outputs(r, report, report->detail.headers);
	rlib_resolve_outputs(r, report, report->alternate.nodata);

	rlib_resolve_graph(r, report, &report->graph);

	if(report->breaks != NULL) {
		for(e = report->breaks; e != NULL; e=e->next) {
			struct rlib_report_break *rb = e->data;
			struct rlib_element *be;
			
			rlib_resolve_outputs(r, report, rb->header);
			rlib_resolve_outputs(r, report, rb->footer);
			rb->newpage_code = rlib_infix_to_pcode(r, report, rb->xml_newpage, TRUE);
			rb->headernewpage_code = rlib_infix_to_pcode(r, report, rb->xml_headernewpage, TRUE);
			rb->suppressblank_code = rlib_infix_to_pcode(r, report, rb->xml_suppressblank, TRUE);
			for(be = rb->fields; be != NULL; be=be->next) {
				struct rlib_break_fields *bf = be->data;
				rlib_break_resolve_pcode(r, report, bf);
			}		
		}
	}
	
	if(report->variables != NULL) {
		for(e = report->variables; e != NULL; e=e->next) {
			struct rlib_report_variable *rv = e->data;
			rlib_variable_resolve_pcode(r, report, rv);
		}
	}
}

void rlib_resolve_part_td(rlib *r, struct rlib_part *part, GSList *part_deviations) {
	GSList *element;
	
	for(element = part_deviations;element != NULL;element = g_slist_next(element)) {
		struct rlib_part_td *td = element->data;
		td->width_code = rlib_infix_to_pcode(r, NULL, td->xml_width, TRUE);
		td->height_code = rlib_infix_to_pcode(r, NULL, td->xml_height, TRUE);
		td->border_width_code = rlib_infix_to_pcode(r, NULL, td->xml_border_width, TRUE);
		td->border_color_code = rlib_infix_to_pcode(r, NULL, td->xml_border_color, TRUE);
	}
}

static void rlib_resolve_part_tr(rlib *r, struct rlib_part *part) {
	GSList *element;
	
	for(element = part->part_rows;element != NULL;element = g_slist_next(element)) {
		struct rlib_part_tr *tr = element->data;
		tr->layout_code = rlib_infix_to_pcode(r, NULL, tr->xml_layout, TRUE);
		tr->newpage_code = rlib_infix_to_pcode(r, NULL, tr->xml_newpage, TRUE);
		rlib_resolve_part_td(r, part, tr->part_deviations);
	}	
}

void rlib_resolve_part_fields(rlib *r, struct rlib_part *part) {
	gfloat f;
	part->orientation = RLIB_ORIENTATION_PORTRAIT;
	part->orientation_code = rlib_infix_to_pcode(r, NULL, part->xml_orientation, TRUE);
	part->font_size = -1;
	part->font_size_code = rlib_infix_to_pcode(r, NULL, part->xml_font_size, TRUE);
	part->top_margin = RLIB_DEFAULT_TOP_MARGIN;
	part->top_margin_code = rlib_infix_to_pcode(r, NULL, part->xml_top_margin, TRUE);
	part->left_margin = RLIB_DEFAULT_LEFT_MARGIN;
	part->left_margin_code = rlib_infix_to_pcode(r, NULL, part->xml_left_margin, TRUE);
	part->bottom_margin = RLIB_DEFAULT_BOTTOM_MARGIN;
	part->bottom_margin_code = rlib_infix_to_pcode(r, NULL, part->xml_bottom_margin, TRUE);
	part->paper = rlib_layout_get_paper(r, RLIB_PAPER_LETTER);
	part->paper_type_code = rlib_infix_to_pcode(r, NULL, part->xml_paper_type, TRUE);
	part->pages_across = 1;
	part->pages_across_code = rlib_infix_to_pcode(r, NULL, part->xml_pages_across, TRUE);
	part->iterations = 1;
	part->iterations_code = rlib_infix_to_pcode(r, NULL, part->xml_iterations, TRUE);

	if (rlib_execute_as_float(r, part->pages_across_code, &f))
		part->pages_across = f;
	if (rlib_execute_as_float(r, part->iterations_code, &f))
		part->iterations = f;

	part->position_top = g_malloc(part->pages_across * sizeof(float));
	part->position_bottom = g_malloc(part->pages_across * sizeof(float));
	part->bottom_size = g_malloc(part->pages_across * sizeof(float));

	rlib_resolve_part_tr(r, part);
	rlib_resolve_outputs(r, NULL, part->page_header);
	rlib_resolve_outputs(r, NULL, part->page_footer);
	rlib_resolve_outputs(r, NULL, part->report_header);
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

void resolve_metadata(gpointer name, gpointer value, gpointer user_data) {
	struct rlib_metadata *metadata = value;
	metadata->formula_code = rlib_infix_to_pcode(user_data, NULL, metadata->xml_formula, FALSE);
	RLIB_VALUE_TYPE_NONE(&metadata->rval_formula);
}

void rlib_resolve_metadata(rlib *r) {
	g_hash_table_foreach(r->input_metadata, resolve_metadata, r);
}

void process_metadata(gpointer name, gpointer value, gpointer user_data) {
	struct rlib_metadata *metadata = value;
	rlib_value_free(&metadata->rval_formula);
	rlib_execute_pcode(user_data, &metadata->rval_formula, metadata->formula_code, NULL);
}

void rlib_process_input_metadata(rlib *r) {
	g_hash_table_foreach(r->input_metadata, process_metadata, r);
}
