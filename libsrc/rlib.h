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
#include <libxml/parser.h>
#include "input.h"

//man 3 llabs says the prototype is in stdlib.. no it aint!
long long int llabs(long long int j);

#define TRUE	1
#define FALSE	0

#define MAXSTRLEN 1024

/*********** STUFF FOR THE PHP SIDE OF THINGS                                  */
#define LE_RLIB_NAME "rlib"
#define RLIB_MAXIMUM_QUERIES	10
#define RLIB_MAXIMUM_REPORTS	5

#define RLIB_CONTENT_TYPE_ERROR	-1
#define RLIB_CONTENT_TYPE_PDF 	1
#define RLIB_CONTENT_TYPE_HTML	2
#define RLIB_CONTENT_TYPE_TXT		3
#define RLIB_CONTENT_TYPE_CSV		4


#define REPORT_ELEMENT_TEXT 1
#define REPORT_ELEMENT_FIELD 2

#define RLIB_FORMAT_PDF 	1
#define RLIB_FORMAT_HTML	2
#define RLIB_FORMAT_TXT 	3
#define RLIB_FORMAT_CSV 	4
#define RLIB_FORMAT_XML 	5

#define RLIB_ORIENTATION_PORTRAIT	1
#define RLIB_ORIENTATION_LANDSCAPE	2

#define GET_MARGIN(r) (r->reports[r->current_report])
#define DEFAULT_BOTTOM_MARGIN .2
#define DEFAULT_LEFT_MARGIN	.2
#define DEFAULT_TOP_MARGIN 	.2

struct rgb {
	float r;
	float g;
	float b;
};

struct rlib_value {
	int type;
	int free;
	void *value;
};

struct report_element {
	int type;
	void *data;
	struct report_element *next;
};

#define RLIB_ALIGN_LEFT 	0
#define RLIB_ALIGN_RIGHT	1
#define RLIB_ALIGN_CENTER	2


struct report_text {
	xmlChar *value;
	xmlChar *xml_align;
	xmlChar *color;
	xmlChar *bgcolor;
	xmlChar *xml_width;
	xmlChar *col;
	
	long width;
	long align;
	
	struct rlib_pcode *color_code;
	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *col_code;
};

struct rlib_resultset_field {
	int resultset;
	int field;
};

struct rlib_line_extra_data {
	int type;
	struct rlib_value rval_code;
	struct rlib_value rval_link;
	struct rlib_value rval_bgcolor;
	struct rlib_value rval_color;
	struct rlib_value rval_col;
	int font_point;
	char formatted_string[MAXSTRLEN];
	long width;
	
	int col;
	
	struct rgb bgcolor;
	int found_bgcolor;
	
	char *link;
	int found_link;
	
	struct rgb color;
	int found_color;
	
	float output_width;
	
	int running_bgcolor_status;
	float running_running_bg_total;
	
};

struct report_field {
	xmlChar *value;
	xmlChar *xml_align;
	xmlChar *bgcolor;
	xmlChar *color;
	xmlChar *xml_width;
	xmlChar *format;
	xmlChar *link;
	xmlChar *col;

	long width;
	long align;

	struct rlib_pcode *code;
	struct rlib_pcode *format_code;
	struct rlib_pcode *link_code;
	struct rlib_pcode *color_code;
	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *col_code;
	struct rlib_value *rval;
};

#define REPORT_PRESENTATION_DATA_LINE	1
#define REPORT_PRESENTATION_DATA_HR 	2
#define REPORT_PRESENTATION_DATA_IMAGE	3

struct report_output {
	int type;
	void *data;
};

struct report_output_array {
	int count;
	struct report_output **data;
};

struct report_horizontal_line {
	xmlChar *bgcolor;
	xmlChar *size;
	xmlChar *indent;
	xmlChar *length;
	xmlChar *fontSize;

	int font_point;
	float realsize;
	int realindent;
	int reallength;
	struct rlib_pcode *bgcolor_code;
};

struct report_image {
	xmlChar *value;
	xmlChar *type;
	xmlChar *width;
	xmlChar *height;
	
	struct rlib_pcode *value_code;
	struct rlib_pcode *type_code;
	struct rlib_pcode *width_code;
	struct rlib_pcode *height_code;
};


struct report_lines {
	xmlChar *bgcolor;
	xmlChar *color;
	xmlChar *fontSize;
	int font_point;
	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *color_code;
	struct report_element *e;
};

struct break_fields {
	xmlChar *value;
	struct rlib_pcode *code;
	struct rlib_value rval2;
	struct rlib_value *rval;
};

