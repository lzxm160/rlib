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
#include <locale.h>
#include <string.h>

#include <cpdflib.h>

#include "config.h"
#include "rlib.h"

#define PDFLOCALE	"en_US"
#define USEPDFLOCALE	1
#define MAX_PDF_PAGES 500

#define BOLD 1
#define ITALICS 2

char *font_names[4] = { "Courier", "Courier-Bold", "Courier-Oblique", "Courier-BoldOblique" };
 

struct _private {
	struct rlib_rgb current_color;
	CPDFdoc *pdf;
	gchar text_on[MAX_PDF_PAGES];
	gchar *buffer;
	gint length;
	gint page_diff;
	gint current_page;
	gboolean is_bold;
	gboolean is_italics;
};


static gfloat rlib_pdf_get_string_width(rlib *r, gchar *text) {
	return cpdf_stringWidth(OUTPUT_PRIVATE(r)->pdf, text)/(RLIB_PDF_DPI);
}


void rlib_pdf_turn_text_off(rlib *r) {
	if(OUTPUT_PRIVATE(r)->text_on[OUTPUT_PRIVATE(r)->current_page] == TRUE) {
		cpdf_endText(OUTPUT_PRIVATE(r)->pdf);
		OUTPUT_PRIVATE(r)->text_on[OUTPUT_PRIVATE(r)->current_page] = FALSE;
	}
}

void rlib_pdf_turn_text_on(rlib *r) {
	if(OUTPUT_PRIVATE(r)->text_on[OUTPUT_PRIVATE(r)->current_page] == FALSE) {
		cpdf_beginText(OUTPUT_PRIVATE(r)->pdf, 0);
		OUTPUT_PRIVATE(r)->text_on[OUTPUT_PRIVATE(r)->current_page] = TRUE;
	}
}

static void rlib_pdf_print_text(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *text, gint backwards, gint col) {
	CPDFdoc *pdf = OUTPUT_PRIVATE(r)->pdf;

#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif

		rlib_pdf_turn_text_on(r);

#if DISABLE_UTF8
		cpdf_text(pdf, left_origin, bottom_origin, 0, text);
#else
	if (!r->current_output_encoder || rlib_char_encoder_isUTF8(r->current_output_encoder)) {
		gchar *tmp = text;
		gchar *buf;
		glong itemsread;
		glong itemswritten;
		GError *error;
		gunichar2 *unistr;
		cpdf_hexStringMode(pdf, YES);
		unistr = g_utf8_to_utf16(tmp, -1, &itemsread, &itemswritten, &error);
		buf = g_malloc(2 * sizeof(gunichar2) * (itemswritten + 2));
		cpdf_convertBinaryToHex((const guchar *) unistr, buf, sizeof(gunichar2) * itemswritten, NO);
		g_free(unistr);
		cpdf_text(pdf, left_origin, bottom_origin, 0, buf);
		g_free(buf);
		cpdf_hexStringMode(pdf, NO);
		r_warning("Using UTF8 output to PDF is not fully supported by CLIBPDF\n"); 
	} else {
		cpdf_text(pdf, left_origin, bottom_origin, 0, text);
	}
#endif	
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}


