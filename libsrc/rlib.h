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
 * 
 * $Id$s
 *
 * This module defines constants and structures used by most of the C code
 * modules in the library.
 *
 */
#include <libxml/parser.h>
#include <time.h>
#include <glib.h>

#include "containers.h"
#include "charencoder.h"
#include "datetime.h"
#include "util.h"

#define USE_RLIB_VAR	0

#define RLIB_WEB_CONTENT_TYPE_HTML "Content-Type: text/html; charset=%s\n"
#define RLIB_WEB_CONTENT_TYPE_TEXT "Content-Type: text/plain; charset=%s\n"
#define RLIB_WEB_CONTENT_TYPE_PDF "Content-Type: application/pdf\n"
#define RLIB_WEB_CONTENT_TYPE_CSV "Content-type: application/octet-stream\nContent-Disposition: attachment; filename=report.csv\n"

#define RLIB_NAVIGATE_FIRST 1
#define RLIB_NAVIGATE_NEXT 2
#define RLIB_NAVIGATE_PREVIOUS 3
#define RLIB_NAVIGATE_LAST 4

#define RLIB_ENCODING "UTF-8"


//man 3 llabs says the prototype is in stdlib.. no it aint!
long long int llabs(long long int j);
double trunc(double x);
 
#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef MAXSTRLEN
#define MAXSTRLEN 1024
#endif

#define RLIB_MAXIMUM_QUERIES	50
#define RLIB_MAXIMUM_REPORTS	5

#define RLIB_CONTENT_TYPE_ERROR	-1
#define RLIB_CONTENT_TYPE_PDF 	1
#define RLIB_CONTENT_TYPE_HTML	2
#define RLIB_CONTENT_TYPE_TXT		3
#define RLIB_CONTENT_TYPE_CSV		4

#define RLIB_MAXIMUM_PAGES_ACROSS	100

#define RLIB_ELEMENT_LITERAL 1
#define RLIB_ELEMENT_FIELD   2
#define RLIB_ELEMENT_REPORT  3
#define RLIB_ELEMENT_PART    4
#define RLIB_ELEMENT_TR      5
#define RLIB_ELEMENT_TD      6
#define RLIB_ELEMENT_LOAD    7

#define RLIB_FORMAT_PDF 	1
#define RLIB_FORMAT_HTML	2
#define RLIB_FORMAT_TXT 	3
#define RLIB_FORMAT_CSV 	4
#define RLIB_FORMAT_XML 	5

#define RLIB_ORIENTATION_PORTRAIT	1
#define RLIB_ORIENTATION_LANDSCAPE	2

#define RLIB_DEFAULT_BOTTOM_MARGIN .2
#define RLIB_DEFAULT_LEFT_MARGIN	.2
#define RLIB_DEFAULT_TOP_MARGIN 	.2

#define RLIB_FILE_XML_STR		400
#define RLIB_FILE_OUTPUTS		500
#define RLIB_FILE_ROA 			600
#define RLIB_FILE_LINE 			700
#define RLIB_FILE_VARIABLES	800
#define RLIB_FILE_VARIABLE		850
#define RLIB_FILE_BREAKS 		900
#define RLIB_FILE_BREAK 		950
#define RLIB_FILE_BREAK_FIELD	975

#define RLIB_PAPER_LETTER     1
#define RLIB_PAPER_LEGAL      2
#define RLIB_PAPER_A4         3
#define RLIB_PAPER_B5         4
#define RLIB_PAPER_C5         5
#define RLIB_PAPER_DL         6
#define RLIB_PAPER_EXECUTIVE  7
#define RLIB_PAPER_COMM10     8
#define RLIB_PAPER_MONARCH    9
#define RLIB_PAPER_FILM35MM   10

#define RLIB_PDF_DPI 72.0

#define RLIB_LAYOUT_FIXED 1
#define RLIB_LAYOUT_FLOW  2

#define RLIB_GET_LINE(a) ((float)(a/RLIB_PDF_DPI))

#define RLIB_SIGNAL_ROW_CHANGE       0
#define RLIB_SIGNAL_REPORT_DONE      1
#define RLIB_SIGNAL_REPORT_ITERATION 2
#define RLIB_SIGNAL_PART_ITERATION   3

#define RLIB_SIGNALS 4

struct rlib_paper {
	char type;
	long width;
	long height;
	char name[30];
};

struct rlib_value {
	gint type;
	gint64 number_value;
	struct rlib_datetime date_value;
	gchar *string_value;
	gpointer iif_value;  
	gint free;
};