struct report_break {
	xmlChar *name;
	xmlChar *xml_newpage;
	xmlChar *xml_headernewpage;
	xmlChar *xml_surpressblank;
	int didheader;
	int newpage;
	int headernewpage;
	int surpressblank;
	struct report_output_array *header;
	struct report_element *fields;
	struct report_output_array *footer;
};

struct report_detail {
	struct report_output_array *textlines;
	struct report_output_array *fields;
};

struct count_amount {
	struct rlib_value count;
	struct rlib_value amount;
};

#define REPORT_VARIABLE_UNDEFINED	-1 
#define REPORT_VARIABLE_EXPRESSION	1
#define REPORT_VARIABLE_COUNT 		2
#define REPORT_VARIABLE_SUM	 		3
#define REPORT_VARIABLE_AVERAGE 		4
#define REPORT_VARIABLE_LOWEST		5
#define REPORT_VARIABLE_HIGHEST		6

#define RLIB_VARIABLE_CA(a)	((struct count_amount *)a->dude)

struct report_variable {
	xmlChar *name;
	xmlChar *str_type;
	xmlChar *value;
	xmlChar *resetonbreak;

	int type;
	struct rlib_pcode *code;
	void *dude;	
};

struct report {
	xmlChar *xml_fontsize;
	xmlChar *defaultResult;
	xmlChar *xml_orientation;
	xmlChar *xml_top_margin;
	xmlChar *xml_left_margin;
	xmlChar *xml_bottom_margin;

	long orientation;
	long fontsize;
	float top_margin;
	float bottom_margin;
	float left_margin;
	
	struct report_output_array *report_header;
	struct report_output_array *page_header;
	struct report_detail detail;
	struct report_output_array *page_footer;
	struct report_output_array *report_footer;
	struct report_element *variables;
	struct report_element *breaks;
	int mainloop_query;
};

struct rlib {
	long length;
	char *bufPDF;
	float position_top;
	float position_bottom;

	int current_page_number;
	int current_line_number;
	int detail_line_count;
	int start_of_new_report;
	
	int font_point;
	int landscape;
	
	int current_font_point;

	int results_count;
	struct report *reports[RLIB_MAXIMUM_REPORTS];
	int reports_count;
	int current_report;
	int current_result;
	int format;
	struct output_filter *o;
	struct input_filter *input;
};
typedef struct rlib rlib;

#define OUTPUT(r) (r->o)
#define OUTPUT_PRIVATE(r) (((struct _private *)r->o->private))

struct output_filter {
	void *private;
	int do_align;
	int do_break;
	float	(*rlib_get_string_width)(rlib *, char *);
	void (*rlib_print_text)(rlib *, float, float, char *, int, int);
	void (*rlib_set_fg_color)(rlib *, float, float, float);
	void (*rlib_set_bg_color)(rlib *, float, float, float);
	void (*rlib_hr)(rlib *, int, float, float, float, float, struct rgb *, float, float);
	void (*rlib_draw_cell_background_start)(rlib *, float, float, float, float, struct rgb *);
	void (*rlib_draw_cell_background_end)(rlib *);
	void (*rlib_boxurl_start)(rlib *, float, float, float, float, char *);
	void (*rlib_boxurl_end)(rlib *);
	void (*rlib_drawimage)(rlib *, float, float, char *, char *, float, float);
	void (*rlib_set_font_point)(rlib *, int);
	void (*rlib_start_new_page)(rlib *);
	void (*rlib_end_page)(rlib *);
	void (*rlib_init_end_page)(rlib *);
	void (*rlib_end_text)(rlib *);
	void (*rlib_init_output)(rlib *);
	void (*rlib_init_output_report)(rlib *);
	void (*rlib_begin_text)(rlib *);
	void (*rlib_finalize_private)(rlib *);
	void (*rlib_spool_private)(rlib *);
	void (*rlib_start_line)(rlib *, int);
	void (*rlib_end_line)(rlib *, int);
	int (*rlib_is_single_page)(rlib *);
	void (*rlib_start_output_section)(rlib *);
	void (*rlib_end_output_section)(rlib *);
};

struct rlib_queries {
	char *sql;
	char *name;
};

struct rip_reports {
	char *name;
	char *query;
};

struct rlib_inout_pass {
	char *database_host;
	char *database_user;
	char *database_password;
	char *database_database;
	struct rlib_queries queries[RLIB_MAXIMUM_QUERIES];
	int queries_count;
	int mainloop_queries_count;
	struct rip_reports reports[RLIB_MAXIMUM_REPORTS];
	int reports_count;
	int content_type;
	int format;
	rlib *r;
};
typedef struct rlib_inout_pass rlib_inout_pass;


