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
 */

#include <stdlib.h>
#include <dlfcn.h>

#include "config.h"
#include "rlib.h"
#include "pcode.h"
#include "rlib_input.h"

static void free_pcode(struct rlib_pcode *code) {
	gint i=0;
	
	if(code == NULL)
		return;
	
	for(i=0;i<code->count;i++) {
		if(code->instructions[i].instruction == PCODE_PUSH) {
			struct rlib_pcode_operand *o = code->instructions[i].value;
			if(o->type == OPERAND_STRING || o->type == OPERAND_NUMBER || o->type == OPERAND_FIELD)
				g_free(o->value);
			if(o->type == OPERAND_IIF) {
				struct rlib_pcode_if *rpif = o->value;
				free_pcode(rpif->evaulation);
				free_pcode(rpif->true);
				free_pcode(rpif->false);
				g_free(rpif);				
			}
			g_free(o);
		}
	}
	g_free(code);
}

static void rlib_image_free_pcode(rlib *r, struct rlib_report_image * ri) {
	free_pcode(ri->value_code);
	free_pcode(ri->type_code);
	free_pcode(ri->width_code);
	free_pcode(ri->height_code);
	g_free(ri);
}

static void rlib_hr_free_pcode(rlib *r, struct rlib_report_horizontal_line * rhl) {
	free_pcode(rhl->bgcolor_code);
	free_pcode(rhl->suppress_code);
	free_pcode(rhl->indent_code);
	free_pcode(rhl->length_code);
	free_pcode(rhl->font_size_code);
	free_pcode(rhl->size_code);
	g_free(rhl);
}

static void rlib_text_free_pcode(rlib *r, struct rlib_report_literal *rt) {
	free_pcode(rt->color_code);
	free_pcode(rt->bgcolor_code);
	free_pcode(rt->col_code);
	free_pcode(rt->width_code);
	free_pcode(rt->bold_code);
	free_pcode(rt->italics_code);
	free_pcode(rt->align_code);
	g_free(rt);
}

static void rlib_field_free_pcode(rlib *r, struct rlib_report_field *rf) {
	free_pcode(rf->code);
	free_pcode(rf->format_code);
	free_pcode(rf->link_code);
	free_pcode(rf->color_code);
	free_pcode(rf->bgcolor_code);
	free_pcode(rf->col_code);
	free_pcode(rf->width_code);
	free_pcode(rf->bold_code);
	free_pcode(rf->italics_code);
	free_pcode(rf->align_code);
	free_pcode(rf->memo_code);
	free_pcode(rf->memo_height_code);
	free_pcode(rf->memo_wrap_chars_code);
	g_free(rf);
}

static void rlib_free_fields(rlib *r, struct rlib_report_output_array *roa) {
	struct rlib_element *e, *save;
	gint j;
	
	if(roa == NULL)
		return;
	for(j=0;j<roa->count;j++) {
		struct rlib_report_output *ro = roa->data[j];
		if(ro->type == RLIB_REPORT_PRESENTATION_DATA_LINE) {
			struct rlib_report_lines *rl = ro->data;	
			e = rl->e;
			free_pcode(rl->bgcolor_code);
			free_pcode(rl->color_code);
			free_pcode(rl->suppress_code);
			free_pcode(rl->font_size_code);
			free_pcode(rl->bold_code);
			free_pcode(rl->italics_code);
			for(; e != NULL; e=e->next) {
				if(e->type == RLIB_ELEMENT_FIELD) {
					rlib_field_free_pcode(r, ((struct rlib_report_field *)e->data));
				} else if(e->type == RLIB_ELEMENT_LITERAL) {
					rlib_text_free_pcode(r, ((struct rlib_report_literal *)e->data));
				}
			}
			for(e=rl->e; e != NULL; ) {
				save = e;
				e=e->next;
				g_free(save);
			}
			g_free(rl);
		} else if(ro->type == RLIB_REPORT_PRESENTATION_DATA_HR) {
			rlib_hr_free_pcode(r, ((struct rlib_report_horizontal_line *)ro->data));
		} else if(ro->type == RLIB_REPORT_PRESENTATION_DATA_IMAGE) {
			rlib_image_free_pcode(r, ((struct rlib_report_image *)ro->data));
		}
		g_free(ro);
	}
	g_free(roa->data);
	g_free(roa);
}


static void rlib_break_free_pcode(rlib *r, struct rlib_break_fields *bf) {
	free_pcode(bf->code);
}

static void rlib_free_output(rlib *r, struct rlib_element *e) {
	struct rlib_report_output_array *roa;
	struct rlib_element *save;
	while(e != NULL) {
		save = e;
		roa = e->data;
		rlib_free_fields(r, roa);
		e=e->next;
		g_free(save);
	}	
}

