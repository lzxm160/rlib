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
#include <glib.h>

#include "containers.h"

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

#define REPORT_ELEMENT_LITERAL	1
#define REPORT_ELEMENT_FIELD		2

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

#define RLIB_FILE_XML_STR		400
#define RLIB_FILE_OUTPUTS		500
#define RLIB_FILE_ROA 			600
#define RLIB_FILE_LINE 			700
#define RLIB_FILE_VARIABLES	800
#define RLIB_FILE_VARIABLE		850
#define RLIB_FILE_BREAKS 		900
#define RLIB_FILE_BREAK 		950
#define RLIB_FILE_BREAK_FIELD	975

struct rgb {
	gfloat r;
	gfloat g;
	gfloat b;
};


struct rlib_datetime {
	GDate date;
	long ltime;
};


struct rlib_value {
	gint type;
	gint64 number_value;
	struct rlib_datetime date_value;
	gchar *string_value;
	gpointer iif_value;
	gint free;
};

struct report_element {
	gint type;
	gpointer data;
	struct report_element *next;
};

#define RLIB_ALIGN_LEFT 	0
#define RLIB_ALIGN_RIGHT	1
#define RLIB_ALIGN_CENTER	2

struct report_literal {
	gchar value[MAXSTRLEN];
	xmlChar *xml_align;
	xmlChar *color;
	xmlChar *bgcolor;
	xmlChar *xml_width;
	xmlChar *col;
	
	gint width;
	gint align;
	
	struct rlib_pcode *color_code;
	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *col_code;
	struct rlib_pcode *width_code;
	struct rlib_pcode *align_code;
};

struct rlib_resultset_field {
	gint resultset;
	gpointer field;
};

struct rlib_results {
	gchar *name;
	gpointer result;
	struct input_filter *input;
};

struct rlib_line_extra_data {
	gint type;
	struct rlib_value rval_code;
	struct rlib_value rval_link;
	struct rlib_value rval_bgcolor;
	struct rlib_value rval_color;
	struct rlib_value rval_col;
	gint font_point;
	gchar formatted_string[MAXSTRLEN];
	gint width;	
	gint col;	
	struct rgb bgcolor;
	gint found_bgcolor;
	gchar *link;
	gint found_link;
	struct rgb color;
	gint found_color;
	gfloat output_width;
	gint running_bgcolor_status;
	gfloat running_running_bg_total;
};

struct report_field {
	gchar value[MAXSTRLEN];
	xmlChar *xml_align;
	xmlChar *bgcolor;
	xmlChar *color;
	xmlChar *xml_width;
	xmlChar *format;
	xmlChar *link;
	xmlChar *col;
	xmlChar *xml_wrapchars;
	xmlChar *xml_maxlines;

	gint width;
	gint align;
	
	struct rlib_pcode *code;
	struct rlib_pcode *format_code;
	struct rlib_pcode *link_code;
	struct rlib_pcode *color_code;
	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *col_code;
	struct rlib_pcode *wrapchars_code;
	struct rlib_pcode *maxlines_code;

	struct rlib_pcode *width_code;
	struct rlib_pcode *align_code;
		
	struct rlib_value *rval;
};

#define REPORT_PRESENTATION_DATA_LINE	1
#define REPORT_PRESENTATION_DATA_HR 	2
#define REPORT_PRESENTATION_DATA_IMAGE	3

struct report_output {
	gint type;
	gpointer data;
};

struct report_output_array {
	gint count;
	xmlChar *xml_page;
	gint page;
	struct report_output **data;
};

struct report_horizontal_line {
	xmlChar *bgcolor;
	xmlChar *size;
	xmlChar *indent;
	xmlChar *length;
	xmlChar *font_size;
	xmlChar *suppress;

	gint font_point;
	gfloat realsize;
	gint realindent;
	gint reallength;

	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *size_code;
	struct rlib_pcode *indent_code;
	struct rlib_pcode *length_code;
	struct rlib_pcode *font_size_code;
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

	gint font_point;

	struct rlib_pcode *bgcolor_code;
	struct rlib_pcode *color_code;
	struct rlib_pcode *suppress_code;
	struct rlib_pcode *font_size_code;
	
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

	gint didheader;
	gint newpage;
	gint headernewpage;
	gint suppressblank;

	struct report_element *header;
	struct report_element *fields;
	struct report_element *footer;
	
	struct rlib_pcode *newpage_code;
	struct rlib_pcode *headernewpage_code;
	struct rlib_pcode *suppressblank_code;
};

struct report_detail {
	struct report_element *textlines;
	struct report_element *fields;
};

struct report_alternate {
	struct report_element *nodata;
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

	gint type;
	struct rlib_pcode *code;
	gpointer dude;	
};

