/*
 *  Copyright (C) 2003-2004 SICOM Systems, INC.
 *
 *  Authors:	Bob Doan <bdoan@sicompos.com>
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
 * This module implements the HTML output renderer for generating an HTML
 * formatted report from the rlib object.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "rlib.h"

struct _data {
	gchar *data;
	gint size;
	gint total_size;
};

struct _private {
	struct rlib_rgb current_fg_color;
	struct rlib_rgb current_bg_color;
	struct _data *top;
	struct _data *bottom;
	gchar *both;
	gint did_bg;
	gint bg_backwards;
	gint do_bg;
	gint length;
	gint page_number;
	gboolean is_bold;
	gboolean is_italics;
};

static void print_text(rlib *r, gchar *text, gint backwards) {
	gchar *str_ptr;
	gint text_size = strlen(text);
	gint *size = NULL;

	if(backwards) {
		make_more_space_if_necessary(&OUTPUT_PRIVATE(r)->bottom[OUTPUT_PRIVATE(r)->page_number].data, 
			&OUTPUT_PRIVATE(r)->bottom[OUTPUT_PRIVATE(r)->page_number].size, 
			&OUTPUT_PRIVATE(r)->bottom[OUTPUT_PRIVATE(r)->page_number].total_size, text_size);
		str_ptr = OUTPUT_PRIVATE(r)->bottom[OUTPUT_PRIVATE(r)->page_number].data;
		size = &OUTPUT_PRIVATE(r)->bottom[OUTPUT_PRIVATE(r)->page_number].size;
	} else {
		make_more_space_if_necessary(&OUTPUT_PRIVATE(r)->top[OUTPUT_PRIVATE(r)->page_number].data, 
			&OUTPUT_PRIVATE(r)->top[OUTPUT_PRIVATE(r)->page_number].size, 
			&OUTPUT_PRIVATE(r)->top[OUTPUT_PRIVATE(r)->page_number].total_size, text_size);
		str_ptr = OUTPUT_PRIVATE(r)->top[OUTPUT_PRIVATE(r)->page_number].data;	
		size = &OUTPUT_PRIVATE(r)->top[OUTPUT_PRIVATE(r)->page_number].size;
	}
	memcpy(str_ptr + (*size), text, text_size+1);
	*size = (*size) + text_size;
}

static gfloat rlib_html_get_string_width(rlib *r, gchar *text) {
	return 1;
}

static gchar *get_html_color(gchar *str, struct rlib_rgb *color) {
	sprintf(str, "#%02x%02x%02x", (gint)(color->r*0xFF), 
		(gint)(color->g*0xFF), (gint)(color->b*0xFF));
	return str;
}

static gint convert_font_point(gint point) {
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

static void rlib_html_print_text(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *text, gint backwards, gint col) {
	gint did_fg = 0;
	gint did_fp = 0;
	gchar buf_font[MAXSTRLEN];

	OUTPUT_PRIVATE(r)->bg_backwards = backwards;
	

	if(OUTPUT_PRIVATE(r)->current_bg_color.r >= 0 && OUTPUT_PRIVATE(r)->current_bg_color.g >= 0 
	&& OUTPUT_PRIVATE(r)->current_bg_color.b >= 0 && !((OUTPUT_PRIVATE(r)->current_bg_color.r == 1.0 
	&& OUTPUT_PRIVATE(r)->current_bg_color.g == 1.0 && OUTPUT_PRIVATE(r)->current_bg_color.b == 1.0)) &&
	OUTPUT_PRIVATE(r)->do_bg) {
		gchar buf[MAXSTRLEN];
		gchar color[40];
		get_html_color(color, &OUTPUT_PRIVATE(r)->current_bg_color);
		sprintf(buf, "<span style=\"background-color: %s\">", color);
		print_text(r, buf, backwards);
		OUTPUT_PRIVATE(r)->did_bg = TRUE;
		OUTPUT_PRIVATE(r)->do_bg = FALSE;
	}
	
	if(r->font_point != r->current_font_point) {
		sprintf(buf_font, "size=\"%d\"", convert_font_point(r->current_font_point));
		did_fp = 1;
	} else
		buf_font[0] = '\0';

	if(OUTPUT_PRIVATE(r)->current_fg_color.r >= 0 && OUTPUT_PRIVATE(r)->current_fg_color.g >= 0 
	&& OUTPUT_PRIVATE(r)->current_fg_color.b >= 0 && !(OUTPUT_PRIVATE(r)->current_fg_color.r == 0 
	&& OUTPUT_PRIVATE(r)->current_fg_color.g == 0 && OUTPUT_PRIVATE(r)->current_fg_color.b == 0)) {
		gchar buf[MAXSTRLEN];
		gchar color[40];
		get_html_color(color, &OUTPUT_PRIVATE(r)->current_fg_color);
		sprintf(buf, "<font %s color=\"%s\">", buf_font, color);
		print_text(r, buf, backwards);
		did_fg = 1;
	}

	if(!did_fg && did_fp) { 
		gchar buf[MAXSTRLEN];
		sprintf(buf, "<font %s>", buf_font);
		print_text(r, buf, backwards);
	}

	if(OUTPUT_PRIVATE(r)->is_bold == TRUE)
		print_text(r, "<b>", backwards);
	if(OUTPUT_PRIVATE(r)->is_italics == TRUE)
		print_text(r, "<i>", backwards);

	print_text(r, text, backwards);
	
	if(OUTPUT_PRIVATE(r)->is_italics == TRUE)
		print_text(r, "</i>", backwards);
	if(OUTPUT_PRIVATE(r)->is_bold == TRUE)
		print_text(r, "</b>", backwards);

	if(did_fg || did_fp) {
		OUTPUT(r)->set_fg_color(r, 0, 0, 0);
		print_text(r, "</font>", backwards);
	}
}


static void rlib_html_set_fg_color(rlib *r, gfloat red, gfloat green, gfloat blue) {
	if(OUTPUT_PRIVATE(r)->current_fg_color.r != red || OUTPUT_PRIVATE(r)->current_fg_color.g != green 
	|| OUTPUT_PRIVATE(r)->current_fg_color.b != blue) {
		OUTPUT_PRIVATE(r)->current_fg_color.r = red;
		OUTPUT_PRIVATE(r)->current_fg_color.g = green;
		OUTPUT_PRIVATE(r)->current_fg_color.b = blue;	
	}
}

static void rlib_html_set_bg_color(rlib *r, gfloat red, gfloat green, gfloat blue) {
	if(OUTPUT_PRIVATE(r)->current_bg_color.r != red || OUTPUT_PRIVATE(r)->current_bg_color.g != green 
	|| OUTPUT_PRIVATE(r)->current_bg_color.b != blue) {
		OUTPUT_PRIVATE(r)->current_bg_color.r = red;
		OUTPUT_PRIVATE(r)->current_bg_color.g = green;
		OUTPUT_PRIVATE(r)->current_bg_color.b = blue;	
	}
}

static void rlib_html_hr(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, 
struct rlib_rgb *color, gfloat indent, gfloat length) {
	gchar buf[MAXSTRLEN];
	gchar color_str[40];
	get_html_color(color_str, color);
	
	if( how_tall > 0 ) { 
		sprintf(buf,"<td height=\"%d\" bgcolor=\"%s\"></td>", (int)how_tall, color_str);
		print_text(r, "<table><tr>", backwards);
		print_text(r, buf, backwards);
		print_text(r, "</tr></table>", backwards);
	}
}

static void rlib_html_start_draw_cell_background(rlib *r, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, 
struct rlib_rgb *color) {
	OUTPUT(r)->set_bg_color(r, color->r, color->g, color->b);
	OUTPUT_PRIVATE(r)->do_bg = TRUE;
}

static void rlib_html_end_draw_cell_background(rlib *r) {
	if(OUTPUT_PRIVATE(r)->did_bg) {
		OUTPUT(r)->set_bg_color(r, 0, 0, 0);
		print_text(r, "</span>", OUTPUT_PRIVATE(r)->bg_backwards);
		OUTPUT_PRIVATE(r)->do_bg = FALSE;
	}
	
}


static void rlib_html_start_boxurl(rlib *r, struct rlib_part *part, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, gchar *url) {
	gchar buf[MAXSTRLEN];
	sprintf(buf, "<a href=\"%s\">", url);
	print_text(r, buf, FALSE);
}

static void rlib_html_end_boxurl(rlib *r) {
	print_text(r, "</a>", FALSE);
}

static void rlib_html_drawimage(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *nname, gchar *type, gfloat nwidth, 
gfloat nheight) {
	gchar buf[MAXSTRLEN];

	print_text(r, "<DIV>", FALSE);	
	sprintf(buf, "<img src=\"%s\">", nname);
	print_text(r, buf, FALSE);
	print_text(r, "</DIV>", FALSE);
}

static void rlib_html_set_font_point(rlib *r, gint point) {
	if(r->current_font_point != point) {
		r->current_font_point = point;
	}
}

static void rlib_html_start_new_page(rlib *r, struct rlib_part *part) {
	part->position_bottom[0] = 11-part->bottom_margin;
}


static void rlib_html_start_report(rlib *r, struct rlib_part *part) {
	gchar buf[MAXSTRLEN];
	gint pages_across = part->pages_across;
	gint i;

	OUTPUT_PRIVATE(r)->bottom = g_malloc(sizeof(struct _data) * pages_across);
	OUTPUT_PRIVATE(r)->top = g_malloc(sizeof(struct _data) * pages_across);
	for(i=0;i<pages_across;i++) {
		OUTPUT_PRIVATE(r)->top[i].data = NULL;
		OUTPUT_PRIVATE(r)->top[i].size = 0;
		OUTPUT_PRIVATE(r)->top[i].total_size = 0;
		OUTPUT_PRIVATE(r)->bottom[i].data = NULL;
		OUTPUT_PRIVATE(r)->bottom[i].size = 0;
		OUTPUT_PRIVATE(r)->bottom[i].total_size = 0;
	}
//	<meta http-equiv=\"content-type\" content=\"text/html; charset=%s\"><style type=\"text/css\">\n", output_encoding);
	sprintf(buf, "<head>\n<style type=\"text/css\">\n");
	print_text(r, buf, FALSE);
	sprintf(buf, "pre { margin:0; padding:0; margin-top:0; margin-bottom:0;}\n");
	print_text(r, buf, FALSE);
	print_text(r, "DIV { position: absolute; left: 0; }\n", FALSE);
	print_text(r, "TABLE { border: 0; cellspacing: 0; cellpadding: 0; width: 100%; }\n", FALSE);
	print_text(r, "</style></head>\n", FALSE);
	print_text(r, "<body><table><tr><td><pre>", FALSE);
	
}

static void rlib_html_end_report(rlib *r, struct rlib_part *part) {
	gint i;
	gint pages_across = part->pages_across;
	gint sofar = OUTPUT_PRIVATE(r)->length;
	gchar *str1 = "<table><tr>";
	gchar *str2 = "<td nowrap><pre>";
	gchar *str3 = "</td>";
	gchar *str4 = "</tr></table>";
	print_text(r, "</pre></td></tr></table>", TRUE);

	OUTPUT_PRIVATE(r)->both = g_realloc(OUTPUT_PRIVATE(r)->both, sofar + 11);
	memcpy(OUTPUT_PRIVATE(r)->both + sofar , str1, 11);
	sofar += 11;
	for(i=0;i<pages_across;i++) {
		OUTPUT_PRIVATE(r)->both = g_realloc(OUTPUT_PRIVATE(r)->both, sofar + 16);
		memcpy(OUTPUT_PRIVATE(r)->both + sofar, str2, 16);
		sofar += 16;

		OUTPUT_PRIVATE(r)->both = g_realloc(OUTPUT_PRIVATE(r)->both, sofar + OUTPUT_PRIVATE(r)->top[i].size + OUTPUT_PRIVATE(r)->bottom[i].size);
		memcpy(OUTPUT_PRIVATE(r)->both + sofar , OUTPUT_PRIVATE(r)->top[i].data, OUTPUT_PRIVATE(r)->top[i].size);
		memcpy(OUTPUT_PRIVATE(r)->both + sofar + OUTPUT_PRIVATE(r)->top[i].size, OUTPUT_PRIVATE(r)->bottom[i].data, OUTPUT_PRIVATE(r)->bottom[i].size);
		sofar += OUTPUT_PRIVATE(r)->top[i].size + OUTPUT_PRIVATE(r)->bottom[i].size;	
		OUTPUT_PRIVATE(r)->both = g_realloc(OUTPUT_PRIVATE(r)->both, sofar + 5);
		memcpy(OUTPUT_PRIVATE(r)->both + sofar, str3, 5);
		sofar += 5;
	}
	OUTPUT_PRIVATE(r)->both = g_realloc(OUTPUT_PRIVATE(r)->both, sofar + 13);
	memcpy(OUTPUT_PRIVATE(r)->both + sofar, str4, 13);
	sofar += 13;
	OUTPUT_PRIVATE(r)->length = sofar;

	for(i=0;i<pages_across;i++) {
		g_free(OUTPUT_PRIVATE(r)->top[i].data);
		g_free(OUTPUT_PRIVATE(r)->bottom[i].data);
	}

	g_free(OUTPUT_PRIVATE(r)->top);
	g_free(OUTPUT_PRIVATE(r)->bottom);
}

static void rlib_html_spool_private(rlib *r) {
	ENVIRONMENT(r)->rlib_write_output(OUTPUT_PRIVATE(r)->both, OUTPUT_PRIVATE(r)->length);
}

static void rlib_html_start_line(rlib *r, int backwards) {
	OUTPUT_PRIVATE(r)->did_bg = FALSE;
}

static void rlib_html_end_line(rlib *r, int backwards) {
	print_text(r, "\n", backwards);
}

static void rlib_html_end_page(rlib *r, struct rlib_part *part) {
	r->current_line_number = 1;
}

static char *rlib_html_get_output(rlib *r) {
	return OUTPUT_PRIVATE(r)->both;
}

static long rlib_html_get_output_length(rlib *r) {
	return OUTPUT_PRIVATE(r)->length;
}

static void rlib_html_set_working_page(rlib *r, struct rlib_part *part, gint page) {
	OUTPUT_PRIVATE(r)->page_number = page-1;
}

static void rlib_html_start_tr(rlib *r) {
	print_text(r, "<table><tr> <!-- REAL TR -->", FALSE);
}

static void rlib_html_end_tr(rlib *r) {
	print_text(r, "</tr></table>", FALSE);
}

static void rlib_html_start_td(rlib *r, struct rlib_part *part, gfloat left_margin, gfloat top_margin, int width, int height, int border_width, struct rlib_rgb *color) {
	char buf[150];
	char border_color[150];
	
	if(color != NULL)
		get_html_color(border_color, color);
	else
		border_color[0] = 0;
	
	if(border_width > 0) {
		sprintf(buf, "<td width=\"%d%%\" valign=\"top\" style=\"border:solid %dpx %s\"><pre>", width, border_width, border_color);
	}
	else
		sprintf(buf, "<td width=\"%d%%\" valign=\"top\"><pre>", width);
	print_text(r, buf, FALSE);
}

static void rlib_html_end_td(rlib *r) {
	print_text(r, "</td>", FALSE);
}


static void rlib_html_start_bold(rlib *r) {
	OUTPUT_PRIVATE(r)->is_bold = TRUE;
}

static void rlib_html_end_bold(rlib *r) {
	OUTPUT_PRIVATE(r)->is_bold = FALSE;
}

static void rlib_html_start_italics(rlib *r) {
	OUTPUT_PRIVATE(r)->is_italics = TRUE;
}

static void rlib_html_end_italics(rlib *r) {
	OUTPUT_PRIVATE(r)->is_italics = FALSE;
}


static void rlib_html_init_end_page(rlib *r) {}
static void rlib_html_init_output(rlib *r) {}
static void rlib_html_finalize_private(rlib *r) {}
static void rlib_html_start_output_section(rlib *r) {}
static void rlib_html_end_output_section(rlib *r) {}
static void rlib_html_set_raw_page(rlib *r, struct rlib_part *part, gint page) {}

static gint rlib_html_free(rlib *r) {
	g_free(OUTPUT_PRIVATE(r)->both);
	g_free(OUTPUT_PRIVATE(r));
	g_free(OUTPUT(r));
	return 0;
}

void rlib_html_new_output_filter(rlib *r) {
	OUTPUT(r) = g_malloc(sizeof(struct output_filter));
	r->o->private = g_malloc(sizeof(struct _private));
	memset(OUTPUT_PRIVATE(r), 0, sizeof(struct _private));

	OUTPUT_PRIVATE(r)->do_bg = FALSE;
	OUTPUT_PRIVATE(r)->page_number = 0;
	OUTPUT_PRIVATE(r)->both = NULL;
	OUTPUT_PRIVATE(r)->length = 0;
	OUTPUT(r)->do_align = TRUE;
	OUTPUT(r)->do_break = TRUE;
	OUTPUT(r)->do_grouptext = FALSE;	
	OUTPUT(r)->paginate = FALSE;
	
	OUTPUT(r)->get_string_width = rlib_html_get_string_width;
	OUTPUT(r)->print_text = rlib_html_print_text;
	OUTPUT(r)->set_fg_color = rlib_html_set_fg_color;
	OUTPUT(r)->set_bg_color = rlib_html_set_bg_color;
	OUTPUT(r)->hr = rlib_html_hr;
	OUTPUT(r)->start_draw_cell_background = rlib_html_start_draw_cell_background;
	OUTPUT(r)->end_draw_cell_background = rlib_html_end_draw_cell_background;
	OUTPUT(r)->start_boxurl = rlib_html_start_boxurl;
	OUTPUT(r)->end_boxurl = rlib_html_end_boxurl;
	OUTPUT(r)->drawimage = rlib_html_drawimage;
	OUTPUT(r)->set_font_point = rlib_html_set_font_point;
	OUTPUT(r)->start_new_page = rlib_html_start_new_page;
	OUTPUT(r)->end_page = rlib_html_end_page;  
	OUTPUT(r)->init_end_page = rlib_html_init_end_page;
	OUTPUT(r)->init_output = rlib_html_init_output;
	OUTPUT(r)->start_report = rlib_html_start_report;
	OUTPUT(r)->end_report = rlib_html_end_report;
	OUTPUT(r)->finalize_private = rlib_html_finalize_private;
	OUTPUT(r)->spool_private = rlib_html_spool_private;
	OUTPUT(r)->start_line = rlib_html_start_line;
	OUTPUT(r)->end_line = rlib_html_end_line;
	OUTPUT(r)->start_output_section = rlib_html_start_output_section;  
	OUTPUT(r)->end_output_section = rlib_html_end_output_section;   
	OUTPUT(r)->get_output = rlib_html_get_output;
	OUTPUT(r)->get_output_length = rlib_html_get_output_length;
	OUTPUT(r)->set_working_page = rlib_html_set_working_page; 
	OUTPUT(r)->set_raw_page = rlib_html_set_raw_page; 
	OUTPUT(r)->start_tr = rlib_html_start_tr; 
	OUTPUT(r)->end_tr = rlib_html_end_tr; 
	OUTPUT(r)->start_td = rlib_html_start_td; 
	OUTPUT(r)->end_td = rlib_html_end_td; 
	OUTPUT(r)->start_bold = rlib_html_start_bold;
	OUTPUT(r)->end_bold = rlib_html_end_bold;
	OUTPUT(r)->start_italics = rlib_html_start_italics;
	OUTPUT(r)->end_italics = rlib_html_end_italics;
	OUTPUT(r)->free = rlib_html_free;
}
