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
#include <math.h>

#include <cpdflib.h>

#include "config.h"
#include "rlib.h"

#define PDFLOCALE	"en_US"
#define USEPDFLOCALE	1
#define MAX_PDF_PAGES 500

#define BOLD 1
#define ITALICS 2

#define PDF_PIXEL 0.002

gchar *font_names[4] = { "Courier", "Courier-Bold", "Courier-Oblique", "Courier-BoldOblique" };

struct _graph {
	gdouble y_min;
	gdouble y_max;
	gdouble y_origin;
	gdouble non_standard_x_start;
	gfloat tmp_y_offset;
	gfloat y_label_width;
	gfloat x_label_width;
	gfloat x_tick_width;
	gfloat legend_width;
	gfloat top;
	gfloat bottom;
	gfloat left;
	gfloat width_before_legend;
	gfloat width;
	gfloat height;
	gfloat x_start;
	gfloat x_width;
	gfloat y_start;
	gfloat y_height;
	gfloat intersection;
	gfloat height_offset;
	gfloat width_offset;
	gfloat title_height;
	gint x_iterations;
	gint y_iterations;
	gint data_plot_count;
	gboolean x_axis_labels_are_under_tick;
	
	gfloat legend_top;
	gfloat legend_left;
};

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
	struct _graph graph;
};

static gfloat pdf_get_string_width(rlib *r, gchar *text) {
	return cpdf_stringWidth(OUTPUT_PRIVATE(r)->pdf, text)/(RLIB_PDF_DPI);
}

void pdf_turn_text_off(rlib *r) {
	if(OUTPUT_PRIVATE(r)->text_on[OUTPUT_PRIVATE(r)->current_page] == TRUE) {
		cpdf_endText(OUTPUT_PRIVATE(r)->pdf);
		OUTPUT_PRIVATE(r)->text_on[OUTPUT_PRIVATE(r)->current_page] = FALSE;
	}
}

void pdf_turn_text_on(rlib *r) {
	if(OUTPUT_PRIVATE(r)->text_on[OUTPUT_PRIVATE(r)->current_page] == FALSE) {
		cpdf_beginText(OUTPUT_PRIVATE(r)->pdf, 0);
		OUTPUT_PRIVATE(r)->text_on[OUTPUT_PRIVATE(r)->current_page] = TRUE;
	}
}

static void pdf_print_text(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *text, gfloat orientation) {
	CPDFdoc *pdf = OUTPUT_PRIVATE(r)->pdf;

#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif

	pdf_turn_text_on(r);

	cpdf_text(pdf, left_origin, bottom_origin, orientation, text);

#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif


}

static void pdf_print_text_API(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *text, gint backwards, gint col) {
	pdf_print_text(r, left_origin, bottom_origin, text, 0); 
}