/***** PROTOTYPES: breaks.c ***************************************************/
void rlib_force_break_headers(rlib *r);
void rlib_handle_break_headers(rlib *r);
void rlib_reset_variables_on_break(rlib *r, char *name);
void rlib_break_all_below_in_reverse_order(rlib *r, struct report_element *e);
void rlib_handle_break_footers(rlib *r);

/***** PROTOTYPES: formatstring.c *********************************************/
int rlb_string_sprintf(char *dest, char *fmtstr, struct rlib_value *rval);
int rlib_number_sprintf(char *dest, char *fmtstr, const struct rlib_value *rval, int special_format);
int rlib_format_string(rlib *r, struct report_field *rf, struct rlib_value *rval, char *buf);

/***** PROTOTYPES: fxp.c ******************************************************/
long long fxp_mul(long long a, long long b, long long factor);
long long fxp_div( long long num, long long denom, int places);

/***** PROTOTYPES: init.c *****************************************************/
rlib * rlib_init(rlib_inout_pass *rip);

/***** PROTOTYPES: parsexml.c *************************************************/
struct report_element * parseLineArray(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
struct report_lines * parseReportLines(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
struct report_element * parseBreakField(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
struct report_element * parseReportBreak(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
struct report_element * parseReportBreaks(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
void parseDetail(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur, struct report_detail *r);
struct report_element * parseReportVariable(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
struct report_element * parseReportVariables(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
struct report * parse_report_file(char *filename);

/***** PROTOTYPES: pcode.c ****************************************************/
struct rlib_value * rlib_execute_pcode(rlib *r, struct rlib_value *rval, struct rlib_pcode *code, struct rlib_value *this_field_value);
long long rlib_str_to_long_long(char *str);
struct rlib_pcode * rlib_infix_to_pcode(rlib *r, char *infix);
int rvalcmp(struct rlib_value *v1, struct rlib_value *v2);

/***** PROTOTYPES: reportgen.c ************************************************/
char *align_text(rlib *r, char *rtn, int len, char *src, long align, long width);
float get_page_width(rlib *r);
void print_detail_line(rlib *r, struct report_output_array *roa, int backwards);
int will_line_fit(rlib *r, struct report_output_array *roa);
int will_this_fit(rlib *r, float total);
float get_output_size(rlib *r, struct report_output_array *roa);
void rlib_print_report_footer(rlib *r);
int rlib_fetch_first_rows(rlib *r);
void rlib_init_variables(rlib *r);
void rlib_end_page_if_line_wont_fit(rlib *r, struct report_output_array *roa);
void rlib_process_variables(rlib *r);
void rlib_init_page(rlib *r, char report_header);
int make_report(rlib *r);
int rlib_spool(rlib *r);
int rlib_finalize(rlib *r);
int rlib_input_close(rlib *r);

/***** PROTOTYPES: resolution.c ***********************************************/
int rlib_resolve_rlib_variable(rlib *r, char *name);
char * rlib_resolve_memory_variable(rlib *r, char *name);
char * rlib_resolve_field_value(rlib *r, struct rlib_resultset_field *rf);
int rlib_lookup_result(rlib *r, char *name);
int rlib_resolve_resultset_field(rlib *r, char *name, int *value, int *xxresultset);
struct report_variable *rlib_resolve_variable(rlib *r, char *name);
void rlib_resolve_fields(rlib *r);

/***** PROTOTYPES: util.c *****************************************************/
char *strlwrexceptquoted (char *s);
char *rmwhitespacesexceptquoted(char *s);
void debugf(const char *fmt, ...);
long long tentothe(int n);
char hextochar(char c);
char *colornames(char *str);
void parsecolor(struct rgb *color, char *strx);
struct tm * stod(struct tm *tm_date, char *str);
void bumpday(int *year, int *month, int *day);
void bumpdaybackwords(int *year, int *month, int *day);
char *strupr (char *s);
char *strlwr (char *s);
char *strproper (char *s);
int daysinmonth(int year, int month);
void initSignals();

/***** PROTOTYPES: pdf.c ******************************************************/
void rlib_pdf_new_output_filter(rlib *r);

/***** PROTOTYPES: html.c ******************************************************/
void rlib_html_new_output_filter(rlib *r);

/***** PROTOTYPES: txt.c ******************************************************/
void rlib_txt_new_output_filter(rlib *r);

/***** PROTOTYPES: csv.c ******************************************************/
void rlib_csv_new_output_filter(rlib *r);

/***** PROTOTYPES: php.c ******************************************************/
void rlib_write_output(char *buf, long length);
char * rlib_php_resolve_memory_variable(char *name);

/***** PROTOTYPES: sql.c ******************************************************/
void * rlib_mysql_new_input_filter();