struct rlib_element {
	gint type;
	gpointer data;
	struct rlib_element *next;
};

#define RLIB_ALIGN_LEFT 	0
#define RLIB_ALIGN_RIGHT	1
#define RLIB_ALIGN_CENTER	2

struct rlib_report_literal {
	gchar value[MAXSTRLEN];
	xmlChar *xml_align;
	xmlChar *xml_color;
	xmlChar *xml_bgcolor;
	xmlChar *xml_width;
	xmlChar *xml_bold;
	xmlChar *xml_italics;
	xmlChar *xml_col;
	
	gint width;
	gint align;
	
	struct rlib_pcode *color_code;
	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *col_code;
	struct rlib_pcode *width_code;
	struct rlib_pcode *bold_code;
	struct rlib_pcode *italics_code;
	struct rlib_pcode *align_code;
};

struct rlib_resultset_field {
	gint resultset;
	gpointer field;
};

struct rlib_results {
	gchar *name;
	gpointer result;
	gboolean next_failed;
	gboolean navigation_failed;
	struct input_filter *input;
};

struct rlib_line_extra_data {
	gint type;
	struct rlib_value rval_code;
	struct rlib_value rval_link;
	struct rlib_value rval_bgcolor;
	struct rlib_value rval_color;
	struct rlib_value rval_bold;
	struct rlib_value rval_italics;
	struct rlib_value rval_col;
	gint font_point;
	gchar formatted_string[MAXSTRLEN];
	gint width;	
	gint col;	
	struct rlib_rgb bgcolor;
	gint found_bgcolor;
	gchar *link;
	gint found_link;
	struct rlib_rgb color;
	gint found_color;
	gfloat output_width;
	gint running_bgcolor_status;
	gfloat running_bg_total;
	gint running_link_status;
	gfloat running_link_total;
	gboolean is_bold;
	gboolean is_italics;
};

struct rlib_report_field {
	gchar value[MAXSTRLEN];
	xmlChar *xml_align;
	xmlChar *xml_bgcolor;
	xmlChar *xml_color;
	xmlChar *xml_width;
	xmlChar *xml_bold;
	xmlChar *xml_italics;
	xmlChar *xml_format;
	xmlChar *xml_link;
	xmlChar *xml_col;
	xmlChar *xml_memo;
	xmlChar *xml_memo_height;
	xmlChar *xml_memo_wrap_chars;

	gint width;
	gint align;
	
	struct rlib_pcode *code;
	struct rlib_pcode *format_code;
	struct rlib_pcode *link_code;
	struct rlib_pcode *color_code;
	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *col_code;
	struct rlib_pcode *width_code;
	struct rlib_pcode *bold_code;
	struct rlib_pcode *italics_code;
	struct rlib_pcode *align_code;
	struct rlib_pcode *memo_code;
	struct rlib_pcode *memo_height_code;
	struct rlib_pcode *memo_wrap_chars_code;
		
	struct rlib_value *rval;
};

#define RLIB_REPORT_PRESENTATION_DATA_LINE	1
#define RLIB_REPORT_PRESENTATION_DATA_HR 	2
#define RLIB_REPORT_PRESENTATION_DATA_IMAGE	3

struct rlib_report_output {
	gint type;
	gpointer data;
};

struct rlib_report_output_array {
	gint count;
	xmlChar *xml_page;
	gint page;
	struct rlib_report_output **data;
};

struct rlib_report_horizontal_line {
	xmlChar *xml_bgcolor;
	xmlChar *xml_size;
	xmlChar *xml_indent;
	xmlChar *xml_length;
	xmlChar *xml_font_size;
	xmlChar *xml_suppress;

	gint font_point;
	gfloat size;
	gint indent;
	gint length;

	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *size_code;
	struct rlib_pcode *indent_code;
	struct rlib_pcode *length_code;
	struct rlib_pcode *font_size_code;
	struct rlib_pcode *suppress_code;
};

struct rlib_report_image {
	xmlChar *xml_value;
	xmlChar *xml_type;
	xmlChar *xml_width;
	xmlChar *xml_height;
	
	struct rlib_pcode *value_code;
	struct rlib_pcode *type_code;
	struct rlib_pcode *width_code;
	struct rlib_pcode *height_code;
};

struct rlib_report_lines {
	xmlChar *xml_bgcolor;
	xmlChar *xml_color;
	xmlChar *xml_bold;
	xmlChar *xml_italics;
	xmlChar *xml_font_size;
	xmlChar *xml_suppress;

