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
#include <string.h>

#include "ralloc.h"
#include "rlib.h"

struct _private {
	struct rgb current_fg_color;
	struct rgb current_bg_color;
	char *top;
	int top_size;
	int top_total_size;
	char *bottom;
	int bottom_size;
	int bottom_total_size;
	int did_bg;
	int bg_backwards;
	int do_bg;
};

static void make_more_space_if_necessary(char **str, int *size, int *total_size, int len) {
	if(*total_size == 0) {
		*str = rcalloc(MAXSTRLEN, 1);
		*total_size = MAXSTRLEN;
	} else if((*size) + len > (*total_size)) {
		*str = rrealloc(*str, (*total_size)*2);
		*total_size = (*total_size) * 2;
	}		
}

static void print_text(rlib *r, char *text, int backwards) {
	char *str_ptr;
	int text_size = strlen(text);
	int *size;

	if(backwards) {
		make_more_space_if_necessary(&OUTPUT_PRIVATE(r)->bottom, &OUTPUT_PRIVATE(r)->bottom_size, 
			&OUTPUT_PRIVATE(r)->bottom_total_size, text_size);
		str_ptr = OUTPUT_PRIVATE(r)->bottom;
		size = &OUTPUT_PRIVATE(r)->bottom_size;
	} else {
		make_more_space_if_necessary(&OUTPUT_PRIVATE(r)->top, &OUTPUT_PRIVATE(r)->top_size, 
			&OUTPUT_PRIVATE(r)->top_total_size, text_size);
		str_ptr = OUTPUT_PRIVATE(r)->top;	
		size = &OUTPUT_PRIVATE(r)->top_size;
	}
	memcpy(str_ptr + (*size), text, text_size+1);
	*size = (*size) + text_size;
}

static float rlib_html_get_string_width(rlib *r, char *text) {
	return 1;
}

static char *get_html_color(char *str, struct rgb *color) {
	sprintf(str, "#%02x%02x%02x", (int)(color->r*0xFF), 
		(int)(color->g*0xFF), (int)(color->b*0xFF));
	return str;
}

static int convert_font_point(int point) {
	if(point <= 8)
		return 1;
	else if(point <= 10)
		return 2;
	else if(point <= 12)
		return 3;
	else if(point <= 14)
		return 4;
	else if(point <= 18)
		return 5;
	else if(point <= 24)
		return 6;
	else 
		return 7;
}

static void rlib_html_print_text(rlib *r, float left_origin, float bottom_origin, char *text, int backwards, int col) {
	int did_fg = 0;
	int did_fp = 0;
	char buf_font[MAXSTRLEN];

	OUTPUT_PRIVATE(r)->bg_backwards = backwards;
	

	if(OUTPUT_PRIVATE(r)->current_bg_color.r >= 0 && OUTPUT_PRIVATE(r)->current_bg_color.g >= 0 
	&& OUTPUT_PRIVATE(r)->current_bg_color.b >= 0 && !((OUTPUT_PRIVATE(r)->current_bg_color.r == 1.0 
	&& OUTPUT_PRIVATE(r)->current_bg_color.g == 1.0 && OUTPUT_PRIVATE(r)->current_bg_color.b == 1.0)) &&
	OUTPUT_PRIVATE(r)->do_bg) {
		char buf[MAXSTRLEN];
		char color[40];
		get_html_color(color, &OUTPUT_PRIVATE(r)->current_bg_color);
		sprintf(buf, "<span style=\"background-color: %s\">", color);
		print_text(r, buf, backwards);
		OUTPUT_PRIVATE(r)->did_bg = TRUE;
		OUTPUT_PRIVATE(r)->do_bg = FALSE;
	}
	
	if(r->font_point != r->current_font_point) {
		sprintf(buf_font, " size=\"%d\" ", convert_font_point(r->current_font_point));
		did_fp = 1;
	} else
		buf_font[0] = '\0';

	if(OUTPUT_PRIVATE(r)->current_fg_color.r >= 0 && OUTPUT_PRIVATE(r)->current_fg_color.g >= 0 
	&& OUTPUT_PRIVATE(r)->current_fg_color.b >= 0 && !(OUTPUT_PRIVATE(r)->current_fg_color.r == 0 
	&& OUTPUT_PRIVATE(r)->current_fg_color.g == 0 && OUTPUT_PRIVATE(r)->current_fg_color.b == 0)) {
		char buf[MAXSTRLEN];
		char color[40];
		get_html_color(color, &OUTPUT_PRIVATE(r)->current_fg_color);
		sprintf(buf, "<font %s color=\"%s\">", buf_font, color);
		print_text(r, buf, backwards);
		did_fg = 1;
	}

	if(!did_fg && did_fp) { 
		char buf[MAXSTRLEN];
		sprintf(buf, "<font %s>", buf_font);
		print_text(r, buf, backwards);
	}

	print_text(r, text, backwards);
	
	if(did_fg || did_fp) {
		OUTPUT(r)->rlib_set_fg_color(r, 0, 0, 0);
		print_text(r, "</font>", backwards);
	}
}


