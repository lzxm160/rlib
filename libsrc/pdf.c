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
#include <cpdflib.h>

#include "ralloc.h"
#include "rlib.h"

struct _private {
	struct rgb current_color;
	CPDFdoc *pdf;
};

static float rlib_pdf_get_string_width(rlib *r, char *text) {
	return cpdf_stringWidth(OUTPUT_PRIVATE(r)->pdf, text)/(72.0);
}

static void rlib_pdf_print_text(rlib *r, float left_origin, float bottom_origin, char *text, int backwards, int col) {
	cpdf_text(OUTPUT_PRIVATE(r)->pdf, left_origin, bottom_origin, 0, text);
}


static void rlib_pdf_set_fg_color(rlib *r, float red, float green, float blue) {
	if(OUTPUT_PRIVATE(r)->current_color.r != red || OUTPUT_PRIVATE(r)->current_color.g != green 
	|| OUTPUT_PRIVATE(r)->current_color.b != blue) {
		if(red != -1 && green != -1 && blue != -1 )
			cpdf_setrgbcolor(OUTPUT_PRIVATE(r)->pdf, red, green, blue);
		OUTPUT_PRIVATE(r)->current_color.r = red;
		OUTPUT_PRIVATE(r)->current_color.g = green;
		OUTPUT_PRIVATE(r)->current_color.b = blue;	
	}
}

static void rlib_pdf_drawbox(rlib *r, float left_origin, float bottom_origin, float how_long, float how_tall, struct rgb *color) {
	if(!(color->r == 1.0 && color->g == 1.0 && color->b == 1.0)) {
		cpdf_endText(OUTPUT_PRIVATE(r)->pdf);
		//the -.002 seems to get around decimal percision problems.. but should investigate this a big further	
		OUTPUT(r)->rlib_set_bg_color(r, color->r, color->g, color->b);
		cpdf_rect(OUTPUT_PRIVATE(r)->pdf, left_origin, bottom_origin, how_long, how_tall-.002);
		cpdf_fill(OUTPUT_PRIVATE(r)->pdf);
		cpdf_beginText(OUTPUT_PRIVATE(r)->pdf, 0);	
		OUTPUT(r)->rlib_set_bg_color(r, 0, 0, 0);
	}
}

static void rlib_pdf_hr(rlib *r, int backwards, float left_origin, float bottom_origin, float how_long, float how_tall, 
struct rgb *color, float indent, float length) {
	how_tall = how_tall / 72.0;
	rlib_pdf_drawbox(r, left_origin, bottom_origin, how_long, how_tall, color);
}

/*
	What was the guy from ClibPDF Smoking....cpdf_SetActionURL origin is bottom right... /me sighs
*/
static void rlib_pdf_boxurl_start(rlib *r, float left_origin, float bottom_origin, float how_long, float how_tall, char *url) {
	if(r->landscape) {
		float new_left = get_page_width(r)-left_origin-how_long;
		float new_bottom = bottom_origin;
		cpdf_setActionURL(OUTPUT_PRIVATE(r)->pdf, new_bottom, new_left, new_bottom+how_tall, new_left+how_long, url, NULL);
	} else  {
		float new_left = left_origin;
		float new_bottom = bottom_origin;
		cpdf_setActionURL(OUTPUT_PRIVATE(r)->pdf, new_left, new_bottom, new_left+how_long, new_bottom+how_tall, url, NULL);
	}
}

static void rlib_pdf_drawimage(rlib *r, float left_origin, float bottom_origin, char *nname, char *type, float nwidth, 
	float nheight) {
	char pathbuf[MAXSTRLEN];
	float xscale = 1.0, yscale = 1.0;
	int realtype=JPEG_IMG;
	if(!strcmp("gif", type))
		realtype = GIF_IMG;
	cpdf_endText(OUTPUT_PRIVATE(r)->pdf);
	cpdf_convertUpathToOS(pathbuf, nname);
	cpdf_importImage(OUTPUT_PRIVATE(r)->pdf, pathbuf, realtype, left_origin, bottom_origin, 0.0, &nwidth, &nheight, &xscale, &yscale, 1);
	cpdf_beginText(OUTPUT_PRIVATE(r)->pdf, 0);	
	OUTPUT(r)->rlib_set_bg_color(r, 0, 0, 0);
}

static void rlib_pdf_set_font_point(rlib *r, int point) {
	if(r->current_font_point != point) {
		cpdf_setFont(OUTPUT_PRIVATE(r)->pdf, "Courier", "Courier", point);
		r->current_font_point = point;
	}
}

static void rlib_pdf_start_new_page(rlib *r) {
	if(r->reports[r->current_report]->orientation == RLIB_ORIENTATION_LANDSCAPE) {
		r->position_bottom = 8.5-GET_MARGIN(r)->bottom_margin;
		cpdf_pageInit(OUTPUT_PRIVATE(r)->pdf, r->current_page_number, LANDSCAPE, LETTER, LETTER); 
		cpdf_translate(OUTPUT_PRIVATE(r)->pdf, 0.0, 11.0);	/* 11.0 must be changed for non-LETTER size */
	   cpdf_rotate(OUTPUT_PRIVATE(r)->pdf, -90.0);
		r->landscape = 1;
	} else {
		r->position_bottom = 11-GET_MARGIN(r)->bottom_margin;
		cpdf_pageInit(OUTPUT_PRIVATE(r)->pdf, r->current_page_number, PORTRAIT, LETTER, LETTER); 
		r->landscape = 0;
	}
	cpdf_beginText(OUTPUT_PRIVATE(r)->pdf, 0);	
}

