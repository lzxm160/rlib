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
#include <iconv.h>
#include <time.h>

#include "rhashtable.h"

#define RLIB_WEB_CONTENT_TYPE_HTML "Content-Type: text/html\n"
#define RLIB_WEB_CONTENT_TYPE_TEXT "Content-Type: text/plain\n"
#define RLIB_WEB_CONTENT_TYPE_PDF "Content-Type: application/pdf\n"
#define RLIB_WEB_CONTENT_TYPE_CSV "Content-type: application/octet-stream\nContent-Disposition: attachment; filename=report.csv\n"

#define RLIB_NAVIGATE_FIRST 1
#define RLIB_NAVIGATE_NEXT 2
#define RLIB_NAVIGATE_PREVIOUS 3
#define RLIB_NAVIGATE_LAST 4

//man 3 llabs says the prototype is in stdlib.. no it aint!
long long int llabs(long long int j);

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

#define RLIB_MAXIMUM_PAGES_ACCROSS	100

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
	long long number_value;
	struct tm date_value;
	char *string_value;
	void *iif_value;
	int free;
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
	char value[MAXSTRLEN];
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
	void * field;
};

struct rlib_results {
	char *name;
	void *result;
	struct input_filter *input;
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
	char value[MAXSTRLEN];
	xmlChar *xml_align;
	xmlChar *bgcolor;
	xmlChar *color;
	xmlChar *xml_width;
	xmlChar *format;
	xmlChar *link;
	xmlChar *col;
	xmlChar *xml_wrapchars;
	xmlChar *xml_maxlines;

	long width;
	long align;
	
	struct rlib_pcode *code;
	struct rlib_pcode *format_code;
	struct rlib_pcode *link_code;
	struct rlib_pcode *color_code;
	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *col_code;
	struct rlib_pcode *wrapchars_code;
	struct rlib_pcode *maxlines_code;
	
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
	xmlChar *xml_page;
	long page;
	struct report_output **data;
};

struct report_horizontal_line {
	xmlChar *bgcolor;
	xmlChar *size;
	xmlChar *indent;
	xmlChar *length;
	xmlChar *font_size;
	xmlChar *suppress;

	int font_point;
	float realsize;
	int realindent;
	int reallength;
	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *suppress_code;
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
	xmlChar *font_size;
	xmlChar *suppress;
	int font_point;
	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *color_code;
	struct rlib_pcode *suppress_code;
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
	xmlChar *xml_suppressblank;
	int didheader;
	int newpage;
	int headernewpage;
	int suppressblank;
	struct report_element *header;
	struct report_element *fields;
	struct report_element *footer;
};

struct report_detail {
	struct report_element *textlines;
	struct report_element *fields;
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

struct rlib_report {
	xmlDocPtr doc;
	xmlChar *xml_font_size;
	xmlChar *xml_orientation;
	xmlChar *xml_top_margin;
	xmlChar *xml_left_margin;
	xmlChar *xml_bottom_margin;
	xmlChar *xml_pages_accross;
	xmlChar *xml_suppress_page_header_first_page;

	float *position_top;
	float *position_bottom;
	float *bottom_size;

	long orientation;
	long font_size;
	float top_margin;
	float bottom_margin;
	float left_margin;
	long pages_accross;
	long suppress_page_header_first_page;
	
	struct report_element *report_header;
	struct report_element *page_header;
	struct report_detail detail;
	struct report_element *page_footer;
	struct report_element *report_footer;
	struct report_element *variables;
	struct report_element *breaks;
	int mainloop_query;
	iconv_t cd;
};

struct rlib_queries {
	char *sql;
	char *name;
	struct input_filter *input;
};

struct rip_reports {
	char *name;
	char *query;
};

#define MAX_INPUT_FILTERS	10

struct input_filters {
	char *name;
	void *handle;
	struct input_filter *input;
};

#define RLIB_MAXIMUM_FOLLOWERS	10
struct rlib_resultset_followers {
	int leader;
	int follower;
};

struct rlib {
	int current_page_number;
	int current_line_number;
	int detail_line_count;
	int start_of_new_report;
	