	gint font_point;

	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *color_code;
	struct rlib_pcode *suppress_code;
	struct rlib_pcode *font_size_code;
	struct rlib_pcode *bold_code;
	struct rlib_pcode *italics_code;
	
	struct rlib_element *e;
};

struct rlib_break_fields {
	xmlChar *xml_value;
	struct rlib_pcode *code;
	struct rlib_value rval2;
	struct rlib_value *rval;
};

struct rlib_report_break {
	xmlChar *xml_name;
	xmlChar *xml_newpage;
	xmlChar *xml_headernewpage;
	xmlChar *xml_suppressblank;

	gint didheader;
	gint headernewpage;
	gint suppressblank;

	struct rlib_element *header;
	struct rlib_element *fields;
	struct rlib_element *footer;
	
	struct rlib_pcode *newpage_code;
	struct rlib_pcode *headernewpage_code;
	struct rlib_pcode *suppressblank_code;
};

struct rlib_report_detail {
	struct rlib_element *headers;
	struct rlib_element *fields;
};

struct rlib_report_alternate {
	struct rlib_element *nodata;
};

struct rlib_count_amount {
	struct rlib_value count;
	struct rlib_value amount;
};

#define RLIB_REPORT_VARIABLE_UNDEFINED	-1 
#define RLIB_REPORT_VARIABLE_EXPRESSION	1
#define RLIB_REPORT_VARIABLE_COUNT 		2
#define RLIB_REPORT_VARIABLE_SUM	 		3
#define RLIB_REPORT_VARIABLE_AVERAGE 		4
#define RLIB_REPORT_VARIABLE_LOWEST		5
#define RLIB_REPORT_VARIABLE_HIGHEST		6

#define RLIB_VARIABLE_CA(a)	((struct rlib_count_amount *)a->data)

struct rlib_report_variable {
	xmlChar *xml_name;
	xmlChar *xml_str_type;
	xmlChar *xml_value;
	xmlChar *xml_resetonbreak;

	gint type;
	struct rlib_pcode *code;
	gpointer data;	
};

struct rlib_part_load {
	xmlChar *xml_name;
	struct rlib_pcode *name_code;
};

struct rlib_part_td {
	xmlChar *xml_width;
	xmlChar *xml_height;
	xmlChar *xml_border_width;
	xmlChar *xml_border_color;
	struct rlib_pcode *width_code;
	struct rlib_pcode *height_code;
	struct rlib_pcode *border_width_code;
	struct rlib_pcode *border_color_code;
	GSList *reports;
};

struct rlib_part_tr {
	xmlChar *xml_layout;
	xmlChar *xml_newpage;

	struct rlib_pcode *layout_code;
	struct rlib_pcode *newpage_code;
	gchar layout;

	GSList *part_deviations;
};

struct rlib_part {
	xmlChar *xml_name;
	xmlChar *xml_pages_across;
	xmlChar *xml_orientation;
	xmlChar *xml_top_margin;
	xmlChar *xml_left_margin;
	xmlChar *xml_bottom_margin;
	xmlChar *xml_paper_type;
	xmlChar *xml_font_size;
	xmlChar *xml_iterations;

	GSList *part_rows;
	struct rlib_element *report_header;
	struct rlib_element *page_header;
	struct rlib_element *page_footer;

	struct rlib_pcode *name_code;
	struct rlib_pcode *pages_across_code;
	struct rlib_pcode *orientation_code;
	struct rlib_pcode *top_margin_code;
	struct rlib_pcode *left_margin_code;
	struct rlib_pcode *bottom_margin_code;
	struct rlib_pcode *paper_type_code;
	struct rlib_pcode *font_size_code;
	struct rlib_pcode *iterations_code;

	struct rlib_paper *paper;
	gint orientation;
	gint font_size;
	gint pages_across;
	gint iterations;
	gfloat *position_top;
	gfloat *position_bottom;
	gfloat *bottom_size;
	gfloat top_margin;
	gfloat bottom_margin;
	gfloat left_margin;
	gint landscape;
};

struct rlib_graph_plot {
	gchar *xml_axis;
	gchar *xml_field;
	gchar *xml_label;
	struct rlib_value rval_axis;
	struct rlib_value rval_field;
	struct rlib_value rval_label;
	struct rlib_pcode *axis_code;
	struct rlib_pcode *field_code;	
	struct rlib_pcode *label_code;	
};