static void rlib_html_set_fg_color(rlib *r, float red, float green, float blue) {
	if(OUTPUT_PRIVATE(r)->current_fg_color.r != red || OUTPUT_PRIVATE(r)->current_fg_color.g != green 
	|| OUTPUT_PRIVATE(r)->current_fg_color.b != blue) {
		OUTPUT_PRIVATE(r)->current_fg_color.r = red;
		OUTPUT_PRIVATE(r)->current_fg_color.g = green;
		OUTPUT_PRIVATE(r)->current_fg_color.b = blue;	
	}
}

static void rlib_html_set_bg_color(rlib *r, float red, float green, float blue) {
	if(OUTPUT_PRIVATE(r)->current_bg_color.r != red || OUTPUT_PRIVATE(r)->current_bg_color.g != green 
	|| OUTPUT_PRIVATE(r)->current_bg_color.b != blue) {
		OUTPUT_PRIVATE(r)->current_bg_color.r = red;
		OUTPUT_PRIVATE(r)->current_bg_color.g = green;
		OUTPUT_PRIVATE(r)->current_bg_color.b = blue;	
	}
}

static void rlib_html_hr(rlib *r, int backwards, float left_origin, float bottom_origin, float how_long, float how_tall, 
struct rgb *color, float indent, float length) {
	char buf[MAXSTRLEN];
	char color_str[40];
	get_html_color(color_str, color);
	
	sprintf(buf,"<td height=\"%d\" bgcolor=\"%s\"></td>", (int)how_tall, color_str);
	
	print_text(r, "<table><tr>", backwards);
	print_text(r, buf, backwards);
	print_text(r, "</tr></table>", backwards);
}

static void rlib_html_draw_cell_background_start(rlib *r, float left_origin, float bottom_origin, float how_long, float how_tall, struct rgb *color) {
	OUTPUT(r)->rlib_set_bg_color(r, color->r, color->g, color->b);
	OUTPUT_PRIVATE(r)->do_bg = TRUE;
}

static void rlib_html_draw_cell_background_end(rlib *r) {
	if(OUTPUT_PRIVATE(r)->did_bg) {
		OUTPUT(r)->rlib_set_bg_color(r, 0, 0, 0);
		print_text(r, "</span>", OUTPUT_PRIVATE(r)->bg_backwards);
		OUTPUT_PRIVATE(r)->do_bg = FALSE;
	}
	
}


static void rlib_html_boxurl_start(rlib *r, float left_origin, float bottom_origin, float how_long, float how_tall, char *url) {
	char buf[MAXSTRLEN];
	sprintf(buf, "<a href=\"%s\">", url);
	print_text(r, buf, FALSE);
}

static void rlib_html_boxurl_end(rlib *r) {
	print_text(r, "</a>", FALSE);
}

static void rlib_html_drawimage(rlib *r, float left_origin, float bottom_origin, char *nname, char *type, float nwidth, 
	float nheight) {
	char buf[MAXSTRLEN];

	print_text(r, "<DIV>", FALSE);	
	sprintf(buf, "<img src=\"%s\">", nname);
	print_text(r, buf, FALSE);
	print_text(r, "</DIV>", FALSE);
}

static void rlib_html_set_font_point(rlib *r, int point) {
	if(r->current_font_point != point) {
		r->current_font_point = point;
	}
}

static void rlib_html_start_new_page(rlib *r) {
	r->position_bottom = 11-GET_MARGIN(r)->bottom_margin;
}

static void rlib_html_init_end_page(rlib *r) {}

static void rlib_html_end_text(rlib *r) {
	print_text(r, "</pre></td></tr></table>", FALSE);
}

static void rlib_html_init_output(rlib *r) {
	char buf[MAXSTRLEN];
	print_text(r, "<head>", FALSE);	
}