struct rlib_report {
	xmlDocPtr doc;
	gchar *contents;
	xmlChar *xml_font_size;
	xmlChar *xml_orientation;
	xmlChar *xml_top_margin;
	xmlChar *xml_left_margin;
	xmlChar *xml_bottom_margin;
	xmlChar *xml_pages_accross;
	xmlChar *xml_suppress_page_header_first_page;
	
	gfloat *position_top;
	gfloat *position_bottom;
	gfloat *bottom_size;

	gint orientation;
	gint font_size;
	gfloat top_margin;
	gfloat bottom_margin;
	gfloat left_margin;
	gint pages_accross;
	gint suppress_page_header_first_page;
	
	struct report_element *report_header;
	struct report_element *page_header;
	struct report_detail detail;
	struct report_element *page_footer;
	struct report_element *report_footer;
	struct report_element *variables;
	struct report_element *breaks;
	struct report_alternate alternate;
	gint mainloop_query;
	iconv_t output_encoder;

	struct rlib_pcode *font_size_code;
	struct rlib_pcode *orientation_code;
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

struct rip_reports {
	gchar *name;
	gchar *query;
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
};

struct rlib {
	gint current_page_number;
	gint current_line_number;
	gint detail_line_count;
	gint start_of_new_report;
	
	gint font_point;
	gint landscape;

	gint current_font_point;

	gchar pdf_fontdir1[256];
	gchar pdf_fontdir2[256];
	gchar pdf_encoding[256];
	gchar pdf_fontname[256];
	gboolean utf8;
	iconv_t output_encoder;
	time_t now; //set when rlib starts now will then be a constant over the time of the report
	
	struct rlib_queries queries[RLIB_MAXIMUM_QUERIES];

	gint mainloop_queries_count;
	gint queries_count;
	struct rip_reports reportstorun[RLIB_MAXIMUM_REPORTS];
	struct rlib_results results[RLIB_MAXIMUM_QUERIES];
	