static void rlib_pdf_set_fg_color(rlib *r, gfloat red, gfloat green, gfloat blue) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	if(OUTPUT_PRIVATE(r)->current_color.r != red || OUTPUT_PRIVATE(r)->current_color.g != green 
	|| OUTPUT_PRIVATE(r)->current_color.b != blue) {
		if(red != -1 && green != -1 && blue != -1 )
			cpdf_setrgbcolor(OUTPUT_PRIVATE(r)->pdf, red, green, blue);
		OUTPUT_PRIVATE(r)->current_color.r = red;
		OUTPUT_PRIVATE(r)->current_color.g = green;
		OUTPUT_PRIVATE(r)->current_color.b = blue;	
	}
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void rlib_pdf_drawbox(rlib *r, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, struct rlib_rgb *color) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	if(!(color->r == 1.0 && color->g == 1.0 && color->b == 1.0)) {
		rlib_pdf_turn_text_off(r);
		//the -.002 seems to get around decimal percision problems.. but should investigate this a big further	
		OUTPUT(r)->set_bg_color(r, color->r, color->g, color->b);
		cpdf_rect(OUTPUT_PRIVATE(r)->pdf, left_origin, bottom_origin, how_long, how_tall-.002);
		cpdf_fill(OUTPUT_PRIVATE(r)->pdf);
		OUTPUT(r)->set_bg_color(r, 0, 0, 0);
	}
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void rlib_pdf_hr(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, 
struct rlib_rgb *color, gfloat indent, gfloat length) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	how_tall = how_tall / RLIB_PDF_DPI;
	rlib_pdf_drawbox(r, left_origin, bottom_origin, how_long, how_tall, color);
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void rlib_pdf_start_boxurl(rlib *r, struct rlib_part *part, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, gchar *url) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	if(part->landscape) {
		gfloat new_left = rlib_layout_get_page_width(r, part)-left_origin-how_long;
		gfloat new_bottom = bottom_origin;
		cpdf_setActionURL(OUTPUT_PRIVATE(r)->pdf, new_bottom, new_left, new_bottom+how_tall, new_left+how_long, url, NULL);
	} else  {
		gfloat new_left = left_origin;
		gfloat new_bottom = bottom_origin;
		cpdf_setActionURL(OUTPUT_PRIVATE(r)->pdf, new_left, new_bottom, new_left+how_long, new_bottom+how_tall, url, NULL);
	}
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void rlib_pdf_drawimage(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *nname, gchar *type, gfloat nwidth, 
gfloat nheight) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	gchar pathbuf[MAXSTRLEN];
	gfloat xscale = 1.0, yscale = 1.0;
	gint realtype=JPEG_IMG;
	if(!strcmp("gif", type))
		realtype = GIF_IMG;
	rlib_pdf_turn_text_off(r);
	cpdf_convertUpathToOS(pathbuf, nname);
	cpdf_importImage(OUTPUT_PRIVATE(r)->pdf, pathbuf, realtype, left_origin, bottom_origin, 0.0, &nwidth, &nheight, &xscale, &yscale, 1);
	OUTPUT(r)->set_bg_color(r, 0, 0, 0);
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void rlib_pdf_set_font_point_actual(rlib *r, gint point) {
	char *encoding;
	char *fontname;
	int which_font = 0;
	int result;
	
	if(OUTPUT_PRIVATE(r)->is_bold)
		which_font += BOLD;

	if(OUTPUT_PRIVATE(r)->is_italics)
		which_font += ITALICS;

	if (*r->pdf_fontdir1) { //if one set other is guaranteed to be set
		cpdf_setFontDirectories(OUTPUT_PRIVATE(r)->pdf, r->pdf_fontdir1, r->pdf_fontdir2);
	}
	encoding = (*r->pdf_encoding)? r->pdf_encoding : NULL;
	fontname = (*r->pdf_fontname)? r->pdf_fontname : font_names[which_font];
#if DISABLE_UTF8
	result = cpdf_setFont(OUTPUT_PRIVATE(r)->pdf, fontname, "WinAnsiEncoding", point);
#else
	result = cpdf_setFont(OUTPUT_PRIVATE(r)->pdf, fontname, encoding, point);
#endif

}

static void rlib_pdf_set_font_point(rlib *r, gint point) {
	if(point == 0)
		point = 8;

	if(r->current_font_point != point) {
		rlib_pdf_set_font_point_actual(r, point);
		r->current_font_point = point;
	}
}
	
static void rlib_pdf_start_new_page(rlib *r, struct rlib_part *part) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	gint i=0;
	gint pages_across = part->pages_across;
	gint page_number = r->current_page_number * pages_across;
	gchar paper_type[40];
	sprintf(paper_type, "0 0 %ld %ld", part->paper->width, part->paper->height);
	for(i=0;i<pages_across;i++) {
		if(part->orientation == RLIB_ORIENTATION_LANDSCAPE) {
			part->position_bottom[i] = (part->paper->width/RLIB_PDF_DPI)-part->bottom_margin;
			cpdf_pageInit(OUTPUT_PRIVATE(r)->pdf, page_number+i, LANDSCAPE, paper_type, paper_type); 
			cpdf_translate(OUTPUT_PRIVATE(r)->pdf, 0.0, (part->paper->height/RLIB_PDF_DPI));	
		   cpdf_rotate(OUTPUT_PRIVATE(r)->pdf, -90.0);
			part->landscape = 1;
		} else {
			part->position_bottom[i] = (part->paper->height/RLIB_PDF_DPI)-part->bottom_margin;
			cpdf_pageInit(OUTPUT_PRIVATE(r)->pdf, page_number+i, PORTRAIT, paper_type, paper_type); 
			part->landscape = 0;
		}
	}
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void rlib_pdf_set_working_page(rlib *r, struct rlib_part *part, gint page) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	gint pages_across = part->pages_across;
	gint page_number = r->current_page_number * pages_across;
	page--;
	cpdf_setCurrentPage(OUTPUT_PRIVATE(r)->pdf, page_number + page - OUTPUT_PRIVATE(r)->page_diff);
	OUTPUT_PRIVATE(r)->current_page = page_number + page - OUTPUT_PRIVATE(r)->page_diff;
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
	setlocale(LC_NUMERIC, tlocale);
}