static void pdf_set_fg_color(rlib *r, gfloat red, gfloat green, gfloat blue) {
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

static void pdf_drawbox(rlib *r, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, struct rlib_rgb *color) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	if(!(color->r == 1.0 && color->g == 1.0 && color->b == 1.0)) {
		pdf_turn_text_off(r);
		//the - PDF_PIXEL seems to get around decimal percision problems.. but should investigate this a big further	
		OUTPUT(r)->set_bg_color(r, color->r, color->g, color->b);
		cpdf_rect(OUTPUT_PRIVATE(r)->pdf, left_origin, bottom_origin, how_long, how_tall-PDF_PIXEL);
		cpdf_fill(OUTPUT_PRIVATE(r)->pdf);
		OUTPUT(r)->set_bg_color(r, 0, 0, 0);
	}
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void pdf_hr(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, 
struct rlib_rgb *color, gfloat indent, gfloat length) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	how_tall = how_tall / RLIB_PDF_DPI;
	pdf_drawbox(r, left_origin, bottom_origin, how_long, how_tall, color);
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void pdf_start_boxurl(rlib *r, struct rlib_part *part, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, gchar *url) {
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

static void pdf_drawimage(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *nname, gchar *type, gfloat nwidth, 
gfloat nheight) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	gchar pathbuf[MAXSTRLEN];
	gfloat xscale = 1.0, yscale = 1.0;
	gint realtype=JPEG_IMG;
	if(!strcmp("gif", type))
		realtype = GIF_IMG;
	pdf_turn_text_off(r);
	cpdf_convertUpathToOS(pathbuf, nname);
	cpdf_importImage(OUTPUT_PRIVATE(r)->pdf, pathbuf, realtype, left_origin, bottom_origin, 0.0, &nwidth, &nheight, &xscale, &yscale, 1);
	OUTPUT(r)->set_bg_color(r, 0, 0, 0);
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void pdf_set_font_point_actual(rlib *r, gint point) {
	char *encoding;
	char *fontname;
	int which_font = 0;
	int result;
	gchar *pdfdir1, *pdfdir2, *pdfencoding, *pdffontname;
	
	pdfdir1 = g_hash_table_lookup(r->output_paramaters, "pdf_fontdir1");
	pdfdir2 = g_hash_table_lookup(r->output_paramaters, "pdf_fontdir2");
	pdfencoding = g_hash_table_lookup(r->output_paramaters, "pdf_encoding");
	pdffontname = g_hash_table_lookup(r->output_paramaters, "pdf_fontname");

	if(pdfdir2 == NULL)
		pdfdir2 = pdfdir1;
	
	if(OUTPUT_PRIVATE(r)->is_bold)
		which_font += BOLD;

	if(OUTPUT_PRIVATE(r)->is_italics)
		which_font += ITALICS;

	if (pdfdir1) 
		cpdf_setFontDirectories(OUTPUT_PRIVATE(r)->pdf, pdfdir1, pdfdir2);

	encoding = pdfencoding;
	fontname = pdffontname ? pdffontname : font_names[which_font];
	
	result = cpdf_setFont(OUTPUT_PRIVATE(r)->pdf, fontname, "WinAnsiEncoding", point);

}

static void pdf_set_font_point(rlib *r, gint point) {
	if(point == 0)
		point = 8;

	if(r->current_font_point != point) {
		pdf_set_font_point_actual(r, point);
		r->current_font_point = point;
	}
}
	
static void pdf_start_new_page(rlib *r, struct rlib_part *part) {
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

static void pdf_set_working_page(rlib *r, struct rlib_part *part, gint page) {
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

static void pdf_set_raw_page(rlib *r, struct rlib_part *part, gint page) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif

	OUTPUT_PRIVATE(r)->page_diff = r->current_page_number - page;
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
	setlocale(LC_NUMERIC, tlocale);
}


static void pdf_init_end_page(rlib *r) {
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

static void pdf_init_output(rlib *r) {
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

static void pdf_finalize_private(rlib *r) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	int length;
	pdf_turn_text_off(r);
	cpdf_finalizeAll(OUTPUT_PRIVATE(r)->pdf);
	OUTPUT_PRIVATE(r)->buffer = cpdf_getBufferForPDF(OUTPUT_PRIVATE(r)->pdf, &length);
	OUTPUT_PRIVATE(r)->length = length;
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void pdf_spool_private(rlib *r) {
#if USEPDFLOCALE
	char *tlocale = setlocale(LC_NUMERIC, PDFLOCALE);
#endif
	ENVIRONMENT(r)->rlib_write_output(OUTPUT_PRIVATE(r)->buffer, OUTPUT_PRIVATE(r)->length);
	cpdf_close(OUTPUT_PRIVATE(r)->pdf);
#if USEPDFLOCALE
	setlocale(LC_NUMERIC, tlocale);
#endif
}

static void pdf_end_page(rlib *r, struct rlib_part *part) {
	int i;
	for(i=0;i<part->pages_across;i++) {
		pdf_set_working_page(r, part, i+1);
		pdf_turn_text_off(r);
	}
	r->current_page_number++;
	r->current_line_number = 1;
}

static void pdf_end_page_again(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	pdf_turn_text_off(r);
}

static int pdf_free(rlib *r) {
	g_free(OUTPUT_PRIVATE(r));
	g_free(OUTPUT(r));
	return 0;
}

static char *pdf_get_output(rlib *r) {
	return OUTPUT_PRIVATE(r)->buffer;
}

static long pdf_get_output_length(rlib *r) {
	return OUTPUT_PRIVATE(r)->length;
}

static void pdf_start_td(rlib *r, struct rlib_part *part, gfloat left_margin, gfloat bottom_margin, int width, int height, int border_width, struct rlib_rgb *border_color) {
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
		pdf_drawbox(r, left_margin, bottom_margin - real_height, box_width, real_height, border_color);
		pdf_drawbox(r, left_margin + real_width - (box_width * 2), bottom_margin - real_height, box_width, real_height, border_color);

		pdf_drawbox(r, left_margin, bottom_margin - real_height, real_width, box_width, border_color);
		pdf_drawbox(r, left_margin, bottom_margin, real_width, box_width, border_color);
	}
}

static void pdf_start_bold(rlib *r) {
	OUTPUT_PRIVATE(r)->is_bold = TRUE;
	pdf_set_font_point_actual(r, r->current_font_point);
}

static void pdf_end_bold(rlib *r) {
	OUTPUT_PRIVATE(r)->is_bold = FALSE;
	pdf_set_font_point_actual(r, r->current_font_point);
}

static void pdf_start_italics(rlib *r) {
	OUTPUT_PRIVATE(r)->is_italics = TRUE;
	pdf_set_font_point_actual(r, r->current_font_point);
}

static void pdf_end_italics(rlib *r) {
	OUTPUT_PRIVATE(r)->is_italics = FALSE;
	pdf_set_font_point_actual(r, r->current_font_point);
}

static void pdf_end_report(rlib *r, struct rlib_report *report) {
	pdf_turn_text_off(r);
}

static void pdf_graph_draw_line(rlib *r, gfloat x, gfloat y, gfloat new_x, gfloat new_y, struct rlib_rgb *color) {
	pdf_turn_text_off(r);
	cpdf_moveto(OUTPUT_PRIVATE(r)->pdf, x, y);
	cpdf_lineto(OUTPUT_PRIVATE(r)->pdf, new_x, new_y);

	cpdf_closepath(OUTPUT_PRIVATE(r)->pdf); 
	cpdf_stroke(OUTPUT_PRIVATE(r)->pdf); 
}

static void pdf_graph_start(rlib *r, gfloat left, gfloat top, gfloat width, gfloat height, gboolean x_axis_labels_are_under_tick) {
	bzero(&OUTPUT_PRIVATE(r)->graph, sizeof(struct _graph));
	
	width /= RLIB_PDF_DPI;
	height /= RLIB_PDF_DPI;
	
	OUTPUT_PRIVATE(r)->graph.top = top;
	OUTPUT_PRIVATE(r)->graph.bottom = top - height;
	OUTPUT_PRIVATE(r)->graph.left = left;
	OUTPUT_PRIVATE(r)->graph.width = width;
	OUTPUT_PRIVATE(r)->graph.width_before_legend = width;
	OUTPUT_PRIVATE(r)->graph.height = height;
	OUTPUT_PRIVATE(r)->graph.intersection = .1;
	OUTPUT_PRIVATE(r)->graph.x_axis_labels_are_under_tick = x_axis_labels_are_under_tick;
}

static void pdf_graph_set_limits(rlib *r, gdouble min, gdouble max, gdouble origin) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	graph->y_min = min;
	graph->y_max = max;
	graph->y_origin = origin;
}

static void pdf_graph_title(rlib *r, gchar *title) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	gfloat title_width = pdf_get_string_width(r, title);
	graph->title_height = RLIB_GET_LINE(r->current_font_point);

	pdf_print_text(r, graph->left + ((graph->width-title_width)/2.0), graph->top-graph->title_height, title, 0);
}

static void pdf_graph_x_axis_title(rlib *r, gchar *title) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	if(title[0] == 0)
		graph->height_offset = RLIB_GET_LINE(r->current_font_point) / 2.0;
	else {
		gfloat title_width = pdf_get_string_width(r, title);
		graph->height_offset = RLIB_GET_LINE(r->current_font_point);
		pdf_print_text(r, graph->left + ((graph->width-title_width)/2.0), graph->bottom+(graph->height_offset/2.0), title, 0);
		graph->height_offset *= 1.5;
	}
}

static void pdf_graph_y_axis_title(rlib *r, gchar *title) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	gfloat title_width = pdf_get_string_width(r, title);
	if(title[0] == 0) {
	
	} else {
		pdf_print_text(r, graph->left+(pdf_get_string_width(r, "W")*1.25), graph->bottom+((graph->height - title_width)/2.0), title,  90);
		graph->tmp_y_offset = pdf_get_string_width(r, "W") * 1.5;
	}

/*	
	
	graph->title_height = RLIB_GET_LINE(r->current_font_point);
	pdf_print_text(r, graph->left + ((graph->width-title_width)/2.0), graph->top-graph->title_height, title, FALSE, 0);*/
}

