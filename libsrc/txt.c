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
	struct _data *top;
	struct _data *bottom;
	gchar *both;
	gint length;
	gint page_number;
};

static void print_text(rlib *r, gchar *text, gint backwards) {
	gchar *str_ptr;
	gint text_size = strlen(text);
	gint *size = NULL;
	if(backwards) {
		make_more_space_if_necessary(&OUTPUT_PRIVATE(r)->bottom[OUTPUT_PRIVATE(r)->page_number].data, 
			&OUTPUT_PRIVATE(r)->bottom[OUTPUT_PRIVATE(r)->page_number].size, 
			&OUTPUT_PRIVATE(r)->bottom[OUTPUT_PRIVATE(r)->page_number].total_size, text_size+1);
		str_ptr = OUTPUT_PRIVATE(r)->bottom[OUTPUT_PRIVATE(r)->page_number].data;
		size = &OUTPUT_PRIVATE(r)->bottom[OUTPUT_PRIVATE(r)->page_number].size;
	} else {
		make_more_space_if_necessary(&OUTPUT_PRIVATE(r)->top[OUTPUT_PRIVATE(r)->page_number].data, 
			&OUTPUT_PRIVATE(r)->top[OUTPUT_PRIVATE(r)->page_number].size, 
			&OUTPUT_PRIVATE(r)->top[OUTPUT_PRIVATE(r)->page_number].total_size, text_size+1);
		str_ptr = OUTPUT_PRIVATE(r)->top[OUTPUT_PRIVATE(r)->page_number].data;	
		size = &OUTPUT_PRIVATE(r)->top[OUTPUT_PRIVATE(r)->page_number].size;
	}
	memcpy(str_ptr + (*size), text, text_size+1);
	*size = (*size) + text_size;
}

static gfloat rlib_txt_get_string_width(rlib *r, gchar *text) {
	return 1;
}

static void rlib_txt_print_text(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *text, gint backwards, gint col) {
	print_text(r, text, backwards);
}

static void rlib_txt_start_new_page(rlib *r, struct rlib_part *part) {
	part->position_bottom[0] = 11-part->bottom_margin;
}

static void rlib_txt_init_end_page(rlib *r) {}
static void rlib_txt_init_output(rlib *r) {}
static void rlib_txt_finalize_private(rlib *r) {}

static void rlib_txt_spool_private(rlib *r) {
	ENVIRONMENT(r)->rlib_write_output(OUTPUT_PRIVATE(r)->both, OUTPUT_PRIVATE(r)->length);
}

static void rlib_txt_end_line(rlib *r, int backwards) {
	print_text(r, "\n", backwards);
}

static void rlib_txt_start_report(rlib *r, struct rlib_part *part) {
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
}

static void rlib_txt_end_part(rlib *r, struct rlib_part *part) {
	gint i;
	gint pages_across = part->pages_across;
	gint sofar = OUTPUT_PRIVATE(r)->length;

	
	for(i=0;i<pages_across;i++) {
		OUTPUT_PRIVATE(r)->both = g_realloc(OUTPUT_PRIVATE(r)->both, sofar + OUTPUT_PRIVATE(r)->top[i].size + OUTPUT_PRIVATE(r)->bottom[i].size);
		memcpy(OUTPUT_PRIVATE(r)->both + sofar , OUTPUT_PRIVATE(r)->top[i].data, OUTPUT_PRIVATE(r)->top[i].size);
		memcpy(OUTPUT_PRIVATE(r)->both + sofar + OUTPUT_PRIVATE(r)->top[i].size, OUTPUT_PRIVATE(r)->bottom[i].data, OUTPUT_PRIVATE(r)->bottom[i].size);
		sofar += OUTPUT_PRIVATE(r)->top[i].size + OUTPUT_PRIVATE(r)->bottom[i].size;	
	}
	OUTPUT_PRIVATE(r)->length += sofar;

	for(i=0;i<pages_across;i++) {
		g_free(OUTPUT_PRIVATE(r)->top[i].data);
		g_free(OUTPUT_PRIVATE(r)->bottom[i].data);
	}
	g_free(OUTPUT_PRIVATE(r)->top);
	g_free(OUTPUT_PRIVATE(r)->bottom);

}

static void rlib_txt_end_page(rlib *r, struct rlib_part *part) {
	r->current_page_number++;
	r->current_line_number = 1;
}

static int rlib_txt_free(rlib *r) {
	g_free(OUTPUT_PRIVATE(r)->both);
	g_free(OUTPUT_PRIVATE(r));
	g_free(OUTPUT(r));
	return 0;
}

static char *rlib_txt_get_output(rlib *r) {
	return OUTPUT_PRIVATE(r)->both;
}

static long rlib_txt_get_output_length(rlib *r) {
	return OUTPUT_PRIVATE(r)->length;
}

static void rlib_txt_set_working_page(rlib *r, struct rlib_part *part, int page) {
	OUTPUT_PRIVATE(r)->page_number = page-1;
}