#define RLIB_GRAPH_TYPE_LINE_NORMAL                   1
#define RLIB_GRAPH_TYPE_LINE_STACKED                  2
#define RLIB_GRAPH_TYPE_LINE_PERCENT                  3
#define RLIB_GRAPH_TYPE_AREA_NORMAL                   4
#define RLIB_GRAPH_TYPE_AREA_STACKED                  5
#define RLIB_GRAPH_TYPE_AREA_PERCENT                  6
#define RLIB_GRAPH_TYPE_COLUMN_NORMAL                 7
#define RLIB_GRAPH_TYPE_COLUMN_STACKED                8
#define RLIB_GRAPH_TYPE_COLUMN_PERCENT                9
#define RLIB_GRAPH_TYPE_ROW_NORMAL                   10
#define RLIB_GRAPH_TYPE_ROW_STACKED                  11
#define RLIB_GRAPH_TYPE_ROW_PERCENT                  12
#define RLIB_GRAPH_TYPE_PIE_NORMAL                   13
#define RLIB_GRAPH_TYPE_PIE_RING                     14
#define RLIB_GRAPH_TYPE_PIE_OFFSET                   15
#define RLIB_GRAPH_TYPE_XY_SYMBOLS_ONLY              16
#define RLIB_GRAPH_TYPE_XY_LINES_WITH_SUMBOLS        17
#define RLIB_GRAPH_TYPE_XY_LINES_ONLY                18
#define RLIB_GRAPH_TYPE_XY_CUBIC_SPLINE              19
#define RLIB_GRAPH_TYPE_XY_CUBIC_SPLINE_WIHT_SYMBOLS 20
#define RLIB_GRAPH_TYPE_XY_BSPLINE                   21
#define RLIB_GRAPH_TYPE_XY_BSPLINE_WITH_SYMBOLS      22

struct rlib_graph {
	gchar *xml_type;	
	gchar *xml_subtype;
	gchar *xml_width;
	gchar *xml_height;
	gchar *xml_title;
	gchar *xml_x_axis_title;
	gchar *xml_y_axis_title;
	struct rlib_pcode *type_code;	
	struct rlib_pcode *subtype_code;	
	struct rlib_pcode *width_code;
	struct rlib_pcode *height_code;
	struct rlib_pcode *title_code;
	struct rlib_pcode *x_axis_title_code;
	struct rlib_pcode *y_axis_title_code;
	GSList *plots;
};

struct rlib_report {
	xmlDocPtr doc;
	gchar *contents;
	xmlChar *xml_font_size;
	xmlChar *xml_query;
	xmlChar *xml_orientation;
	xmlChar *xml_top_margin;
	xmlChar *xml_left_margin;
	xmlChar *xml_bottom_margin;
	xmlChar *xml_pages_across;
	xmlChar *xml_suppress_page_header_first_page;
	xmlChar *xml_height;
	xmlChar *xml_iterations;
	
	gchar xml_encoding_name[ENCODING_NAME_SIZE]; //UTF8 if "", else whatever specified in xml
	rlib_char_encoder *output_encoder;
	
	gfloat *position_top;
	gfloat *position_bottom;
	gfloat *bottom_size;

	gint main_loop_query;
	gint raw_page_number;

	gint orientation;
	gint font_size;
	gfloat top_margin;
	gfloat bottom_margin;
	gfloat left_margin;
	gfloat page_width;
	gint iterations;
	gint pages_across;
	gint suppress_page_header_first_page;
	gboolean is_the_only_report;
	struct rlib_pcode *iterations_code;
	
	struct rlib_element *report_header;
	struct rlib_element *page_header;
	struct rlib_report_detail detail;
	struct rlib_element *page_footer;
	struct rlib_element *report_footer;
	struct rlib_element *variables;
	struct rlib_element *breaks;
	struct rlib_report_alternate alternate;
	struct rlib_graph graph;
	gint mainloop_query;

	struct rlib_pcode *font_size_code;
	struct rlib_pcode *query_code;
	struct rlib_pcode *orientation_code;
	struct rlib_pcode *height_code;
	struct rlib_pcode *top_margin_code;
	struct rlib_pcode *left_margin_code;
	struct rlib_pcode *bottom_margin_code;
	struct rlib_pcode *pages_across_code;
	struct rlib_pcode *suppress_page_header_first_page_code;
};

struct rlib_queries {
	gchar *sql;
	gchar *name;
	struct input_filter *input;
};

#define RLIB_REPORT_TYPE_FILE 1
#define RLIB_REPORT_TYPE_BUFFER 2

struct rlib_rip_reports {
	gchar *name;
	gchar type;
};