static void rlib_pdf_set_raw_page(rlib *r, struct rlib_part *part, gint page) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	OUTPUT_PRIVATE(r)->page_diff = r->current_page_number - page;
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
	setlocale(LC_NUMERIC, tlocale);
}


static void rlib_pdf_init_end_page(rlib *r) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
//TODO: Why is this needed?

	if(r->start_of_new_report == TRUE) {
		r->start_of_new_report = FALSE;
	}
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void rlib_pdf_init_output(rlib *r) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	CPDFdoc *pdf;
	CPDFdocLimits dL = {MAX_PDF_PAGES, -1, -1, 10000, 10000};

	r_debug("CPDF version %s\n", cpdf_version() );

	pdf = OUTPUT_PRIVATE(r)->pdf = cpdf_open(0, &dL);
	cpdf_enableCompression(pdf, NO);
//	cpdf_enableCompression(pdf, YES);
	cpdf_init(pdf);
//	OUTPUT_PRIVATE(r)->text_on[OUTPUT_PRIVATE(r)->current_page] = FALSE;
	cpdf_setTitle(pdf, "RLIB Report");
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void rlib_pdf_finalize_private(rlib *r) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	int length;
	rlib_pdf_turn_text_off(r);
	cpdf_finalizeAll(OUTPUT_PRIVATE(r)->pdf);
	OUTPUT_PRIVATE(r)->buffer = cpdf_getBufferForPDF(OUTPUT_PRIVATE(r)->pdf, &length);
	OUTPUT_PRIVATE(r)->length = length;
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void rlib_pdf_spool_private(rlib *r) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	ENVIRONMENT(r)->rlib_write_output(OUTPUT_PRIVATE(r)->buffer, OUTPUT_PRIVATE(r)->length);
	cpdf_close(OUTPUT_PRIVATE(r)->pdf);
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void rlib_pdf_end_page(rlib *r, struct rlib_part *part) {
	int i;
	for(i=0;i<part->pages_across;i++) {
		rlib_pdf_set_working_page(r, part, i+1);
		rlib_pdf_turn_text_off(r);
	}
	r->current_page_number++;
	r->current_line_number = 1;
}