void rlib_free_report(rlib *r, struct rlib_report *report) {
	struct rlib_element *e, *prev;

	if(report->doc != NULL) {
//		xmlFreeDoc(report->doc);
	} else {
		g_free(report->contents);
	}
	
	free_pcode(report->font_size_code);
	free_pcode(report->orientation_code);
	free_pcode(report->top_margin_code);
	free_pcode(report->left_margin_code);
	free_pcode(report->bottom_margin_code);
	free_pcode(report->pages_across_code);
	free_pcode(report->suppress_page_header_first_page_code);
	
	rlib_free_output(r, report->report_header);
	rlib_free_output(r, report->page_header);
	rlib_free_output(r, report->page_footer);
	rlib_free_output(r, report->report_footer);
	rlib_free_output(r, report->detail.fields);
	rlib_free_output(r, report->detail.headers);
	rlib_free_output(r, report->alternate.nodata);

	g_free(report->position_top);
	g_free(report->position_bottom);
	g_free(report->bottom_size);
	
	if(report->breaks != NULL) {
		for(e = report->breaks; e != NULL; e=e->next) {
			struct rlib_report_break *rb = e->data;
			struct rlib_element *be;
			rlib_free_output(r, rb->header);
			rlib_free_output(r, rb->footer);
			for(be = rb->fields; be != NULL; be=be->next) {
				struct rlib_break_fields *bf = be->data;
				rlib_break_free_pcode(r, bf);
				g_free(bf);
			}
			
			while(rb->fields) {
				prev = NULL;
				for(be = rb->fields; be->next != NULL; be=be->next) {
					prev = be;
				}
				g_free(be);
				if(prev != NULL)
					prev->next = NULL;
				else
					break;
			}
			
			g_free(rb);
		}

		while(report->breaks) {
			prev = NULL;
			for(e = report->breaks; e->next != NULL; e=e->next) {
				prev = e;
			}
			g_free(e);
			if(prev != NULL)
				prev->next = NULL;
			else
				break;
		}

	}	


	if(report->variables != NULL) {
		for(e = report->variables; e != NULL; e=e->next) {
			struct rlib_report_variable *rv = e->data;
			free_pcode(rv->code);

			if(rv->type == RLIB_REPORT_VARIABLE_EXPRESSION) {
				rlib_value_free(&RLIB_VARIABLE_CA(rv)->amount);
				g_free(RLIB_VARIABLE_CA(rv));
			} else if(rv->type == RLIB_REPORT_VARIABLE_COUNT)
				g_free(RLIB_VARIABLE_CA(rv));
			else if(rv->type == RLIB_REPORT_VARIABLE_SUM)
				g_free(RLIB_VARIABLE_CA(rv));
			else if(rv->type == RLIB_REPORT_VARIABLE_AVERAGE)
				g_free(RLIB_VARIABLE_CA(rv));
			else if(rv->type == RLIB_REPORT_VARIABLE_LOWEST)
				g_free(RLIB_VARIABLE_CA(rv));
			else if(rv->type == RLIB_REPORT_VARIABLE_HIGHEST)
				g_free(RLIB_VARIABLE_CA(rv));
			g_free(rv);
		}
		
		while(report->variables) {
			prev = NULL;
			for(e = report->variables; e->next != NULL; e=e->next) {
				prev = e;
			}
			g_free(e);
			if(prev != NULL)
				prev->next = NULL;
			else
				break;
		}
	}
}

void rlib_free_part_td(rlib *r, struct rlib_part *part, struct rlib_element *e_td) {
	struct rlib_element *e, *td_contents;
	for(e=e_td;e != NULL;e=e->next) {
		struct rlib_part_td *td = e->data;
		free_pcode(td->width_code);
		free_pcode(td->height_code);
		free_pcode(td->border_width_code);
		free_pcode(td->border_color_code);

		for(td_contents=td->e;td_contents != NULL;td_contents=td_contents->next) {
			if(td_contents->type == RLIB_ELEMENT_REPORT) {
				struct rlib_report *report = td_contents->data;
				rlib_free_report(r, report);
			}
		}

	}
}

void rlib_free_part_tr(rlib *r, struct rlib_part *part, struct rlib_element *e_tr) {
	struct rlib_element *e;
	
	for(e=e_tr;e != NULL;e=e->next) {
		struct rlib_part_tr *tr = e->data;
		free_pcode(tr->layout_code);
		free_pcode(tr->newpage_code);
		rlib_free_part_td(r, part, tr->e);
	}	
}

void rlib_free_part(rlib *r, struct rlib_part *part) {
	free_pcode(part->orientation_code);
	free_pcode(part->font_size_code);
	free_pcode(part->top_margin_code);
	free_pcode(part->left_margin_code);
	free_pcode(part->bottom_margin_code);
	free_pcode(part->paper_type_code);
	free_pcode(part->pages_across_code);
	
	g_free(part->position_top);
	g_free(part->position_bottom);
	g_free(part->bottom_size);

	rlib_free_output(r, part->page_header);
	rlib_free_output(r, part->page_footer);

	rlib_free_part_tr(r, part, part->tr_elements);
}

void rlib_free_tree(rlib *r) {
	int i;
	for(i=0;i<r->parts_count;i++) {
		struct rlib_part *part = r->parts[i];
		rlib_free_part(r, part);
		g_free(r->reportstorun[i].name);
		r->parts[i] = NULL;
	}
}

void free_results(rlib *r) {
	int i;
	for(i=0;i<r->queries_count;i++) {
		INPUT(r, i)->free_result(INPUT(r, i), r->results[i].result);
		g_free(r->queries[i].sql);
		g_free(r->queries[i].name);
	}
}


gint rlib_free(rlib *r) {
	int i;
	rlib_char_encoder_destroy(&r->db_encoder);
	rlib_char_encoder_destroy(&r->param_encoder);
	rlib_char_encoder_destroy(&r->output_encoder);


	rlib_free_tree(r);
	xmlCleanupParser();
	free_results(r);
	for(i=0;i<r->inputs_count;i++) {
		r->inputs[i].input->input_close(r->inputs[i].input);
		r->inputs[i].input->free(r->inputs[i].input);	
		if(r->inputs[i].handle != NULL)
			dlclose(r->inputs[i].handle);
	}

	if (r->htParameters) {
		rlib_hashtable_destroy(r->htParameters);
	}
	
	OUTPUT(r)->free(r);
	
	ENVIRONMENT(r)->free(r);
				
	g_free(r);
	return 0;
}