#define MAX_INPUT_FILTERS	10

struct input_filters {
	gchar *name;
	gpointer handle;
	struct input_filter *input;
};

#define RLIB_MAXIMUM_FOLLOWERS	10
struct rlib_resultset_followers {
	gint leader;
	gint follower;
	gchar *leader_field;
	gchar *follower_field;
	struct rlib_pcode *leader_code;
	struct rlib_pcode *follower_code;
};

struct rlib_signal_functions {
	gboolean (*signal_function)(gpointer, gpointer);
	gpointer data;
};

struct rlib_metadata {
	gchar *xml_formula;
	struct rlib_value rval_formula;
	struct rlib_pcode *formula_code;
};

struct rlib {
	gint current_page_number;
	gint total_pages_allocated;
	gint current_line_number;
	gint detail_line_count;
	gint start_of_new_report;
	
	gint font_point;

	gint current_font_point;

	GHashTable * output_paramaters;
	GHashTable * input_metadata;
	
	rlib_char_encoder *output_encoder;		//_destroy all of these
	rlib_char_encoder *db_encoder;
	rlib_char_encoder *param_encoder;

	rlib_char_encoder *current_output_encoder; //DO NOT use _destroy these
	rlib_char_encoder *current_db_encoder;
	rlib_char_encoder *current_param_encoder;
	
	time_t now; //set when rlib starts now will then be a constant over the time of the report
	
	struct rlib_signal_functions signal_functions[RLIB_SIGNALS];
	
	struct rlib_queries queries[RLIB_MAXIMUM_QUERIES];

	gint queries_count;
	struct rlib_rip_reports reportstorun[RLIB_MAXIMUM_REPORTS];
	struct rlib_results results[RLIB_MAXIMUM_QUERIES];
	
	struct rlib_part *parts[RLIB_MAXIMUM_REPORTS];
	gint parts_count;
	
	gint current_result;

	gint resultset_followers_count;
	struct rlib_resultset_followers followers[RLIB_MAXIMUM_FOLLOWERS];

	gint format;
	gint inputs_count;

	struct output_filter *o;
	struct input_filters inputs[MAX_INPUT_FILTERS];
	struct environment_filter *environment;
	rlib_hashtable_ptr htParameters;
};
typedef struct rlib rlib;

#define INPUT(r, i) (r->results[i].input)
#define ENVIRONMENT(r) (r->environment)
#define ENVIRONMENT_PRIVATE(r) (((struct _private *)r->evnironment->private))

struct environment_filter {
	gpointer private;
	gchar *(*rlib_resolve_memory_variable)(char *);
	gint (*rlib_write_output)(char *, int);
	void (*free)(rlib *);
};

#define OUTPUT(r) (r->o)
#define OUTPUT_PRIVATE(r) (((struct _private *)r->o->private))

struct output_filter {
	gpointer *private;
	gint do_align;
	gint do_break;
	gint do_grouptext;
	gint paginate;
	gfloat (*get_string_width)(rlib *, char *);
	void (*print_text)(rlib *, float, float, char *, int, int);
	void (*set_fg_color)(rlib *, float, float, float);
	void (*set_bg_color)(rlib *, float, float, float);
	void (*hr)(rlib *, int, float, float, float, float, struct rlib_rgb *, float, float);
	void (*start_draw_cell_background)(rlib *, float, float, float, float, struct rlib_rgb *);
	void (*end_draw_cell_background)(rlib *);
	void (*start_boxurl)(rlib *, struct rlib_part *part, float, float, float, float, char *);
	void (*end_boxurl)(rlib *);
	void (*start_bold)(rlib *);
	void (*end_bold)(rlib *);
	void (*start_italics)(rlib *);
	void (*end_italics)(rlib *);
	void (*drawimage)(rlib *, float, float, char *, char *, float, float);
	void (*set_font_point)(rlib *, int);
	void (*start_new_page)(rlib *, struct rlib_part *);
	void (*end_page)(rlib *, struct rlib_part *);
	void (*end_page_again)(rlib *, struct rlib_part *, struct rlib_report *);
	void (*init_end_page)(rlib *);
	void (*init_output)(rlib *);
	void (*set_working_page)(rlib *, struct rlib_part *, int);
	void (*set_raw_page)(rlib *, struct rlib_part *, int);
	void (*start_report)(rlib *, struct rlib_part *);
	void (*end_part)(rlib *, struct rlib_part *);
	void (*end_report)(rlib *, struct rlib_report *);
	void (*finalize_private)(rlib *);
	void (*spool_private)(rlib *);
	void (*start_line)(rlib *, int);
	void (*end_line)(rlib *, int);
	void (*start_output_section)(rlib *);
	void (*end_output_section)(rlib *);
	char *(*get_output)(rlib *);
	long (*get_output_length)(rlib *);
	void (*start_tr)(rlib *);
	void (*end_tr)(rlib *);
	void (*start_td)(rlib *, struct rlib_part *part, gfloat left_margin, gfloat bottom_margin, int width, int height, gint border_width, struct rlib_rgb *color);
	void (*end_td)(rlib *);
	
