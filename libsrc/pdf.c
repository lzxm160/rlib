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

#include "rlib.h"

struct _private {
	struct rgb current_color;
	CPDFdoc *pdf;
	gchar *buffer;
	gint length;
};


static gfloat rlib_pdf_get_string_width(rlib *r, gchar *text) {
//	struct rlib_report *rr = r->reports[r->current_report]; //Should be a better way
//	if (rr->output_encoder != (iconv_t) -1) text = (gchar *) encode(rr->output_encoder, text);
//	else if(r->output_encoder != (iconv_t) -1) text = (gchar *) encode(r->output_encoder, text);
	return cpdf_stringWidth(OUTPUT_PRIVATE(r)->pdf, text)/(RLIB_PDF_DPI);
}

static void rlib_pdf_print_text(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *t, gint backwards, gint col) {
	CPDFdoc *pdf = OUTPUT_PRIVATE(r)->pdf;
	gchar *tmp = t;
	gchar *buf;
	glong itemsread;
	glong itemswritten;
	GError *error;
	gunichar2 *unistr;

	if (r->utf8) {
//rlib_trap();				output_encoder
		cpdf_hexStringMode(pdf, YES);
		unistr = g_utf8_to_utf16(tmp, -1, &itemsread, &itemswritten, &error);
		buf = g_malloc(2 * sizeof(gunichar2) * (itemswritten + 2));
		cpdf_convertBinaryToHex((const guchar *) unistr, buf, sizeof(gunichar2) * itemswritten, NO);
		g_free(unistr);
		cpdf_text(pdf, left_origin, bottom_origin, 0, buf);
		g_free(buf);
		cpdf_hexStringMode(pdf, NO);
	} else {
//		struct rlib_report *rr = r->reports[r->current_report]; //YECH!!
//		if (rr->output_encoder != (iconv_t) -1) t = (gchar *) encode(rr->output_encoder, t);
//		else if(r->output_encoder != (iconv_t) -1) t = (gchar *) encode(r->output_encoder, t);
		cpdf_text(pdf, left_origin, bottom_origin, 0, t);
	}
}

static void rlib_pdf_set_fg_color(rlib *r, gfloat red, gfloat green, gfloat blue) {
	if(OUTPUT_PRIVATE(r)->current_color.r != red || OUTPUT_PRIVATE(r)->current_color.g != green 
	|| OUTPUT_PRIVATE(r)->current_color.b != blue) {
		if(red != -1 && green != -1 && blue != -1 )
			cpdf_setrgbcolor(OUTPUT_PRIVATE(r)->pdf, red, green, blue);
		OUTPUT_PRIVATE(r)->current_color.r = red;
		OUTPUT_PRIVATE(r)->current_color.g = green;
		OUTPUT_PRIVATE(r)->current_color.b = blue;	
	}
}