static void rlib_pdf_init_end_page(rlib *r) {
//TODO: Why is this needed?
	if(r->start_of_new_report == TRUE) {
		r->start_of_new_report = FALSE;
		cpdf_endText(OUTPUT_PRIVATE(r)->pdf);
	}

}

static void rlib_pdf_end_text(rlib *r) {
	cpdf_endText(OUTPUT_PRIVATE(r)->pdf);
}

static void rlib_pdf_init_output(rlib *r) {
	CPDFdocLimits dL = {500, -1, -1, 10000, 10000};
	OUTPUT_PRIVATE(r)->pdf = cpdf_open(0, &dL);
	cpdf_enableCompression(OUTPUT_PRIVATE(r)->pdf, YES);
	cpdf_init(OUTPUT_PRIVATE(r)->pdf);
	cpdf_setTitle(OUTPUT_PRIVATE(r)->pdf, "SICOM Systems Report");
}

static void rlib_pdf_begin_text(rlib *r) {
	cpdf_beginText(OUTPUT_PRIVATE(r)->pdf, 0);	
}

static void rlib_pdf_finalize_private(rlib *r) {
	int length;
	cpdf_finalizeAll(OUTPUT_PRIVATE(r)->pdf);
	r->bufPDF = cpdf_getBufferForPDF(OUTPUT_PRIVATE(r)->pdf, &length);
	r->length = length;
}

static void rlib_pdf_spool_private(rlib *r) {
	ENVIRONMENT(r)->rlib_write_output(r->bufPDF, r->length);
	cpdf_close(OUTPUT_PRIVATE(r)->pdf);
}

static void rlib_pdf_end_page(rlib *r) {
	OUTPUT(r)->rlib_end_text(r);
	r->current_page_number++;
	r->current_line_number = 1;
	rlib_init_page(r, FALSE);
}

static int rlib_pdf_is_single_page(rlib *r) {
	return FALSE;
}

static int rlib_pdf_free(rlib *r) {
	rfree(OUTPUT_PRIVATE(r));
	rfree(OUTPUT(r));
	return 0;
}


static void rlib_pdf_stub_line(rlib *r, int backwards) {}

static void rlib_pdf_end_output_section(rlib *r) {}
static void rlib_pdf_start_output_section(rlib *r) {}
static void rlib_pdf_boxurl_end(rlib *r) {}
static void rlib_pdf_draw_cell_background_end(rlib *r) {}
static void rlib_pdf_init_output_report(rlib *r) {}

void rlib_pdf_new_output_filter(rlib *r) {
	OUTPUT(r) = rmalloc(sizeof(struct output_filter));
	OUTPUT_PRIVATE(r) = rmalloc(sizeof(struct _private));
	bzero(OUTPUT_PRIVATE(r), sizeof(struct _private));

	OUTPUT(r)->do_align = TRUE;
	OUTPUT(r)->do_break = TRUE;
	OUTPUT(r)->do_grouptext = TRUE;	

	OUTPUT(r)->rlib_get_string_width = rlib_pdf_get_string_width;
	OUTPUT(r)->rlib_print_text = rlib_pdf_print_text;
	OUTPUT(r)->rlib_set_fg_color = rlib_pdf_set_fg_color;
	OUTPUT(r)->rlib_set_bg_color = rlib_pdf_set_fg_color;
	OUTPUT(r)->rlib_draw_cell_background_start = rlib_pdf_drawbox;
	OUTPUT(r)->rlib_draw_cell_background_end = rlib_pdf_draw_cell_background_end;
	OUTPUT(r)->rlib_hr = rlib_pdf_hr;
	OUTPUT(r)->rlib_boxurl_start = rlib_pdf_boxurl_start;
	OUTPUT(r)->rlib_boxurl_end = rlib_pdf_boxurl_end;
	OUTPUT(r)->rlib_drawimage = rlib_pdf_drawimage;
	OUTPUT(r)->rlib_set_font_point = rlib_pdf_set_font_point;
	OUTPUT(r)->rlib_start_new_page = rlib_pdf_start_new_page;
	OUTPUT(r)->rlib_end_page = rlib_pdf_end_page;
	OUTPUT(r)->rlib_init_end_page = rlib_pdf_init_end_page;
	OUTPUT(r)->rlib_end_text = rlib_pdf_end_text;
	OUTPUT(r)->rlib_init_output = rlib_pdf_init_output;
	OUTPUT(r)->rlib_init_output_report = rlib_pdf_init_output_report;
	OUTPUT(r)->rlib_begin_text = rlib_pdf_begin_text;
	OUTPUT(r)->rlib_finalize_private = rlib_pdf_finalize_private;
	OUTPUT(r)->rlib_spool_private = rlib_pdf_spool_private;
	OUTPUT(r)->rlib_start_line = rlib_pdf_stub_line;
	OUTPUT(r)->rlib_end_line = rlib_pdf_stub_line;
	OUTPUT(r)->rlib_is_single_page = rlib_pdf_is_single_page;
	OUTPUT(r)->rlib_start_output_section = rlib_pdf_start_output_section;
	OUTPUT(r)->rlib_end_output_section = rlib_pdf_end_output_section;
	OUTPUT(r)->rlib_free = rlib_pdf_free;
}
