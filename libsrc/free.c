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

static void rlib_image_free_pcode(rlib *r, struct report_image * ri) {
	rfree(ri->value_code);
	rfree(ri->type_code);
	rfree(ri->width_code);
	rfree(ri->height_code);
	free(ri);
}

static void rlib_hr_free_pcode(rlib *r, struct report_horizontal_line * rhl) {
	rfree(rhl->bgcolor_code);
	free(rhl);
}

static void rlib_text_free_pcode(rlib *r, struct report_text *rt) {
	rfree(rt->color_code);
	rfree(rt->bgcolor_code);
	rfree(rt->col_code);
	rfree(rt);
}

static void rlib_field_free_pcode(rlib *r, struct report_field *rf) {
	rfree(rf->code);
	rfree(rf->format_code);
	rfree(rf->link_code);
	rfree(rf->color_code);
	rfree(rf->bgcolor_code);
	rfree(rf->col_code);
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
			rfree(rl->bgcolor_code);
			rfree(rl->color_code);
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
		rfree(roa->data);
	}
	rfree(roa);
}

void rlib_free_report(rlib *r, int which) {
	struct report_element *e;

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
debugf("FREEING TREE [%d]\n", i);	
		rlib_free_report(r, i);
	}
}

int rlib_free(rlib *r) {
	INPUT(r)->rlib_input_close(INPUT(r));
	INPUT(r)->free(INPUT(r));
	
	rlib_free_tree(r);
		
	rfree(r);
}