static void pdf_graph_do_grid(rlib *r, gboolean just_a_box) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;

	graph->width_offset = graph->tmp_y_offset + graph->y_label_width + graph->intersection;

	pdf_graph_draw_line(r, graph->left, graph->bottom, graph->left, graph->top, NULL);
	pdf_graph_draw_line(r, graph->left+graph->width_before_legend, graph->bottom, graph->left+graph->width_before_legend, graph->top, NULL);
	pdf_graph_draw_line(r, graph->left, graph->bottom, graph->left+graph->width_before_legend, graph->bottom, NULL);
	pdf_graph_draw_line(r, graph->left, graph->top, graph->left+graph->width_before_legend, graph->top, NULL);

	graph->top -= graph->title_height*1.1;
	graph->height -= graph->title_height*1.1;

	graph->height_offset += graph->intersection;
	
	graph->x_width = graph->width - graph->width_offset - graph->intersection;
	
	if(graph->x_axis_labels_are_under_tick)	
		graph->x_tick_width = graph->x_width/(graph->x_iterations-1);
	else
		graph->x_tick_width = graph->x_width/graph->x_iterations;

	//Make more room for the x axis label is we need to rotate the text
	if(graph->x_label_width > (graph->x_tick_width))
		graph->height_offset += graph->x_label_width - 	RLIB_GET_LINE(r->current_font_point);

	
	if(graph->x_axis_labels_are_under_tick)
		graph->height_offset += graph->intersection;

	if(!just_a_box) {
		pdf_graph_draw_line(r, graph->left+graph->width_offset, graph->bottom+graph->height_offset-graph->intersection, graph->left+graph->width_offset, graph->top-graph->intersection, NULL);
		pdf_graph_draw_line(r, graph->left+graph->width_offset-graph->intersection, graph->bottom+graph->height_offset, graph->left+graph->width-graph->intersection, graph->bottom+graph->height_offset, NULL);
	}
	
	graph->x_start = graph->left+graph->width_offset;


	graph->y_start = graph->bottom + graph->height_offset;
	graph->y_height = graph->height - graph->height_offset - graph->intersection;

}

