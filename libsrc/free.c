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

#include "ralloc.h"
#include "rlib.h"
#include "pcode.h"

static void free_pcode(struct rlib_pcode *code) {
	int i=0;
	
	if(code == NULL)
		return;
	
	for(i=0;i<code->count;i++) {
		if(code->instructions[i].instruction == PCODE_PUSH) {
			struct rlib_pcode_operand *o = code->instructions[i].value;
			if(o->type == OPERAND_STRING || o->type == OPERAND_NUMBER || o->type == OPERAND_FIELD)
				rfree(o->value);
			if(o->type == OPERAND_IIF) {
				struct rlib_pcode_if *rpif = o->value;
				free_pcode(rpif->evaulation);
				free_pcode(rpif->true);
				free_pcode(rpif->false);
				rfree(rpif);				
			}
			rfree(o);
		}
	}

	rfree(code);
}

static void rlib_image_free_pcode(rlib *r, struct report_image * ri) {
	free_pcode(ri->value_code);
	free_pcode(ri->type_code);
	free_pcode(ri->width_code);
	free_pcode(ri->height_code);
	rfree(ri);
}

static void rlib_hr_free_pcode(rlib *r, struct report_horizontal_line * rhl) {
	free_pcode(rhl->bgcolor_code);
	rfree(rhl);
}

static void rlib_text_free_pcode(rlib *r, struct report_text *rt) {
	free_pcode(rt->color_code);
	free_pcode(rt->bgcolor_code);
	free_pcode(rt->col_code);
	rfree(rt);
}

static void rlib_field_free_pcode(rlib *r, struct report_field *rf) {
	free_pcode(rf->code);
	free_pcode(rf->format_code);
	free_pcode(rf->link_code);
	free_pcode(rf->color_code);
	free_pcode(rf->bgcolor_code);
	free_pcode(rf->col_code);
	rfree(rf);
}

static void rlib_free_fields(rlib *r, struct report_output_array *roa) {
	struct report_element *e, *save;
	int j;
	
	if(roa == NULL)
		return;
	for(j=0;j<roa->count;j++) {
		struct report_output *ro = roa->data[j];
		if(ro->type == REPORT_PRESENTATION_DATA_LINE) {
			struct report_lines *rl = ro->data;	
			e = rl->e;
			free_pcode(rl->bgcolor_code);
			free_pcode(rl->color_code);
			for(; e != NULL; e=e->next) {
				if(e->type == REPORT_ELEMENT_FIELD) {
					rlib_field_free_pcode(r, ((struct report_field *)e->data));
				} else if(e->type == REPORT_ELEMENT_TEXT) {
					rlib_text_free_pcode(r, ((struct report_text *)e->data));
				}
			}
			for(e=rl->e; e != NULL; ) {
				save = e;
				e=e->next;
				rfree(save);
			}
			rfree(rl);
		} else if(ro->type == REPORT_PRESENTATION_DATA_HR) {
			rlib_hr_free_pcode(r, ((struct report_horizontal_line *)ro->data));
		} else if(ro->type == REPORT_PRESENTATION_DATA_IMAGE) {
			rlib_image_free_pcode(r, ((struct report_image *)ro->data));
		}
		rfree(ro);
	}
	rfree(roa->data);
	rfree(roa);
}

void rlib_free_report(rlib *r, int which) {
	rlib_free_fields(r, r->reports[which]->report_header);
	rlib_free_fields(r, r->reports[which]->page_header);
	rlib_free_fields(r, r->reports[which]->page_footer);
	rlib_free_fields(r, r->reports[which]->report_footer);
	rlib_free_fields(r, r->reports[which]->detail.fields);
	rlib_free_fields(r, r->reports[which]->detail.textlines);

}

void rlib_free_tree(rlib *r) {
	int i;
	for(i=0;i<r->reports_count;i++) {
		rlib_free_report(r, i);
		xmlFreeDoc(r->reports[i]->doc);
		rfree(r->reports[i]);
		r->reports[i] = NULL;
	}
}

void free_results(rlib *r) {
	int i;
	for(i=0;i<r->queries_count;i++) {
		INPUT(r)->rlib_free_result(INPUT(r), r->results[i].result);
	}
}

int rlib_free(rlib *r) {
	rlib_free_tree(r);
	xmlCleanupParser();
	
	free_results(r);
	INPUT(r)->input_close(INPUT(r));
	INPUT(r)->free(INPUT(r));	

	OUTPUT(r)->rlib_free(r);
	
	ENVIRONMENT(r)->free(r);
				
	rfree(r);
	return 0;
}