static void rlib_pdf_drawbox(rlib *r, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, struct rgb *color) {
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

static void rlib_pdf_hr(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, 
struct rgb *color, gfloat indent, gfloat length) {
	how_tall = how_tall / RLIB_PDF_DPI;
	rlib_pdf_drawbox(r, left_origin, bottom_origin, how_long, how_tall, color);
}

/*
	What was the guy from ClibPDF Smoking....cpdf_SetActionURL origin is bottom right... /me sighs
*/
static void rlib_pdf_boxurl_start(rlib *r, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, gchar *url) {
	if(r->landscape) {
		gfloat new_left = get_page_width(r)-left_origin-how_long;
		gfloat new_bottom = bottom_origin;
		cpdf_setActionURL(OUTPUT_PRIVATE(r)->pdf, new_bottom, new_left, new_bottom+how_tall, new_left+how_long, url, NULL);
	} else  {
		gfloat new_left = left_origin;
		gfloat new_bottom = bottom_origin;
		cpdf_setActionURL(OUTPUT_PRIVATE(r)->pdf, new_left, new_bottom, new_left+how_long, new_bottom+how_tall, url, NULL);
	}
}

static void rlib_pdf_drawimage(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *nname, gchar *type, gfloat nwidth, 
gfloat nheight) {
	gchar pathbuf[MAXSTRLEN];
	gfloat xscale = 1.0, yscale = 1.0;
	gint realtype=JPEG_IMG;
	if(!strcmp("gif", type))
		realtype = GIF_IMG;
	cpdf_endText(OUTPUT_PRIVATE(r)->pdf);
	cpdf_convertUpathToOS(pathbuf, nname);
	cpdf_importImage(OUTPUT_PRIVATE(r)->pdf, pathbuf, realtype, left_origin, bottom_origin, 0.0, &nwidth, &nheight, &xscale, &yscale, 1);
	cpdf_beginText(OUTPUT_PRIVATE(r)->pdf, 0);	
	OUTPUT(r)->rlib_set_bg_color(r, 0, 0, 0);
}

static void rlib_pdf_set_font_point(rlib *r, gint point) {
	char *encoding;
	char *fontname;
	int result;
	if(r->current_font_point != point) {
		if (*r->pdf_fontdir1) { //if one set other is guaranteed to be set
			cpdf_setFontDirectories(OUTPUT_PRIVATE(r)->pdf, r->pdf_fontdir1, r->pdf_fontdir2);
//r_debug("Using font directories %s, %s", r->pdf_fontdir1, r->pdf_fontdir1);
		}
		encoding = (*r->pdf_encoding)? r->pdf_encoding : NULL;
		fontname = (*r->pdf_fontname)? r->pdf_fontname : "Courier";
//r_debug("Fontname %s, encoding %s", fontname, encoding);
		result = cpdf_setFont(OUTPUT_PRIVATE(r)->pdf, fontname, encoding, point);
//r_debug("cpdf_setFont returned %d for f:%s, e:%s", result, fontname, encoding);
		r->current_font_point = point;
	}
}

static void rlib_pdf_start_new_page(rlib *r) {
	struct rlib_report *report = r->reports[r->current_report];
	gint i=0;
	gint pages_accross = report->pages_accross;
	gint page_number = r->current_page_number * pages_accross;
	gchar paper_type[40];
	sprintf(paper_type, "0 0 %d %d", report->paper->width, report->paper->height);
	for(i=0;i<pages_accross;i++) {
		if(report->orientation == RLIB_ORIENTATION_LANDSCAPE) {
			report->position_bottom[i] = (report->paper->width/RLIB_PDF_DPI)-GET_MARGIN(r)->bottom_margin;
			cpdf_pageInit(OUTPUT_PRIVATE(r)->pdf, page_number+i, LANDSCAPE, paper_type, paper_type); 
			cpdf_translate(OUTPUT_PRIVATE(r)->pdf, 0.0, (report->paper->height/RLIB_PDF_DPI));	
		   cpdf_rotate(OUTPUT_PRIVATE(r)->pdf, -90.0);
			r->landscape = 1;
		} else {
			report->position_bottom[i] = 11-GET_MARGIN(r)->bottom_margin;
			cpdf_pageInit(OUTPUT_PRIVATE(r)->pdf, page_number+i, PORTRAIT, paper_type, paper_type); 
			r->landscape = 0;
		}
		cpdf_beginText(OUTPUT_PRIVATE(r)->pdf, 0);	
	}
}

static void rlib_pdf_set_working_page(rlib *r, gint page) {
	gint pages_accross = r->reports[r->current_report]->pages_accross;
	gint page_number = r->current_page_number * pages_accross;
	page--;
	cpdf_setCurrentPage(OUTPUT_PRIVATE(r)->pdf, page_number + page);
}

static void rlib_pdf_end_text(rlib *r) {
	gint i=0;
	gint pages_accross = r->reports[r->current_report]->pages_accross;
	gint page_number = r->current_page_number * pages_accross;

	for(i=0;i<pages_accross;i++) {
		cpdf_setCurrentPage(OUTPUT_PRIVATE(r)->pdf, page_number+i);
		cpdf_endText(OUTPUT_PRIVATE(r)->pdf);
	}
}

static void rlib_pdf_init_end_page(rlib *r) {
//TODO: Why is this needed?
	if(r->start_of_new_report == TRUE) {
		r->start_of_new_report = FALSE;
		cpdf_endText(OUTPUT_PRIVATE(r)->pdf);
	}

}

static void rlib_pdf_init_output(rlib *r) {
	CPDFdoc *pdf;
	CPDFdocLimits dL = {500, -1, -1, 10000, 10000};

	r_debug("CPDF version %s", cpdf_version() );
	pdf = OUTPUT_PRIVATE(r)->pdf = cpdf_open(0, &dL);
	cpdf_enableCompression(pdf, YES);
	cpdf_init(pdf);
	cpdf_setTitle(pdf, "RLIB Report");
}

static void rlib_pdf_begin_text(rlib *r) {
	cpdf_beginText(OUTPUT_PRIVATE(r)->pdf, 0);	
}

static void rlib_pdf_finalize_private(rlib *r) {
	int length;
	cpdf_finalizeAll(OUTPUT_PRIVATE(r)->pdf);
	OUTPUT_PRIVATE(r)->buffer = cpdf_getBufferForPDF(OUTPUT_PRIVATE(r)->pdf, &length);
	OUTPUT_PRIVATE(r)->length = length;
}

static void rlib_pdf_spool_private(rlib *r) {
	ENVIRONMENT(r)->rlib_write_output(OUTPUT_PRIVATE(r)->buffer, OUTPUT_PRIVATE(r)->length);
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
	g_free(OUTPUT_PRIVATE(r));
	g_free(OUTPUT(r));
	return 0;
}

static char *rlib_pdf_get_output(rlib *r) {
	return OUTPUT_PRIVATE(r)->buffer;
}

static long rlib_pdf_get_output_length(rlib *r) {
	return OUTPUT_PRIVATE(r)->length;
}

static void rlib_pdf_stub_line(rlib *r, int backwards) {}

static void rlib_pdf_end_output_section(rlib *r) {}
static void rlib_pdf_start_output_section(rlib *r) {}
static void rlib_pdf_boxurl_end(rlib *r) {}
static void rlib_pdf_draw_cell_background_end(rlib *r) {}
static void rlib_pdf_start_report(rlib *r) {}
static void rlib_pdf_end_report(rlib *r) {}

void rlib_pdf_new_output_filter(rlib *r) {
	OUTPUT(r) = g_malloc(sizeof(struct output_filter));
	OUTPUT_PRIVATE(r) = g_malloc(sizeof(struct _private));
	memset(OUTPUT_PRIVATE(r), 0, sizeof(struct _private));

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
	OUTPUT(r)->rlib_start_report = rlib_pdf_start_report;
	OUTPUT(r)->rlib_end_report = rlib_pdf_end_report;
	OUTPUT(r)->rlib_begin_text = rlib_pdf_begin_text;
	OUTPUT(r)->rlib_finalize_private = rlib_pdf_finalize_private;
	OUTPUT(r)->rlib_spool_private = rlib_pdf_spool_private;
	OUTPUT(r)->rlib_start_line = rlib_pdf_stub_line;
	OUTPUT(r)->rlib_end_line = rlib_pdf_stub_line;
	OUTPUT(r)->rlib_set_working_page = rlib_pdf_set_working_page;
	OUTPUT(r)->rlib_is_single_page = rlib_pdf_is_single_page;
	OUTPUT(r)->rlib_start_output_section = rlib_pdf_start_output_section;
	OUTPUT(r)->rlib_end_output_section = rlib_pdf_end_output_section;
	OUTPUT(r)->rlib_get_output = rlib_pdf_get_output;
	OUTPUT(r)->rlib_get_output_length = rlib_pdf_get_output_length;
	OUTPUT(r)->rlib_free = rlib_pdf_free;
}