static void rlib_txt_set_fg_color(rlib *r, gfloat red, gfloat green, gfloat blue) {}
static void rlib_txt_set_bg_color(rlib *r, gfloat red, gfloat green, gfloat blue) {}
static void rlib_txt_hr(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, 
struct rlib_rgb *color, gfloat indent, gfloat length) {}
static void rlib_txt_start_draw_cell_background(rlib *r, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, 
struct rlib_rgb *color) {}
static void rlib_txt_end_draw_cell_background(rlib *r) {}
static void rlib_txt_start_boxurl(rlib *r, struct rlib_part * part, gfloat left_origin, gfloat bottom_origin, gfloat how_long, gfloat how_tall, gchar *url) {}
static void rlib_txt_end_boxurl(rlib *r) {}
static void rlib_txt_drawimage(rlib *r, gfloat left_origin, gfloat bottom_origin, gchar *nname, gchar *type, gfloat nwidth, gfloat nheight) {}
static void rlib_txt_set_font_point(rlib *r, gint point) {}
static void rlib_txt_start_line(rlib *r, gint backwards) {}
static void rlib_txt_start_output_section(rlib *r) {}
static void rlib_txt_end_output_section(rlib *r) {}
static void rlib_txt_start_tr(rlib *r) {}
static void rlib_txt_end_report(rlib *r, struct rlib_report *report) {}
static void rlib_txt_end_tr(rlib *r) {}
static void rlib_txt_start_td(rlib *r, struct rlib_part *part, gfloat left_margin, gfloat top_margin, int width, int height, int border_width, struct rlib_rgb *color) {}
static void rlib_txt_end_td(rlib *r) {}
static void rlib_txt_set_raw_page(rlib *r, struct rlib_part *part, gint page) {}
static void rlib_txt_start_bold(rlib *r) {}
static void rlib_txt_end_bold(rlib *r) {}
static void rlib_txt_start_italics(rlib *r) {}
static void rlib_txt_end_italics(rlib *r) {}

void rlib_txt_new_output_filter(rlib *r) {
	OUTPUT(r) = g_malloc(sizeof(struct output_filter));
	r->o->private = g_malloc(sizeof(struct _private));
	memset(OUTPUT_PRIVATE(r), 0, sizeof(struct _private));
	
	OUTPUT(r)->do_align = TRUE;
	OUTPUT(r)->do_break = TRUE;
	OUTPUT(r)->do_grouptext = FALSE;	
	OUTPUT(r)->paginate = FALSE;
	
	OUTPUT(r)->get_string_width = rlib_txt_get_string_width;
	OUTPUT(r)->print_text = rlib_txt_print_text;
	OUTPUT(r)->set_fg_color = rlib_txt_set_fg_color;
	OUTPUT(r)->set_bg_color = rlib_txt_set_bg_color;
	OUTPUT(r)->hr = rlib_txt_hr;
	OUTPUT(r)->start_draw_cell_background = rlib_txt_start_draw_cell_background;
	OUTPUT(r)->end_draw_cell_background = rlib_txt_end_draw_cell_background;
	OUTPUT(r)->start_boxurl = rlib_txt_start_boxurl;
	OUTPUT(r)->end_boxurl = rlib_txt_end_boxurl;
	OUTPUT(r)->drawimage = rlib_txt_drawimage;
	OUTPUT(r)->set_font_point = rlib_txt_set_font_point;
	OUTPUT(r)->start_new_page = rlib_txt_start_new_page;
	OUTPUT(r)->end_page = rlib_txt_end_page;   
	OUTPUT(r)->init_end_page = rlib_txt_init_end_page;
	OUTPUT(r)->init_output = rlib_txt_init_output;
	OUTPUT(r)->start_report = rlib_txt_start_report;
	OUTPUT(r)->end_report = rlib_txt_end_report;
	OUTPUT(r)->end_part = rlib_txt_end_part;
	OUTPUT(r)->finalize_private = rlib_txt_finalize_private;
	OUTPUT(r)->spool_private = rlib_txt_spool_private;
	OUTPUT(r)->start_line = rlib_txt_start_line;
	OUTPUT(r)->end_line = rlib_txt_end_line;
	OUTPUT(r)->start_output_section = rlib_txt_start_output_section;   
	OUTPUT(r)->end_output_section = rlib_txt_end_output_section; 
	OUTPUT(r)->get_output = rlib_txt_get_output;
	OUTPUT(r)->get_output_length = rlib_txt_get_output_length;
	OUTPUT(r)->set_working_page = rlib_txt_set_working_page;  
	OUTPUT(r)->set_raw_page = rlib_txt_set_raw_page; 
	OUTPUT(r)->start_tr = rlib_txt_start_tr; 
	OUTPUT(r)->end_tr = rlib_txt_end_tr; 
	OUTPUT(r)->start_td = rlib_txt_start_td; 
	OUTPUT(r)->end_td = rlib_txt_end_td; 
	OUTPUT(r)->start_bold = rlib_txt_start_bold;
	OUTPUT(r)->end_bold = rlib_txt_end_bold;
	OUTPUT(r)->start_italics = rlib_txt_start_italics;
	OUTPUT(r)->end_italics = rlib_txt_end_italics;

	OUTPUT(r)->free = rlib_txt_free;  
}