static void rlib_html_init_output_report(rlib *r) {
	char buf[MAXSTRLEN];
	print_text(r, "<head><style type=\"text/css\">", FALSE);
	sprintf(buf, "pre { margin:0; padding:0; margin-top:0; margin-bottom:0; font-size: %dpt;}\n", r->font_point);
	print_text(r, buf, FALSE);
	print_text(r, "DIV { position: absolute; left: 0; }\n", FALSE);
	print_text(r, "TABLE { border: 0; cellspacing: 0; cellpadding: 0; width: 100%; }\n", FALSE);
	print_text(r, "</style></head>\n", FALSE);
	print_text(r, "<body><table><tr><td><pre>", FALSE);
	
}
static void rlib_html_begin_text(rlib *r) {}

static void rlib_html_finalize_private(rlib *r) {
	r->length = OUTPUT_PRIVATE(r)->top_size + OUTPUT_PRIVATE(r)->bottom_size;
}

static void rlib_html_spool_private(rlib *r) {
	rlib_write_output(OUTPUT_PRIVATE(r)->top, strlen(OUTPUT_PRIVATE(r)->top));
	rlib_write_output(OUTPUT_PRIVATE(r)->bottom, strlen(OUTPUT_PRIVATE(r)->bottom));
}

static void rlib_html_start_line(rlib *r, int backwards) {
	OUTPUT_PRIVATE(r)->did_bg = FALSE;
}

static void rlib_html_end_line(rlib *r, int backwards) {
	print_text(r, "\n", backwards);
}

static void rlib_html_start_output_section(rlib *r) {}
static void rlib_html_end_output_section(rlib *r) {}

static void rlib_html_end_page(rlib *r) {
	r->current_page_number++;
	r->current_line_number = 1;
	rlib_init_page(r, FALSE);
}

static int rlib_html_is_single_page(rlib *r) {
	return TRUE;
}

void rlib_html_new_output_filter(rlib *r) {
	OUTPUT(r) = rmalloc(sizeof(struct output_filter));
	OUTPUT_PRIVATE(r) = rmalloc(sizeof(struct _private));
	bzero(OUTPUT_PRIVATE(r), sizeof(struct _private));
	OUTPUT_PRIVATE(r)->top = NULL;
	OUTPUT_PRIVATE(r)->top_size = 0;
	OUTPUT_PRIVATE(r)->top_total_size = 0;
	OUTPUT_PRIVATE(r)->bottom = NULL;
	OUTPUT_PRIVATE(r)->bottom_size = 0;
	OUTPUT_PRIVATE(r)->bottom_total_size = 0;
	OUTPUT_PRIVATE(r)->do_bg = FALSE;
	
	OUTPUT(r)->do_align = TRUE;
	OUTPUT(r)->do_break = TRUE;
	
	OUTPUT(r)->rlib_get_string_width = rlib_html_get_string_width;
	OUTPUT(r)->rlib_print_text = rlib_html_print_text;
	OUTPUT(r)->rlib_set_fg_color = rlib_html_set_fg_color;
	OUTPUT(r)->rlib_set_bg_color = rlib_html_set_bg_color;
	OUTPUT(r)->rlib_hr = rlib_html_hr;
	OUTPUT(r)->rlib_draw_cell_background_start = rlib_html_draw_cell_background_start;
	OUTPUT(r)->rlib_draw_cell_background_end = rlib_html_draw_cell_background_end;
	OUTPUT(r)->rlib_boxurl_start = rlib_html_boxurl_start;
	OUTPUT(r)->rlib_boxurl_end = rlib_html_boxurl_end;
	OUTPUT(r)->rlib_drawimage = rlib_html_drawimage;
	OUTPUT(r)->rlib_set_font_point = rlib_html_set_font_point;
	OUTPUT(r)->rlib_start_new_page = rlib_html_start_new_page;
	OUTPUT(r)->rlib_end_page = rlib_html_end_page;	
	OUTPUT(r)->rlib_init_end_page = rlib_html_init_end_page;
	OUTPUT(r)->rlib_end_text = rlib_html_end_text;
	OUTPUT(r)->rlib_init_output = rlib_html_init_output;
	OUTPUT(r)->rlib_init_output_report = rlib_html_init_output_report;
	OUTPUT(r)->rlib_begin_text = rlib_html_begin_text;
	OUTPUT(r)->rlib_finalize_private = rlib_html_finalize_private;
	OUTPUT(r)->rlib_spool_private = rlib_html_spool_private;
	OUTPUT(r)->rlib_start_line = rlib_html_start_line;
	OUTPUT(r)->rlib_end_line = rlib_html_end_line;
	OUTPUT(r)->rlib_is_single_page = rlib_html_is_single_page;
	OUTPUT(r)->rlib_start_output_section = rlib_html_start_output_section;	
	OUTPUT(r)->rlib_end_output_section = rlib_html_end_output_section;	
}
