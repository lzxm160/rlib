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
#include <mysql.h>

#include "ralloc.h"
#include "rlib.h"

#define MAX_COL	40

struct _private {
	char col[MAX_COL][MAXSTRLEN];
	char *top;
	int top_size;
	int top_total_size;
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

static void print_text(rlib *r, char *text, int backwards, int col) {
	if(col < MAX_COL) {
		if(OUTPUT_PRIVATE(r)->col[col][0] == 0)
			memcpy(OUTPUT_PRIVATE(r)->col[col], text, strlen(text)+1);
		else
			sprintf(OUTPUT_PRIVATE(r)->col[col], "%s %s", OUTPUT_PRIVATE(r)->col[col], text);
	}
}

static float rlib_csv_get_string_width(rlib *r, char *text) {
	return 1;
}

static void rlib_csv_print_text(rlib *r, float left_origin, float bottom_origin, char *text, int backwards, int col) {
	if(col) {
		print_text(r, text, backwards, col-1);
	}
}


static void rlib_csv_set_fg_color(rlib *r, float red, float green, float blue) {}
static void rlib_csv_set_bg_color(rlib *r, float red, float green, float blue) {}
static void rlib_csv_hr(rlib *r, int backwards, float left_origin, float bottom_origin, float how_long, float how_tall, 
struct rgb *color, float indent, float length) {}
static void rlib_csv_draw_cell_background_start(rlib *r, float left_origin, float bottom_origin, float how_long, float how_tall, struct rgb *color) {}
static void rlib_csv_draw_cell_background_end(rlib *r) {}
static void rlib_csv_boxurl_start(rlib *r, float left_origin, float bottom_origin, float how_long, float how_tall, char *url) {}
static void rlib_csv_boxurl_end(rlib *r) {}
static void rlib_csv_drawimage(rlib *r, float left_origin, float bottom_origin, char *nname, char *type, float nwidth, 
	float nheight) {}

static void rlib_csv_set_font_point(rlib *r, int point) {}
static void rlib_csv_start_new_page(rlib *r) {
	r->position_bottom = 11-GET_MARGIN(r)->bottom_margin;
}

static void rlib_csv_init_end_page(rlib *r) {}

static void rlib_csv_end_text(rlib *r) {}

static void rlib_csv_init_output(rlib *r) {}

static void rlib_csv_begin_text(rlib *r) {}

static void rlib_csv_finalize_private(rlib *r) {
	r->length = OUTPUT_PRIVATE(r)->top_size;
}

static void rlib_csv_spool_private(rlib *r) {
	rlib_write_output(OUTPUT_PRIVATE(r)->top, strlen(OUTPUT_PRIVATE(r)->top));
}

static void rlib_csv_start_line(rlib *r, int backwards) {}

static void really_print_text(rlib *r, char *text) {
	char buf[MAXSTRLEN];
	char *str_ptr;
	int text_size = strlen(text);
	int *size;

	if(text != NULL && text[0] != '\n') {
		sprintf(buf, "\"%s\",", text);
		text_size += 3;
	} else {
		strcpy(buf, text);
	}
	
	make_more_space_if_necessary(&OUTPUT_PRIVATE(r)->top, &OUTPUT_PRIVATE(r)->top_size, 
		&OUTPUT_PRIVATE(r)->top_total_size, text_size);
	str_ptr = OUTPUT_PRIVATE(r)->top;	
	size = &OUTPUT_PRIVATE(r)->top_size;
	memcpy(str_ptr + (*size), buf, text_size+1);
	*size = (*size) + text_size;

}

static void rlib_csv_end_line(rlib *r, int backwards) {}
static void rlib_csv_init_output_report(rlib *r) {}

static void rlib_csv_start_output_section(rlib *r) {
	int i;
	for(i=0;i<MAX_COL;i++) {
		OUTPUT_PRIVATE(r)->col[i][0] = 0;
	}
}

static void rlib_csv_end_output_section(rlib *r) {
	int i;
	int biggest = 0;
	for(i=0;i<MAX_COL;i++)
		if(OUTPUT_PRIVATE(r)->col[i][0] != 0)
			biggest = i;
	
	for(i=0;i<=biggest;i++)
		if(OUTPUT_PRIVATE(r)->col[i][0] != 0) 
			really_print_text(r, OUTPUT_PRIVATE(r)->col[i]);
		else
			really_print_text(r, "");
			
	really_print_text(r, "\n");	
}

static void rlib_csv_end_page(rlib *r) {
	r->current_page_number++;
	r->current_line_number = 1;
	rlib_init_page(r, FALSE);
}

static int rlib_csv_is_single_page(rlib *r) {
	return TRUE;
}

void rlib_csv_new_output_filter(rlib *r) {
	OUTPUT(r) = rmalloc(sizeof(struct output_filter));
	OUTPUT_PRIVATE(r) = rmalloc(sizeof(struct _private));
	bzero(OUTPUT_PRIVATE(r), sizeof(struct _private));
	OUTPUT_PRIVATE(r)->top = NULL;
	OUTPUT_PRIVATE(r)->top_size = 0;
	OUTPUT_PRIVATE(r)->top_total_size = 0;

	OUTPUT(r)->do_align = FALSE;	
	OUTPUT(r)->do_break = FALSE;	

	OUTPUT(r)->rlib_get_string_width = rlib_csv_get_string_width;
	OUTPUT(r)->rlib_print_text = rlib_csv_print_text;
	OUTPUT(r)->rlib_set_fg_color = rlib_csv_set_fg_color;
	OUTPUT(r)->rlib_set_bg_color = rlib_csv_set_bg_color;
	OUTPUT(r)->rlib_hr = rlib_csv_hr;
	OUTPUT(r)->rlib_draw_cell_background_start = rlib_csv_draw_cell_background_start;
	OUTPUT(r)->rlib_draw_cell_background_end = rlib_csv_draw_cell_background_end;
	OUTPUT(r)->rlib_boxurl_start = rlib_csv_boxurl_start;
	OUTPUT(r)->rlib_boxurl_end = rlib_csv_boxurl_end;
	OUTPUT(r)->rlib_drawimage = rlib_csv_drawimage;
	OUTPUT(r)->rlib_set_font_point = rlib_csv_set_font_point;
	OUTPUT(r)->rlib_start_new_page = rlib_csv_start_new_page;
	OUTPUT(r)->rlib_end_page = rlib_csv_end_page;	
	OUTPUT(r)->rlib_init_end_page = rlib_csv_init_end_page;
	OUTPUT(r)->rlib_end_text = rlib_csv_end_text;
	OUTPUT(r)->rlib_init_output = rlib_csv_init_output;
	OUTPUT(r)->rlib_init_output_report = rlib_csv_init_output_report;
	OUTPUT(r)->rlib_begin_text = rlib_csv_begin_text;
	OUTPUT(r)->rlib_finalize_private = rlib_csv_finalize_private;
	OUTPUT(r)->rlib_spool_private = rlib_csv_spool_private;
	OUTPUT(r)->rlib_start_line = rlib_csv_start_line;
	OUTPUT(r)->rlib_end_line = rlib_csv_end_line;
	OUTPUT(r)->rlib_is_single_page = rlib_csv_is_single_page;
	OUTPUT(r)->rlib_start_output_section = rlib_csv_start_output_section;	
	OUTPUT(r)->rlib_end_output_section = rlib_csv_end_output_section;	
}