	void (*graph_start)(rlib *r, float, float, float, float);
	void (*graph_title)(rlib *r, gchar *title);
	void (*graph_x_axis_title)(rlib *r, gchar *title);
	void (*graph_y_axis_title)(rlib *r, gchar *title);
	void (*graph_set_limits)(rlib *r, gdouble min, gdouble max, gdouble origin);
	void (*graph_do_grid)(rlib *r);
	void (*graph_draw_line)(rlib *, float, float, float, float, struct rlib_rgb *);
	void (*graph_set_x_iterations)(rlib *, int iterations);
	void (*graph_tick_x)(rlib *);
	void (*graph_tick_y)(rlib *, int iterations);
	void (*graph_set_data_plot_count)(rlib *r, int count);
	void (*graph_hint_label_x)(rlib *r, gchar *label);
	void (*graph_label_x)(rlib *r, int iteration, gchar *label);
	void (*graph_label_y)(rlib *r, int iteration, gchar *label, gboolean false_x);
	void (*graph_draw_bar)(rlib *r, int iteration, int plot, gfloat height, struct rlib_rgb *,gfloat last_height);
	void (*graph_hint_label_y)(rlib *r, gchar *string);
	void (*graph_hint_legend)(rlib *r, gchar *string);
	void (*graph_draw_legend)(rlib *r);
	void (*graph_draw_legend_label)(rlib *r, gint iteration, gchar *string, struct rlib_rgb *);
	int (*free)(rlib *r);
};

/***** PROTOTYPES: breaks.c ***************************************************/
void rlib_force_break_headers(rlib *r, struct rlib_part *part, struct rlib_report *report);
void rlib_handle_break_headers(rlib *r, struct rlib_part *part, struct rlib_report *report);
void rlib_reset_variables_on_break(rlib *r, struct rlib_part *part, struct rlib_report *report, gchar *name);
void rlib_break_all_below_in_reverse_order(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_element *e);
void rlib_handle_break_footers(rlib *r, struct rlib_part *part, struct rlib_report *report);

/***** PROTOTYPES: formatstring.c *********************************************/
gint rlb_string_sprintf(char *dest, gchar *fmtstr, struct rlib_value *rval);
gint rlib_number_sprintf(char *dest, gchar *fmtstr, const struct rlib_value *rval, gint special_format);
gint rlib_format_string(rlib *r, struct rlib_report_field *rf, struct rlib_value *rval, gchar *buf);

/***** PROTOTYPES: fxp.c ******************************************************/
gint64 rlib_fxp_mul(gint64 a, gint64 b, gint64 factor);
gint64 rlib_fxp_div(gint64 num, gint64 denom, gint places);

/***** PROTOTYPES: api.c ******************************************************/
rlib * rlib_init(void);
rlib * rlib_init_with_environment(struct environment_filter *environment);
gint rlib_add_query_as(rlib *r, gchar *input_name, gchar *sql, gchar *name);
gint rlib_add_query_pointer_as(rlib *r, gchar *input_source, gchar *sql, gchar *name);
gint rlib_add_report(rlib *r, gchar *name);
gint rlib_add_report_from_buffer(rlib *r, gchar *buffer);
gint rlib_execute(rlib *r);
gchar * rlib_get_content_type_as_text(rlib *r);
gint rlib_spool(rlib *r);
gint rlib_set_output_format(rlib *r, gint format);
gint rlib_set_output_format_from_text(rlib *r, gchar * name);
gboolean rlib_query_refresh(rlib *r);
gboolean rlib_signal_connect_string(rlib *r, gchar *signal_name, gboolean (*signal_function)(rlib *, gpointer), gpointer data);
gboolean rlib_signal_connect(rlib *r, gint signal_number, gboolean (*signal_function)(rlib *, gpointer), gpointer data);
gchar *rlib_get_output(rlib *r);
gint rlib_get_output_length(rlib *r);
gint rlib_mysql_report(gchar *hostname, gchar *username, gchar *password, gchar *database, gchar *xmlfilename, gchar *sqlquery, 
	gchar *outputformat);