static void pdf_graph_tick_x(rlib *r) {
	gfloat i;
	gfloat spot;
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;

	graph->height_offset	+= graph->intersection;
	
	for(i=0;i<graph->x_iterations;i++) {
		spot = graph->x_start + ((graph->x_tick_width)*i);
		pdf_graph_draw_line(r, spot, graph->y_start-(graph->intersection), spot, graph->y_start, NULL);
	}

}

static void pdf_graph_set_x_iterations(rlib *r, gint iterations) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	graph->x_iterations = iterations;
}

static void pdf_graph_hint_label_x(rlib *r, gchar *label) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	gfloat string_width = pdf_get_string_width(r, label);
	if(string_width > graph->x_label_width)
		graph->x_label_width = string_width;
}

static void pdf_graph_label_x(rlib *r, gint iteration, gchar *label) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	gfloat rotation = 0;
	gfloat white_space = graph->x_tick_width;
	gfloat left = graph->x_start + (white_space * iteration);
	gfloat string_width = pdf_get_string_width(r, label);
	gfloat w_width = pdf_get_string_width(r, "W");
	gfloat height = RLIB_GET_LINE(r->current_font_point);
	gfloat y_offset = RLIB_GET_LINE(r->current_font_point);
	
	if(graph->x_label_width > (graph->x_tick_width)) {
		y_offset = 0;
		rotation = -90;
		left += (white_space - w_width) / 2;
		height = w_width;
		if(graph->x_axis_labels_are_under_tick)
			y_offset += graph->intersection / 2;
	} else {
		if(string_width < white_space)
			left += (white_space - string_width) / 2;
	}
	
	if(graph->x_axis_labels_are_under_tick) {
		left -= white_space / 2;
		y_offset += graph->intersection / 2;
	} else
		y_offset /= 1.2;

	pdf_print_text(r, left, graph->y_start - y_offset, label, rotation);
}

