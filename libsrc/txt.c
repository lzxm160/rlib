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

struct _data {
	char *data;
	int size;
	int total_size;
};

struct _private {
	struct _data *top;
	struct _data *bottom;
	char *both;
	long length;
	int page_number;
};

static void print_text(rlib *r, char *text, int backwards) {
	char *str_ptr;
	int text_size = strlen(text);
	int *size = NULL;
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

static float rlib_txt_get_string_width(rlib *r, char *text) {
	return 1;
}

static void rlib_txt_print_text(rlib *r, float left_origin, float bottom_origin, char *text, int backwards, int col) {
	print_text(r, text, backwards);
}

static void rlib_txt_start_new_page(rlib *r) {
	r->reports[r->current_report]->position_bottom[0] = 11-GET_MARGIN(r)->bottom_margin;
}

static void rlib_txt_init_end_page(rlib *r) {}
static void rlib_txt_end_text(rlib *r) {}
static void rlib_txt_init_output(rlib *r) {}
static void rlib_txt_begin_text(rlib *r) {}
static void rlib_txt_finalize_private(rlib *r) {}

static void rlib_txt_spool_private(rlib *r) {
	ENVIRONMENT(r)->rlib_write_output(OUTPUT_PRIVATE(r)->both, OUTPUT_PRIVATE(r)->length);
}

static void rlib_txt_end_line(rlib *r, int backwards) {
	print_text(r, "\n", backwards);
}

static void rlib_txt_start_report(rlib *r) {
	int pages_accross = r->reports[r->current_report]->pages_accross;
	int i;

	OUTPUT_PRIVATE(r)->bottom = rmalloc(sizeof(struct _data) * pages_accross);
	OUTPUT_PRIVATE(r)->top = rmalloc(sizeof(struct _data) * pages_accross);
	for(i=0;i<pages_accross;i++) {
		OUTPUT_PRIVATE(r)->top[i].data = NULL;
		OUTPUT_PRIVATE(r)->top[i].size = 0;
		OUTPUT_PRIVATE(r)->top[i].total_size = 0;
		OUTPUT_PRIVATE(r)->bottom[i].data = NULL;
		OUTPUT_PRIVATE(r)->bottom[i].size = 0;
		OUTPUT_PRIVATE(r)->bottom[i].total_size = 0;
	}	
}

static void rlib_txt_end_report(rlib *r) {
	int i;
	int pages_accross = r->reports[r->current_report]->pages_accross;
	int sofar = OUTPUT_PRIVATE(r)->length;
	for(i=0;i<pages_accross;i++) {
		OUTPUT_PRIVATE(r)->both = rrealloc(OUTPUT_PRIVATE(r)->both, OUTPUT_PRIVATE(r)->length + OUTPUT_PRIVATE(r)->top[i].size + OUTPUT_PRIVATE(r)->bottom[i].size);
		memcpy(OUTPUT_PRIVATE(r)->both + sofar , OUTPUT_PRIVATE(r)->top[i].data, OUTPUT_PRIVATE(r)->top[i].size);
		memcpy(OUTPUT_PRIVATE(r)->both + sofar + OUTPUT_PRIVATE(r)->top[i].size, OUTPUT_PRIVATE(r)->bottom[i].data, OUTPUT_PRIVATE(r)->bottom[i].size);
		sofar += OUTPUT_PRIVATE(r)->top[i].size + OUTPUT_PRIVATE(r)->bottom[i].size;	
	}
	OUTPUT_PRIVATE(r)->length += sofar;

	for(i=0;i<pages_accross;i++) {
		rfree(OUTPUT_PRIVATE(r)->top[i].data);
		rfree(OUTPUT_PRIVATE(r)->bottom[i].data);
	}
	rfree(OUTPUT_PRIVATE(r)->top);
	rfree(OUTPUT_PRIVATE(r)->bottom);
}

static void rlib_txt_end_page(rlib *r) {
	r->current_page_number++;
	r->current_line_number = 1;
	rlib_init_page(r, FALSE);
}

static int rlib_txt_is_single_page(rlib *r) {
	return TRUE;
}

static int rlib_txt_free(rlib *r) {
	rfree(OUTPUT_PRIVATE(r)->both);
	rfree(OUTPUT_PRIVATE(r));
	rfree(OUTPUT(r));
	return 0;
}

static char *rlib_txt_get_output(rlib *r) {
	return OUTPUT_PRIVATE(r)->both;
}

static long rlib_txt_get_output_length(rlib *r) {
	return OUTPUT_PRIVATE(r)->length;
}

static void rlib_txt_set_working_page(rlib *r, int page) {
	OUTPUT_PRIVATE(r)->page_number = page-1;
}

static void rlib_txt_set_fg_color(rlib *r, float red, float green, float blue) {}
static void rlib_txt_set_bg_color(rlib *r, float red, float green, float blue) {}
static void rlib_txt_hr(rlib *r, int backwards, float left_origin, float bottom_origin, float how_long, float how_tall, struct rgb *color, float indent, float length) {}
static void rlib_txt_draw_cell_background_start(rlib *r, float left_origin, float bottom_origin, float how_long, float how_tall, struct rgb *color) {}
static void rlib_txt_draw_cell_background_end(rlib *r) {}
static void rlib_txt_boxurl_start(rlib *r, float left_origin, float bottom_origin, float how_long, float how_tall, char *url) {}
static void rlib_txt_boxurl_end(rlib *r) {}
static void rlib_txt_drawimage(rlib *r, float left_origin, float bottom_origin, char *nname, char *type, float nwidth, float nheight) {}
static void rlib_txt_set_font_point(rlib *r, int point) {}
static void rlib_txt_start_line(rlib *r, int backwards) {}
static void rlib_txt_start_output_section(rlib *r) {}
static void rlib_txt_end_output_section(rlib *r) {}

void rlib_txt_new_output_filter(rlib *r) {
	OUTPUT(r) = rmalloc(sizeof(struct output_filter));
	OUTPUT_PRIVATE(r) = rmalloc(sizeof(struct _private));
	bzero(OUTPUT_PRIVATE(r), sizeof(struct _private));
	
	OUTPUT(r)->do_align = TRUE;
	OUTPUT(r)->do_break = TRUE;
	OUTPUT(r)->do_grouptext = FALSE;	

	OUTPUT(r)->rlib_get_string_width = rlib_txt_get_string_width;
	OUTPUT(r)->rlib_print_text = rlib_txt_print_text;
	OUTPUT(r)->rlib_set_fg_color = rlib_txt_set_fg_color;
	OUTPUT(r)->rlib_set_bg_color = rlib_txt_set_bg_color;
	OUTPUT(r)->rlib_hr = rlib_txt_hr;
	OUTPUT(r)->rlib_draw_cell_background_start = rlib_txt_draw_cell_background_start;
	OUTPUT(r)->rlib_draw_cell_background_end = rlib_txt_draw_cell_background_end;
	OUTPUT(r)->rlib_boxurl_start = rlib_txt_boxurl_start;
	OUTPUT(r)->rlib_boxurl_end = rlib_txt_boxurl_end;
	OUTPUT(r)->rlib_drawimage = rlib_txt_drawimage;
	OUTPUT(r)->rlib_set_font_point = rlib_txt_set_font_point;
	OUTPUT(r)->rlib_start_new_page = rlib_txt_start_new_page;
	OUTPUT(r)->rlib_end_page = rlib_txt_end_page;	
	OUTPUT(r)->rlib_init_end_page = rlib_txt_init_end_page;
	OUTPUT(r)->rlib_end_text = rlib_txt_end_text;
	OUTPUT(r)->rlib_init_output = rlib_txt_init_output;
	OUTPUT(r)->rlib_start_report = rlib_txt_start_report;
	OUTPUT(r)->rlib_end_report = rlib_txt_end_report;
	OUTPUT(r)->rlib_begin_text = rlib_txt_begin_text;
	OUTPUT(r)->rlib_finalize_private = rlib_txt_finalize_private;
	OUTPUT(r)->rlib_spool_private = rlib_txt_spool_private;
	OUTPUT(r)->rlib_start_line = rlib_txt_start_line;
	OUTPUT(r)->rlib_end_line = rlib_txt_end_line;
	OUTPUT(r)->rlib_is_single_page = rlib_txt_is_single_page;
	OUTPUT(r)->rlib_start_output_section = rlib_txt_start_output_section;	
	OUTPUT(r)->rlib_end_output_section = rlib_txt_end_output_section;	
	OUTPUT(r)->rlib_get_output = rlib_txt_get_output;
	OUTPUT(r)->rlib_get_output_length = rlib_txt_get_output_length;
	OUTPUT(r)->rlib_set_working_page = rlib_txt_set_working_page;	
	OUTPUT(r)->rlib_free = rlib_txt_free;	
}