gint rlib_postgre_report(gchar *connstr, gchar *xmlfilename, gchar *sqlquery, gchar *outputformat);
gint rlib_add_resultset_follower(rlib *r, gchar *leader, gchar *follower);
gint rlib_add_resultset_follower_n_to_1(rlib *r, gchar *leader, gchar *leader_field, gchar *follower,gchar *follower_field );
gint rlib_add_parameter(rlib *r, const gchar *name, const gchar *value);
gint rlib_set_locale(rlib *r, gchar *locale);
void rlib_init_profiler(void);
void rlib_dump_profile(gint profilenum, const gchar *filename);
void rlib_trap(void); //For internals debugging only
gchar *rlib_version(); // returns the version string.
void rlib_set_encodings(rlib *r, const char *output, const char *database, const char *params);
void rlib_set_database_encoding(rlib *r, const char *encoding);
void rlib_set_parameter_encoding(rlib *r, const char *encoding);
gint rlib_set_datasource_encoding(rlib *r, gchar *input_name, gchar *encoding);
void rlib_set_output_encoding(rlib *r, const char *encoding);
void rlib_set_output_paramater(rlib *r, gchar *paramater, gchar *value);


/***** PROTOTYPES: parsexml.c *************************************************/
struct rlib_part * parse_part_file(rlib *r, gchar *filename, gchar type);
struct rlib_report_output * report_output_new(gint type, gpointer data);

/***** PROTOTYPES: pcode.c ****************************************************/
struct rlib_value * rlib_execute_pcode(rlib *r, struct rlib_value *rval, struct rlib_pcode *code, struct rlib_value *this_field_value);
gint64 rlib_str_to_long_long(gchar *str);
struct rlib_pcode * rlib_infix_to_pcode(rlib *r, struct rlib_report *report, gchar *infix, gboolean look_at_metadata);
gint rvalcmp(struct rlib_value *v1, struct rlib_value *v2);
gint rlib_value_free(struct rlib_value *rval);
struct rlib_value * rlib_value_dup(struct rlib_value *orig);
struct rlib_value * rlib_value_dup_contents(struct rlib_value *rval);
struct rlib_value * rlib_value_new_error(struct rlib_value *rval);
gint rlib_execute_as_int(rlib *r, struct rlib_pcode *pcode, gint *result);
gint rlib_execute_as_boolean(rlib *r, struct rlib_pcode *pcode, gint *result);
gint rlib_execute_as_string(rlib *r, struct rlib_pcode *pcode, gchar *buf, gint buf_len);
gint rlib_execute_as_int_inlist(rlib *r, struct rlib_pcode *pcode, gint *result, gchar *list[]);
gint rlib_execute_as_float(rlib *r, struct rlib_pcode *pcode, gfloat *result);
void rlib_pcode_free(struct rlib_pcode *code);

/***** PROTOTYPES: reportgen.c ****************************************************/
void rlib_set_report_from_part(rlib *r, struct rlib_part *part, struct rlib_report *report, gfloat top_margin_offset);
gchar *align_text(rlib *r, char *rtn, gint len, gchar *src, gint align, gint width);
gint will_outputs_fit(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_element *e, gint page);
gint will_this_fit(rlib *r, struct rlib_part *part, struct rlib_report *report, gfloat total, gint page);
gfloat get_output_size(rlib *r, struct rlib_report_output_array *roa);
gint rlib_fetch_first_rows(rlib *r);
gint rlib_end_page_if_line_wont_fit(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_element *e) ;
gfloat get_outputs_size(rlib *r, struct rlib_element *e, gint page);
void rlib_init_page(rlib *r, struct rlib_part *part, struct rlib_report *report, gchar report_header);
gint rlib_make_report(rlib *r);
gint rlib_finalize(rlib *r);
void rlib_process_expression_variables(rlib *r, struct rlib_report *report);
gint get_font_point(rlib *r, struct rlib_report_lines *rl);
gint rlib_format_get_number(gchar *name);
gchar * rlib_format_get_name(gint number);