static void pdf_graph_tick_y(rlib *r, gint iterations) {
	gfloat i;
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	graph->y_iterations = iterations;
	for(i=1;i<iterations+1;i++) {
		gfloat y = graph->y_start + ((graph->y_height/iterations) * i);
		pdf_graph_draw_line(r, graph->left+graph->width_offset-graph->intersection, y, graph->left+graph->width-graph->intersection, y, NULL);
	}

}

static void pdf_graph_label_y(rlib *r, gint iteration, gchar *label, gboolean false_x) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	gfloat white_space = graph->y_height/graph->y_iterations;
	gfloat line_width = RLIB_GET_LINE(r->current_font_point) / 3.0;
	gfloat top = graph->y_start + (white_space * iteration) - line_width;
//	if(false_x) {
//		graph->non_standard_x_start = graph->y_start + (white_space * iteration);
//	}
	pdf_print_text(r, graph->left+graph->tmp_y_offset, top, label, 0);
}

static void pdf_graph_hint_label_y(rlib *r, gchar *label) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	gfloat width =  pdf_get_string_width(r, label);
	if(width > graph->y_label_width)
		graph->y_label_width = width;
};

static void pdf_graph_set_data_plot_count(rlib *r, gint count) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	graph->data_plot_count = count;
}


static void pdf_graph_plot_bar(rlib *r, gint iteration, gint plot, gfloat height_percent, struct rlib_rgb *color,gfloat last_height, gboolean divide_iterations) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;	
	gfloat bar_width = graph->x_tick_width *.6;
	gfloat left = graph->x_start + (graph->x_tick_width * iteration) + (graph->x_tick_width *.2);
	gfloat start = graph->y_start;
	pdf_turn_text_off(r);

	if(graph->y_origin != graph->y_min)  {
		gfloat n = fabs(graph->y_max)+fabs(graph->y_origin);
		gfloat d = fabs(graph->y_min)+fabs(graph->y_max);
		gfloat real_height =  1 - (n / d);				
		start += (real_height * graph->y_height);
	}
	
	if(divide_iterations)
		bar_width /= graph->data_plot_count;

	left += (bar_width)*plot;	
	bar_width -= (PDF_PIXEL * 4);
	OUTPUT(r)->set_bg_color(r, color->r, color->g, color->b);
	cpdf_rect(OUTPUT_PRIVATE(r)->pdf, left, start + last_height*graph->y_height, bar_width, graph->y_height*(height_percent));
	cpdf_fill(OUTPUT_PRIVATE(r)->pdf);
	OUTPUT(r)->set_bg_color(r, 0, 0, 0);
}

void pdf_graph_plot_line(rlib *r, int iteration, gfloat p1_height, gfloat p1_last_height, gfloat p2_height, gfloat p2_last_height, struct rlib_rgb * color) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	gfloat p1_start = graph->y_start;
	gfloat p2_start = graph->y_start;
	gfloat left = graph->x_start + (graph->x_tick_width * (iteration-1));

	p1_height += p1_last_height;
	p2_height += p2_last_height;

	pdf_turn_text_off(r);
	if(graph->y_origin != graph->y_min)  {
		gfloat n = fabs(graph->y_max)+fabs(graph->y_origin);
		gfloat d = fabs(graph->y_min)+fabs(graph->y_max);
		gfloat real_height =  1 - (n / d);				
		p1_start += (real_height * graph->y_height);
		p2_start += (real_height * graph->y_height);
	}
	
	OUTPUT(r)->set_bg_color(r, color->r, color->g, color->b);
	pdf_graph_draw_line(r, left, p1_start + (graph->y_height * p1_height), left+graph->x_tick_width, p2_start + (graph->y_height * p2_height), NULL);
	OUTPUT(r)->set_bg_color(r, 0, 0, 0);
}

