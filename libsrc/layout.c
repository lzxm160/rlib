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

gfloat rlib_layout_get_page_width(rlib *r, struct rlib_part *part) {
	if(!part->landscape)
		return (part->paper->width/RLIB_PDF_DPI);
	else
		return (part->paper->height/RLIB_PDF_DPI);
}

void rlib_layout_init_part_page(rlib *r, struct rlib_part *part) {
	gint i;

r_error("rlib_layout_init_part_page!!!!!!!!!!\n");
	for(i=0;i<part->pages_accross;i++) {
		part->position_top[i] = part->top_margin;
		part->bottom_size[i] = get_outputs_size(r, part->page_footer, i);
		part->position_bottom[i] -= part->bottom_size[i];
	}		
	r->current_font_point = -1;
	OUTPUT(r)->rlib_start_new_page(r, part);
	OUTPUT(r)->rlib_set_font_point(r, r->font_point);
	
	rlib_layout_report_output(r, part, NULL, part->page_header, FALSE);

	for(i=0; i<part->pages_accross; i++)
		part->position_bottom[i] -= part->bottom_size[i];

	rlib_layout_report_output(r, part, NULL, part->page_footer, TRUE);

	OUTPUT(r)->rlib_init_end_page(r);
}