	struct rlib_report *reports[RLIB_MAXIMUM_REPORTS];
	gint reports_count;
	gint current_report;
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
	gfloat	(*rlib_get_string_width)(rlib *, char *);
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
void rlib_reset_variables_on_break(rlib *r, gchar *name);
void rlib_break_all_below_in_reverse_order(rlib *r, struct report_element *e);
void rlib_handle_break_footers(rlib *r);

/***** PROTOTYPES: formatstring.c *********************************************/
gint rlb_string_sprintf(char *dest, gchar *fmtstr, struct rlib_value *rval);
gint rlib_number_sprintf(char *dest, gchar *fmtstr, const struct rlib_value *rval, gint special_format);
gint rlib_format_string(rlib *r, struct report_field *rf, struct rlib_value *rval, gchar *buf);

/***** PROTOTYPES: fxp.c ******************************************************/
gint64 rlib_fxp_mul(gint64 a, gint64 b, gint64 factor);
gint64 rlib_fxp_div(gint64 num, gint64 denom, gint places);

/***** PROTOTYPES: api.c ******************************************************/
rlib * rlib_init(void);
rlib * rlib_init_with_environment(struct environment_filter *environment);
gint rlib_add_query_as(rlib *r, gchar *input_name, gchar *sql, gchar *name);
gint rlib_add_report(rlib *r, gchar *name, gchar *mainloop);
gint rlib_execute(rlib *r);
char * rlib_get_content_type_as_text(rlib *r);
gint rlib_spool(rlib *r);
gint rlib_set_output_format(rlib *r, gint format);
gint rlib_set_output_format_from_text(rlib *r, gchar * name);
gchar *rlib_get_output(rlib *r);
gint rlib_get_output_length(rlib *r);
gint rlib_mysql_report(gchar *hostname, gchar *username, gchar *password, gchar *database, gchar *xmlfilename, gchar *sqlquery, 
	gchar *outputformat);
gint rlib_postgre_report(gchar *connstr, gchar *xmlfilename, gchar *sqlquery, gchar *outputformat);
gint rlib_add_resultset_follower(rlib *r, gchar *leader, gchar *follower);
gint rlib_add_parameter(rlib *r, const gchar *name, const gchar *value);
gint rlib_set_locale(rlib *r, gchar *locale);
void rlib_init_profiler(void);
void rlib_dump_profile(gint profilenum, const gchar *filename);
void rlib_trap(void); //For internals debugging only
gchar *rlib_version(); // returns the version string.

/***** PROTOTYPES: parsexml.c *************************************************/
struct rlib_report * parse_report_file(gchar *filename);
struct report_output * report_output_new(gint type, gpointer data);

/***** PROTOTYPES: pcode.c ****************************************************/
struct rlib_value * rlib_execute_pcode(rlib *r, struct rlib_value *rval, struct rlib_pcode *code, struct rlib_value *this_field_value);
gint64 rlib_str_to_long_long(gchar *str);
struct rlib_pcode * rlib_infix_to_pcode(rlib *r, gchar *infix);
gint rvalcmp(struct rlib_value *v1, struct rlib_value *v2);
gint rlib_value_free(struct rlib_value *rval);
struct rlib_value * rlib_value_dup(struct rlib_value *orig);
struct rlib_value * rlib_value_dup_contents(struct rlib_value *rval);
struct rlib_value * rlib_value_new_error(struct rlib_value *rval);

/***** PROTOTYPES: reportgen.c ************************************************/
gchar *align_text(rlib *r, char *rtn, gint len, gchar *src, gint align, gint width);
gfloat get_page_width(rlib *r);
gint print_report_output(rlib *r, struct report_element *e, gint backwards);
gint will_outputs_fit(rlib *r, struct report_element *e, gint page);
gint will_this_fit(rlib *r, gfloat total, gint page);
gfloat get_output_size(rlib *r, struct report_output_array *roa);
void rlib_print_report_footer(rlib *r);
gint rlib_fetch_first_rows(rlib *r);
void rlib_init_variables(rlib *r);
gint rlib_end_page_if_line_wont_fit(rlib *r, struct report_element *e) ;
gfloat get_outputs_size(rlib *r, struct report_element *e, gint page);
void rlib_process_variables(rlib *r);
void rlib_init_page(rlib *r, gchar report_header);
gint make_report(rlib *r);
gint rlib_finalize(rlib *r);

/***** PROTOTYPES: resolution.c ***********************************************/
gint rlib_resolve_rlib_variable(rlib *r, gchar *name);
gchar * rlib_resolve_memory_variable(rlib *r, gchar *name);
gchar * rlib_resolve_field_value(rlib *r, struct rlib_resultset_field *rf);
gint rlib_lookup_result(rlib *r, gchar *name);
gint rlib_resolve_resultset_field(rlib *r, gchar *name, void **rtn_field, gint *rtn_resultset);
struct report_variable *rlib_resolve_variable(rlib *r, gchar *name);
void rlib_resolve_fields(rlib *r);

/***** PROTOTYPES: util.c *****************************************************/
gchar *strlwrexceptquoted (gchar *s);
gchar *rmwhitespacesexceptquoted(gchar *s);
void rlogit(const gchar *fmt, ...);
void r_debug(const gchar *fmt, ...);
void r_info(const gchar *fmt, ...);
void r_warning(const gchar *fmt, ...);
void r_error(const gchar *fmt, ...);
void rlogit_setmessagewriter(void(*writer)(const gchar *msg));
gint rutil_enableSignalHandler(gint trueorfalse);
gint64 tentothe(gint n);
gchar hextochar(gchar c);
gchar *colornames(gchar *str);
void parsecolor(struct rgb *color, gchar *strx);
struct rlib_datetime * stod(struct rlib_datetime *tm_date, gchar *str);
void bumpday(gint *year, gint *month, gint *day);
void bumpdaybackwords(gint *year, gint *month, gint *day);
gchar *strupr (gchar *s);
gchar *strlwr (gchar *s);
gchar *strproper (gchar *s);
gint daysinmonth(gint year, gint month);
void init_signals(void);
void make_more_space_if_necessary(gchar **str, gint *size, gint *total_size, gint len);
const char *encode(iconv_t cd, const char *txt); //do an iconv conversion

/***** PROTOTYPES: navigation.c ***********************************************/
gint rlib_navigate_next(rlib *r, gint resultset_num);
gint rlib_navigate_first(rlib *r, gint resultset_num);
gint rlib_navigate_previous(rlib *r, gint resultset_num);
gint rlib_navigate_last(rlib *r, gint resultset_num);

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
struct rlib_report * load_report(gchar *filename);


/* temp/test stuff */
void rlib_set_pdf_font(rlib *r, const char *encoding, const char *fontname);
void rlib_set_pdf_font_directories(rlib *r, const char *d1, const char *d2);
//void rlib_set_pdf_conversion(rlib *r, int rptnum, const char *encoding);

/* rlib_datetime - should be a separate module (class) */
void rlib_datetime_clear(struct rlib_datetime *t1);
void rlib_datetime_set_date(struct rlib_datetime *dt, int y, int m, int d);
void rlib_datetime_set_time(struct rlib_datetime *dt, int h, int m, int s);
int rlib_datetime_valid_date(struct rlib_datetime *dt);
int rlib_datetime_valid_time(struct rlib_datetime *dt);
void rlib_datetime_clear_time(struct rlib_datetime *t);
void rlib_datetime_clear_date(struct rlib_datetime *t);
gint rlib_datetime_compare(struct rlib_datetime *t1, struct rlib_datetime *t2);
void rlib_datetime_format(struct rlib_datetime *dt, char *buf, int max, const char *fmt);
int rlib_datetime_daysdiff(struct rlib_datetime *dt, struct rlib_datetime *dt2);

//#define charcount(s) g_utf8_strlen(s, -1)
#define charcount(s) strlen(s)
#define bytelength(s) strlen(s)