static void pdf_graph_plot_pie(rlib *r, gfloat start, gfloat end, gboolean offset, struct rlib_rgb *color) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	gfloat start_angle = 360.0 * start;
	gfloat end_angle = 360.0 * end;
	gfloat x = graph->x_start + (graph->x_width / 2);
	gfloat y = graph->y_start + (graph->y_height / 2);
	gfloat radius = 0;
	gfloat offset_factor = 0;
	gfloat rads;
	
	if(graph->x_width < graph->y_height)
		radius = graph->x_width / 2;
	else
		radius = graph->y_height /2 ;

	if(offset)
		offset_factor = radius * .1;
	else	
		offset_factor = radius *.01;

	rads = (((start_angle+end_angle)/2.0))*3.1415927/180.0;
	x += offset_factor * cosf(rads);
	y += offset_factor * sinf(rads);
	radius -= offset_factor;

	pdf_turn_text_off(r);
	OUTPUT(r)->set_bg_color(r, color->r, color->g, color->b);
	cpdf_moveto(OUTPUT_PRIVATE(r)->pdf, x, y);
	cpdf_arc(OUTPUT_PRIVATE(r)->pdf, x, y, radius, start_angle, end_angle, 0);
	cpdf_closepath(OUTPUT_PRIVATE(r)->pdf);
	cpdf_fillAndStroke(OUTPUT_PRIVATE(r)->pdf);
	OUTPUT(r)->set_bg_color(r, 0, 0, 0);
}

static void pdf_graph_hint_legend(rlib *r, gchar *label) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	gfloat width =  pdf_get_string_width(r, label) + pdf_get_string_width(r, "WWW");

	if(width > graph->legend_width)
		graph->legend_width = width;
};

static void pdf_graph_draw_legend(rlib *r) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	gfloat left = graph->left+graph->width - graph->legend_width;
	gfloat width = graph->legend_width;
	gfloat height = (RLIB_GET_LINE(r->current_font_point) * graph->data_plot_count) + (RLIB_GET_LINE(r->current_font_point) / 2);
	gfloat top = graph->top;
	gfloat bottom = graph->top - height;

	pdf_graph_draw_line(r, left, bottom, left, top, NULL);
	pdf_graph_draw_line(r, left+width, bottom, left+width, top, NULL);
	pdf_graph_draw_line(r, left, bottom, left+width, bottom, NULL);
	pdf_graph_draw_line(r, left, top, left+width, top, NULL);
	
	graph->legend_top = top;
	graph->legend_left = left;
	
	graph->width -= width;
}

static void pdf_graph_draw_legend_label(rlib *r, gint iteration, gchar *label, struct rlib_rgb *color) {
	struct _graph *graph = &OUTPUT_PRIVATE(r)->graph;
	gfloat offset =  ((iteration + 1) * RLIB_GET_LINE(r->current_font_point) );
	gfloat w_width = pdf_get_string_width(r, "W");
	gfloat line_height = RLIB_GET_LINE(r->current_font_point);

	OUTPUT(r)->set_bg_color(r, color->r, color->g, color->b);
	pdf_turn_text_off(r);
	cpdf_rect(OUTPUT_PRIVATE(r)->pdf, graph->legend_left + (w_width/2), graph->legend_top - offset , w_width, line_height*.6);
	cpdf_fill(OUTPUT_PRIVATE(r)->pdf);
	pdf_turn_text_on(r);
	OUTPUT(r)->set_bg_color(r, 0, 0, 0);
	pdf_print_text(r, graph->legend_left + (w_width*2), graph->legend_top - offset, label, 0);
}

static void pdf_end_td(rlib *r) {}
static void pdf_stub_line(rlib *r, int backwards) {}
static void pdf_end_output_section(rlib *r) {}
static void pdf_start_output_section(rlib *r) {}
static void pdf_end_boxurl(rlib *r) {}
static void pdf_end_draw_cell_background(rlib *r) {}
static void pdf_start_report(rlib *r, struct rlib_part *part) {}
static void pdf_end_part(rlib *r, struct rlib_part *part) {}
static void pdf_start_tr(rlib *r) {}
static void pdf_end_tr(rlib *r) {}