static void rlib_pdf_end_page_again(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	rlib_pdf_turn_text_off(r);
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

static void rlib_pdf_start_td(rlib *r, struct rlib_part *part, gfloat left_margin, gfloat bottom_margin, int width, int height, int border_width, struct rlib_rgb *border_color) {
	struct rlib_rgb color;
	gfloat real_width;
	gfloat real_height;
	gfloat box_width = width / RLIB_PDF_DPI / 100;
	gfloat space = part->position_bottom[0] - part->position_top[0];

	if(part->orientation == RLIB_ORIENTATION_LANDSCAPE) {
		real_height = space * ((gfloat)height/100);
	} else {
		real_height = space * ((gfloat)height/100);
	}


	if(part->orientation == RLIB_ORIENTATION_LANDSCAPE) {
		real_width = ((part->paper->height/RLIB_PDF_DPI) - (part->left_margin*2)) * ((gfloat)width/100);
	} else {
		real_width = ((part->paper->width/RLIB_PDF_DPI) - (part->left_margin*2)) * ((gfloat)width/100);
	}
	
	bzero(&color, sizeof(struct rlib_rgb));
	
	if(border_color == NULL)
		border_color = &color;

	if(border_width > 0) {
		rlib_pdf_drawbox(r, left_margin, bottom_margin - real_height, box_width, real_height, border_color);
		rlib_pdf_drawbox(r, left_margin + real_width - (box_width * 2), bottom_margin - real_height, box_width, real_height, border_color);

		rlib_pdf_drawbox(r, left_margin, bottom_margin - real_height, real_width, box_width, border_color);
		rlib_pdf_drawbox(r, left_margin, bottom_margin, real_width, box_width, border_color);
	}
}

static void rlib_pdf_start_bold(rlib *r) {
	OUTPUT_PRIVATE(r)->is_bold = TRUE;
	rlib_pdf_set_font_point_actual(r, r->current_font_point);
}

static void rlib_pdf_end_bold(rlib *r) {
	OUTPUT_PRIVATE(r)->is_bold = FALSE;
	rlib_pdf_set_font_point_actual(r, r->current_font_point);
}

static void rlib_pdf_start_italics(rlib *r) {
	OUTPUT_PRIVATE(r)->is_italics = TRUE;
	rlib_pdf_set_font_point_actual(r, r->current_font_point);
}

static void rlib_pdf_end_italics(rlib *r) {
	OUTPUT_PRIVATE(r)->is_italics = FALSE;
	rlib_pdf_set_font_point_actual(r, r->current_font_point);
}

static void rlib_pdf_end_td(rlib *r) {}
static void rlib_pdf_stub_line(rlib *r, int backwards) {}
static void rlib_pdf_end_output_section(rlib *r) {}
static void rlib_pdf_start_output_section(rlib *r) {}
static void rlib_pdf_end_boxurl(rlib *r) {}
static void rlib_pdf_end_draw_cell_background(rlib *r) {}
static void rlib_pdf_start_report(rlib *r, struct rlib_part *part) {}
static void rlib_pdf_end_report(rlib *r, struct rlib_part *part) {}
static void rlib_pdf_start_tr(rlib *r) {}
static void rlib_pdf_end_tr(rlib *r) {}

void rlib_pdf_new_output_filter(rlib *r) {
	OUTPUT(r) = g_malloc(sizeof(struct output_filter));
	r->o->private = g_malloc(sizeof(struct _private));
	memset(OUTPUT_PRIVATE(r), 0, sizeof(struct _private));

	OUTPUT(r)->do_align = TRUE;
	OUTPUT(r)->do_break = TRUE;
	OUTPUT(r)->do_grouptext = TRUE;	
	OUTPUT(r)->paginate = TRUE;

	OUTPUT(r)->get_string_width = rlib_pdf_get_string_width;
	OUTPUT(r)->print_text = rlib_pdf_print_text;
	OUTPUT(r)->set_fg_color = rlib_pdf_set_fg_color;
	OUTPUT(r)->set_bg_color = rlib_pdf_set_fg_color;
	OUTPUT(r)->start_draw_cell_background = rlib_pdf_drawbox;
	OUTPUT(r)->end_draw_cell_background = rlib_pdf_end_draw_cell_background;
	OUTPUT(r)->hr = rlib_pdf_hr;
	OUTPUT(r)->start_boxurl = rlib_pdf_start_boxurl;
	OUTPUT(r)->end_boxurl = rlib_pdf_end_boxurl;
	OUTPUT(r)->drawimage = rlib_pdf_drawimage;
	OUTPUT(r)->set_font_point = rlib_pdf_set_font_point;
	OUTPUT(r)->start_new_page = rlib_pdf_start_new_page;
	OUTPUT(r)->end_page = rlib_pdf_end_page;
	OUTPUT(r)->end_page_again = rlib_pdf_end_page_again;
	OUTPUT(r)->init_end_page = rlib_pdf_init_end_page;
	OUTPUT(r)->init_output = rlib_pdf_init_output;
	OUTPUT(r)->start_report = rlib_pdf_start_report;
	OUTPUT(r)->end_report = rlib_pdf_end_report;
	OUTPUT(r)->finalize_private = rlib_pdf_finalize_private;
	OUTPUT(r)->spool_private = rlib_pdf_spool_private;
	OUTPUT(r)->start_line = rlib_pdf_stub_line;
	OUTPUT(r)->end_line = rlib_pdf_stub_line;
	OUTPUT(r)->set_working_page = rlib_pdf_set_working_page;
	OUTPUT(r)->set_raw_page = rlib_pdf_set_raw_page;
	OUTPUT(r)->start_output_section = rlib_pdf_start_output_section;
	OUTPUT(r)->end_output_section = rlib_pdf_end_output_section;
	OUTPUT(r)->get_output = rlib_pdf_get_output;
	OUTPUT(r)->get_output_length = rlib_pdf_get_output_length;
	OUTPUT(r)->start_tr = rlib_pdf_start_tr;
	OUTPUT(r)->end_tr = rlib_pdf_end_tr;
	OUTPUT(r)->start_td = rlib_pdf_start_td;
	OUTPUT(r)->end_td = rlib_pdf_end_td;
	OUTPUT(r)->start_bold = rlib_pdf_start_bold;
	OUTPUT(r)->end_bold = rlib_pdf_end_bold;
	OUTPUT(r)->start_italics = rlib_pdf_start_italics;
	OUTPUT(r)->end_italics = rlib_pdf_end_italics;
	OUTPUT(r)->free = rlib_pdf_free;
}