/***** PROTOTYPES: resolution.c ***********************************************/
gint rlib_resolve_rlib_variable(rlib *r, gchar *name);
gchar * rlib_resolve_memory_variable(rlib *r, gchar *name);
gchar * rlib_resolve_field_value(rlib *r, struct rlib_resultset_field *rf);
gint rlib_lookup_result(rlib *r, gchar *name);
gint rlib_resolve_resultset_field(rlib *r, gchar *name, void **rtn_field, gint *rtn_resultset);
struct rlib_report_variable *rlib_resolve_variable(rlib *r, struct rlib_report *report, gchar *name);
void rlib_resolve_report_fields(rlib *r, struct rlib_report *report);
void rlib_resolve_part_fields(rlib *r, struct rlib_part *part);
void rlib_resolve_metadata(rlib *r);
void rlib_resolve_followers(rlib *r);
void rlib_process_input_metadata(rlib *r);

/***** PROTOTYPES: navigation.c ***********************************************/
gint rlib_navigate_next(rlib *r, gint resultset_num);
gint rlib_navigate_first(rlib *r, gint resultset_num);
gint rlib_navigate_previous(rlib *r, gint resultset_num);
gint rlib_navigate_last(rlib *r, gint resultset_num);

/***** PROTOTYPES: environment.c **********************************************/
void rlib_new_c_environment(rlib *r);

/***** PROTOTYPES: free.c *****************************************************/
int rlib_free(rlib *r);
void rlib_free_results(rlib *r);

/***** PROTOTYPES: pdf.c ******************************************************/
void rlib_pdf_new_output_filter(rlib *r);

/***** PROTOTYPES: html.c *****************************************************/
void rlib_html_new_output_filter(rlib *r);

/***** PROTOTYPES: txt.c ******************************************************/
void rlib_txt_new_output_filter(rlib *r);

/***** PROTOTYPES: csv.c ******************************************************/
void rlib_csv_new_output_filter(rlib *r);

/***** PROTOTYPES: mysql.c ****************************************************/
gpointer rlib_mysql_new_input_filter(void);
gpointer rlib_mysql_real_connect(gpointer input_ptr, gchar *group, gchar *host, gchar *user, gchar *password, gchar *database);

/***** PROTOTYPES: datasource.c ***********************************************/
gint rlib_add_datasource(rlib *r, gchar *input_name, struct input_filter *input);
gint rlib_add_datasource_mysql(rlib *r, gchar *input_name, gchar *database_host, gchar *database_user, gchar *database_password, 
	gchar *database_database);
gint rlib_add_datasource_mysql_from_group(rlib *r, gchar *input_name, gchar *group);
gint rlib_add_datasource_postgre(rlib *r, gchar *input_name, gchar *conn);
gint rlib_add_datasource_odbc(rlib *r, gchar *input_name, gchar *source, gchar *user, gchar *password);

/***** PROTOTYPES: postgre.c **************************************************/
gpointer rlib_postgre_new_input_filter(void);
gpointer rlib_postgre_connect(gpointer input_ptr, gchar *conn);

/***** PROTOTYPES: save.c *****************************************************/
gint save_report(struct rlib_report *rep, char *filename);

/***** PROTOTYPES: load.c *****************************************************/
struct rlib_part * load_report(gchar *filename);

/***** PROTOTYPES: layout.c ***************************************************/
gfloat rlib_layout_get_page_width(rlib *r, struct rlib_part *part);
void rlib_layout_init_part_page(rlib *r, struct rlib_part *part, gboolean first);
gint rlib_layout_report_output(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_element *e, gint backwards);
struct rlib_paper * rlib_layout_get_paper(rlib *r, gint paper_type);
struct rlib_paper * rlib_layout_get_paper_by_name(rlib *r, gchar *paper_name);
gint rlib_layout_report_output_with_break_headers(rlib *r, struct rlib_part *part, struct rlib_report *report);
void rlib_layout_init_report_page(rlib *r, struct rlib_part *part, struct rlib_report *report);
void rlib_layout_report_footer(rlib *r, struct rlib_part *part, struct rlib_report *report);
gfloat rlib_layout_get_next_line(rlib *r, struct rlib_part *part, gfloat position, gfloat foint_point);
gint rlib_layout_end_page(rlib *r, struct rlib_part *part, struct rlib_report *report);

/***** PROTOTYPES: graphing.c **************************************************/
void rlib_graph(rlib *r, struct rlib_part *part, struct rlib_report *report, gfloat left_margin_offset, gfloat top_margin_offset);

/***** PROTOTYPES: axis.c ******************************************************/
void rlib_graph_find_y_range(rlib *r, gdouble a, gdouble b, gdouble *y_min, gdouble *y_max, gint graph_type);
gint rlib_graph_num_ticks(rlib *r, gdouble a, gdouble b);