	int font_point;
	int landscape;

	int current_font_point;

	struct rlib_queries queries[RLIB_MAXIMUM_QUERIES];

	int mainloop_queries_count;
	int queries_count;
	struct rip_reports reportstorun[RLIB_MAXIMUM_REPORTS];
	struct rlib_results results[RLIB_MAXIMUM_QUERIES];
	
	struct rlib_report *reports[RLIB_MAXIMUM_REPORTS];
	int reports_count;
	int current_report;
	int current_result;
	
	int resultset_followers_count;
	struct rlib_resultset_followers followers[RLIB_MAXIMUM_FOLLOWERS];

	int format;
	int inputs_count;

	struct output_filter *o;
	struct input_filters inputs[MAX_INPUT_FILTERS];
	struct environment_filter *environment;
	RHashtable *htParameters;
};
typedef struct rlib rlib;

#define INPUT(r, i) (r->results[i].input)
#define ENVIRONMENT(r) (r->environment)
#define ENVIRONMENT_PRIVATE(r) (((struct _private *)r->evnironment->private))

struct environment_filter {
	void *private;
	char *(*rlib_resolve_memory_variable)(char *);
	int (*rlib_write_output)(char *, int);
	void (*free)(rlib *);
};

#define OUTPUT(r) (r->o)
#define OUTPUT_PRIVATE(r) (((struct _private *)r->o->private))


struct output_filter {
	void *private;
	int do_align;
	int do_break;
	int do_grouptext;
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
	void (*rlib_set_working_page)(rlib *, int);
	void (*rlib_start_report)(rlib *);
	void (*rlib_end_report)(rlib *);
	void (*rlib_begin_text)(rlib *);
	void (*rlib_finalize_private)(rlib *);
	void (*rlib_spool_private)(rlib *);
	void (*rlib_start_line)(rlib *, int);
	void (*rlib_end_line)(rlib *, int);
	int (*rlib_is_single_page)(rlib *);
	void (*rlib_start_output_section)(rlib *);
	void (*rlib_end_output_section)(rlib *);
	char *(*rlib_get_output)(rlib *);
	long (*rlib_get_output_length)(rlib *);
	int (*rlib_free)(rlib *r);
};


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
long long rlib_fxp_mul(long long a, long long b, long long factor);
long long rlib_fxp_div( long long num, long long denom, int places);

/***** PROTOTYPES: api.c ******************************************************/
rlib * rlib_init();
rlib * rlib_init_with_environment(struct environment_filter *environment);
int rlib_add_query_as(rlib *r, char *input_name, char *sql, char *name);
int rlib_add_report(rlib *r, char *name, char *mainloop);
int rlib_execute(rlib *r);
char * rlib_get_content_type_as_text(rlib *r);
int rlib_spool(rlib *r);
int rlib_set_output_format(rlib *r, int format);
int rlib_set_output_format_from_text(rlib *r, char * name);
char *rlib_get_output(rlib *r);
long rlib_get_output_length(rlib *r);
int rlib_mysql_report(char *hostname, char *username, char *password, char *database, char *xmlfilename, char *sqlquery, char *outputformat);
int rlib_postgre_report(char *connstr, char *xmlfilename, char *sqlquery, char *outputformat);
int rlib_add_resultset_follower(rlib *r, char *leader, char *follower);
int rlib_add_parameter(rlib *r, const char *name, const char *value);
int rlib_set_locale(rlib *r, char *locale);
void rlib_trap(void); //For internals debugging only


/***** PROTOTYPES: parsexml.c *************************************************/
struct rlib_report * parse_report_file(char *filename);

/***** PROTOTYPES: pcode.c ****************************************************/
struct rlib_value * rlib_execute_pcode(rlib *r, struct rlib_value *rval, struct rlib_pcode *code, struct rlib_value *this_field_value);
long long rlib_str_to_long_long(char *str);
struct rlib_pcode * rlib_infix_to_pcode(rlib *r, char *infix);
int rvalcmp(struct rlib_value *v1, struct rlib_value *v2);
int rlib_value_free(struct rlib_value *rval);
struct rlib_value * rlib_value_dup(struct rlib_value *orig);
struct rlib_value * rlib_value_dup_contents(struct rlib_value *rval);
struct rlib_value * rlib_value_new_error(struct rlib_value *rval);

/***** PROTOTYPES: reportgen.c ************************************************/
char *align_text(rlib *r, char *rtn, int len, char *src, long align, long width);
float get_page_width(rlib *r);
int print_report_output(rlib *r, struct report_element *e, int backwards);
int will_outputs_fit(rlib *r, struct report_element *e, int page);
int will_this_fit(rlib *r, float total, int page);
float get_output_size(rlib *r, struct report_output_array *roa);
void rlib_print_report_footer(rlib *r);
int rlib_fetch_first_rows(rlib *r);
void rlib_init_variables(rlib *r);
int rlib_end_page_if_line_wont_fit(rlib *r, struct report_element *e) ;
float get_outputs_size(rlib *r, struct report_element *e, int page);
void rlib_process_variables(rlib *r);
void rlib_init_page(rlib *r, char report_header);
int make_report(rlib *r);
int rlib_finalize(rlib *r);

/***** PROTOTYPES: resolution.c ***********************************************/
int rlib_resolve_rlib_variable(rlib *r, char *name);
char * rlib_resolve_memory_variable(rlib *r, char *name);
char * rlib_resolve_field_value(rlib *r, struct rlib_resultset_field *rf);
int rlib_lookup_result(rlib *r, char *name);
int rlib_resolve_resultset_field(rlib *r, char *name, void **rtn_field, int *rtn_resultset);
struct report_variable *rlib_resolve_variable(rlib *r, char *name);
void rlib_resolve_fields(rlib *r);

/***** PROTOTYPES: util.c *****************************************************/
char *strlwrexceptquoted (char *s);
char *rmwhitespacesexceptquoted(char *s);
void rlogit(const char *fmt, ...);
void rlogit_setmessagewriter(void(*writer)(const char *msg));
int rutil_enableSignalHandler(int trueorfalse);
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
void init_signals();
void make_more_space_if_necessary(char **str, int *size, int *total_size, int len);

/***** PROTOTYPES: navigation.c ***********************************************/
int rlib_navigate_next(rlib *r, int resultset_num);
int rlib_navigate_first(rlib *r, int resultset_num);
int rlib_navigate_previous(rlib *r, int resultset_num);
int rlib_navigate_last(rlib *r, int resultset_num);

/***** PROTOTYPES: environment.c **********************************************/
void rlib_new_c_environment(rlib *r);

/***** PROTOTYPES: free.c *****************************************************/
int rlib_free(rlib *r);

/***** PROTOTYPES: pdf.c ******************************************************/
void rlib_pdf_new_output_filter(rlib *r);

/***** PROTOTYPES: html.c *****************************************************/
void rlib_html_new_output_filter(rlib *r);

/***** PROTOTYPES: txt.c ******************************************************/
void rlib_txt_new_output_filter(rlib *r);

/***** PROTOTYPES: csv.c ******************************************************/
void rlib_csv_new_output_filter(rlib *r);

/***** PROTOTYPES: mysql.c ****************************************************/
void * rlib_mysql_new_input_filter();
void * rlib_mysql_real_connect(void * input_ptr, char *host, char *user, char *password, char *database);

/***** PROTOTYPES: datasource.c ***********************************************/
int rlib_add_datasource(rlib *r, char *input_name, struct input_filter *input);
int rlib_add_datasource_mysql(rlib *r, char *input_name, char *database_host, char *database_user, char *database_password, char *database_database);
int rlib_add_datasource_postgre(rlib *r, char *input_name, char *conn);
int rlib_add_datasource_odbc(rlib *r, char *input_name, char *source, char *user, char *password);

/***** PROTOTYPES: postgre.c **************************************************/
void * rlib_postgre_new_input_filter();
void * rlib_postgre_connect(void * input_ptr, char *conn);