void rlib_pdf_new_output_filter(rlib *r) {
	OUTPUT(r) = g_malloc(sizeof(struct output_filter));
	r->o->private = g_malloc(sizeof(struct _private));
	memset(OUTPUT_PRIVATE(r), 0, sizeof(struct _private));

	OUTPUT(r)->do_align = TRUE;
	OUTPUT(r)->do_break = TRUE;
	OUTPUT(r)->do_grouptext = TRUE;	
	OUTPUT(r)->paginate = TRUE;

	OUTPUT(r)->get_string_width = pdf_get_string_width;
	OUTPUT(r)->print_text = pdf_print_text_API;
	OUTPUT(r)->set_fg_color = pdf_set_fg_color;
	OUTPUT(r)->set_bg_color = pdf_set_fg_color;
	OUTPUT(r)->start_draw_cell_background = pdf_drawbox;
	OUTPUT(r)->end_draw_cell_background = pdf_end_draw_cell_background;
	OUTPUT(r)->hr = pdf_hr;
	OUTPUT(r)->start_boxurl = pdf_start_boxurl;
	OUTPUT(r)->end_boxurl = pdf_end_boxurl;

	OUTPUT(r)->drawimage = pdf_drawimage;
	OUTPUT(r)->set_font_point = pdf_set_font_point;
	OUTPUT(r)->start_new_page = pdf_start_new_page;
	OUTPUT(r)->end_page = pdf_end_page;
	OUTPUT(r)->end_page_again = pdf_end_page_again;
	OUTPUT(r)->init_end_page = pdf_init_end_page;
	OUTPUT(r)->init_output = pdf_init_output;
	OUTPUT(r)->start_report = pdf_start_report;
	OUTPUT(r)->end_report = pdf_end_report;
	OUTPUT(r)->end_part = pdf_end_part;
	OUTPUT(r)->finalize_private = pdf_finalize_private;
	OUTPUT(r)->spool_private = pdf_spool_private;
	OUTPUT(r)->start_line = pdf_stub_line;
	OUTPUT(r)->end_line = pdf_stub_line;
	OUTPUT(r)->set_working_page = pdf_set_working_page;
	OUTPUT(r)->set_raw_page = pdf_set_raw_page;
	OUTPUT(r)->start_output_section = pdf_start_output_section;
	OUTPUT(r)->end_output_section = pdf_end_output_section;
	OUTPUT(r)->get_output = pdf_get_output;
	OUTPUT(r)->get_output_length = pdf_get_output_length;
	OUTPUT(r)->start_tr = pdf_start_tr;
	OUTPUT(r)->end_tr = pdf_end_tr;
	OUTPUT(r)->start_td = pdf_start_td;
	OUTPUT(r)->end_td = pdf_end_td;
	OUTPUT(r)->start_bold = pdf_start_bold;
	OUTPUT(r)->end_bold = pdf_end_bold;
	OUTPUT(r)->start_italics = pdf_start_italics;
	OUTPUT(r)->end_italics = pdf_end_italics;
	
	OUTPUT(r)->graph_start = pdf_graph_start;
	OUTPUT(r)->graph_set_limits = pdf_graph_set_limits;
	OUTPUT(r)->graph_title = pdf_graph_title;
	OUTPUT(r)->graph_x_axis_title = pdf_graph_x_axis_title;
	OUTPUT(r)->graph_y_axis_title = pdf_graph_y_axis_title;
	OUTPUT(r)->graph_do_grid = pdf_graph_do_grid;
	OUTPUT(r)->graph_tick_x = pdf_graph_tick_x;
	OUTPUT(r)->graph_set_x_iterations = pdf_graph_set_x_iterations;
	OUTPUT(r)->graph_tick_y = pdf_graph_tick_y;
	OUTPUT(r)->graph_hint_label_x = pdf_graph_hint_label_x;
	OUTPUT(r)->graph_label_x = pdf_graph_label_x;
	OUTPUT(r)->graph_label_y = pdf_graph_label_y;
	OUTPUT(r)->graph_draw_line = pdf_graph_draw_line;
	OUTPUT(r)->graph_plot_bar = pdf_graph_plot_bar;
	OUTPUT(r)->graph_plot_line = pdf_graph_plot_line;
	OUTPUT(r)->graph_plot_pie = pdf_graph_plot_pie;
	OUTPUT(r)->graph_set_data_plot_count = pdf_graph_set_data_plot_count;
	OUTPUT(r)->graph_hint_label_y = pdf_graph_hint_label_y;
	OUTPUT(r)->graph_hint_legend = pdf_graph_hint_legend;
	OUTPUT(r)->graph_draw_legend = pdf_graph_draw_legend;
	OUTPUT(r)->graph_draw_legend_label = pdf_graph_draw_legend_label;
	OUTPUT(r)->free = pdf_free;
}
