/*
 *  Copyright (C) 2003-2005 SICOM Systems, INC.
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
#include <stdio.h>
#include <string.h>

#include <libxml/xmlversion.h>
#include <libxml/xmlmemory.h>
#include <libxml/xinclude.h>

#include <glib.h>

#include "config.h"
#include "rlib.h"

void dump_part(struct rlib_part *part);

iconv_t cd = (void *)-1;

static struct rlib_report * parse_report_file(rlib *r, gchar *filename, gchar *query);

void safestrncpy(gchar *dest, gchar *str, int n) {
	if (!dest) return;
	*dest = '\0';
	if (str) g_strlcpy(dest, str, n);
}


#if DISABLE_UTF8
static void utf8_to_8813(gchar *dest, gchar *str) {
	size_t len = MAXSTRLEN;
	size_t slen;
	gchar *olddest = dest;
	if(str != NULL && str[0] != 0) {
		if(cd != NULL && cd != (void *)-1) {
			slen = strlen(str);
			memset(dest, 0, MAXSTRLEN);
#if ICONV_CONST_CHAR_PP
			iconv(cd, (const char **) &str, &slen, &olddest, &len);
#else
			iconv(cd, (char **)&str, &slen, &olddest, &len);
#endif
		} else {
			strcpy(dest, str);
		}
	} else {
		dest[0] = 0;
	}
}
#endif

static int ignoreElement(const char *elname) {
	int result = FALSE;
	if (!xmlStrcmp(elname, "comment") 
		|| !xmlStrcmp(elname, "text") 
		|| !xmlStrcmp(elname, "include")) {
		result = TRUE;
	}
	return result;
}

static struct rlib_report_image * parse_image(xmlNodePtr cur) {
	struct rlib_report_image *ri = g_new0(struct rlib_report_image, 1);
	ri->xml_value = xmlGetProp(cur, (const xmlChar *) "value");
	ri->xml_type = xmlGetProp(cur, (const xmlChar *) "type");
	ri->xml_width = xmlGetProp(cur, (const xmlChar *) "width");
	ri->xml_height = xmlGetProp(cur, (const xmlChar *) "height");
	return ri;
}

static struct rlib_element * parse_line_array(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_element *e, *current;
	e = NULL;
	
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		current = NULL;
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "field"))) {
			struct rlib_report_field *f = g_new0(struct rlib_report_field, 1);
			current = (void *)g_new0(struct rlib_element, 1);
#if DISABLE_UTF8
			utf8_to_8813(f->value, xmlGetProp(cur, (const xmlChar *) "value"));
#else
			safestrncpy(f->value, xmlGetProp(cur, (const xmlChar *) "value"), sizeof(f->value));
#endif

//Nevermind			//TODO: we need to utf to 8813 all string values in single quotes
			strcpy(f->value, xmlGetProp(cur, (const xmlChar *) "value"));
			f->xml_align = xmlGetProp(cur, (const xmlChar *) "align");
			f->xml_bgcolor = xmlGetProp(cur, (const xmlChar *) "bgcolor");
			f->xml_color = xmlGetProp(cur, (const xmlChar *) "color");
			f->xml_width = xmlGetProp(cur, (const xmlChar *) "width");
			f->xml_bold = xmlGetProp(cur, (const xmlChar *) "bold");
			f->xml_italics = xmlGetProp(cur, (const xmlChar *) "italics");
			f->xml_format = xmlGetProp(cur, (const xmlChar *) "format");
			f->xml_link = xmlGetProp(cur, (const xmlChar *) "link");
			f->xml_col = xmlGetProp(cur, (const xmlChar *) "col");
			f->xml_memo = xmlGetProp(cur, (const xmlChar *) "memo");
			f->xml_memo_height = xmlGetProp(cur, (const xmlChar *) "memo_height");
			f->xml_memo_wrap_chars = xmlGetProp(cur, (const xmlChar *) "memo_wrap_chars");
			current->data = f;
			current->type = RLIB_ELEMENT_FIELD;
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "literal"))) {
			struct rlib_report_literal *t = g_new0(struct rlib_report_literal, 1);
			current = g_new0(struct rlib_element, 1);
#if DISABLE_UTF8
			utf8_to_8813(t->value, xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
#else
			safestrncpy(t->value, xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), sizeof(t->value));
#endif
			t->xml_align = xmlGetProp(cur, (const xmlChar *) "align");
			t->xml_bgcolor = xmlGetProp(cur, (const xmlChar *) "bgcolor");
			t->xml_color = xmlGetProp(cur, (const xmlChar *) "color");
			t->xml_width = xmlGetProp(cur, (const xmlChar *) "width");
			t->xml_bold = xmlGetProp(cur, (const xmlChar *) "bold");
			t->xml_italics = xmlGetProp(cur, (const xmlChar *) "italics");
			t->xml_col = xmlGetProp(cur, (const xmlChar *) "col");
			current->data = t;
			current->type = RLIB_ELEMENT_LITERAL;
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "Image"))) {
			struct rlib_report_image *ri = parse_image(cur);
			current = (void *)g_new0(struct rlib_element, 1);
			current->data = ri;
			current->type = RLIB_ELEMENT_IMAGE;
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			r_error("Unknown element [%s] in <Line>\n", cur->name);
		}
		if (current != NULL) {
			if(e == NULL) {
				e = current;
			} else {
				struct rlib_element *xxx;
				xxx = e;
				while(xxx->next != NULL) xxx =  xxx->next;
				xxx->next = current;
			}
		}
		cur = cur->next;
	}

	return e;
}

struct rlib_report_output * report_output_new(gint type, gpointer data) {
	struct rlib_report_output *ro = g_malloc(sizeof(struct rlib_report_output));
	ro->type = type;
	ro->data = data;
	return ro;
}

static struct rlib_element * parse_report_output(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_element *e = g_malloc(sizeof(struct rlib_report));
	struct rlib_report_output_array *roa = g_new0(struct rlib_report_output_array, 1);
	roa->count = 0;
	roa->data = NULL;
	e->next = NULL;
	e->data = roa;
	roa->xml_page = NULL;
	if(cur != NULL && (!xmlStrcmp(cur->name, (const xmlChar *) "Output"))) {
		roa->xml_page = xmlGetProp(cur, (const xmlChar *) "page");
		cur = cur->xmlChildrenNode;
	}	
	while (cur != NULL) {      
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "Line"))) {
			struct rlib_report_lines *rl = g_new0(struct rlib_report_lines, 1);
			rl->xml_bgcolor = xmlGetProp(cur, (const xmlChar *) "bgcolor");
			rl->xml_color = xmlGetProp(cur, (const xmlChar *) "color");
			rl->xml_bold = xmlGetProp(cur, (const xmlChar *) "bold");
			rl->xml_italics = xmlGetProp(cur, (const xmlChar *) "italics");
			rl->xml_font_size = xmlGetProp(cur, (const xmlChar *) "fontSize");
			rl->xml_suppress = xmlGetProp(cur, (const xmlChar *) "suppress");
			if(rl->xml_font_size == NULL)
				rl->font_point = -1;
			else
				rl->font_point = atoi(rl->xml_font_size);
				
			rl->e = parse_line_array(doc, ns, cur);
			roa->data = g_realloc(roa->data, sizeof(struct rlib_report_output_array *) * (roa->count + 1));
			roa->data[roa->count++] = report_output_new(RLIB_REPORT_PRESENTATION_DATA_LINE, rl);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "HorizontalLine"))) {
			struct rlib_report_horizontal_line *rhl = g_new0(struct rlib_report_horizontal_line, 1);
			rhl->xml_bgcolor = xmlGetProp(cur, (const xmlChar *) "bgcolor");
			rhl->xml_size = xmlGetProp(cur, (const xmlChar *) "size");
			rhl->xml_indent = xmlGetProp(cur, (const xmlChar *) "indent");
			rhl->xml_length = xmlGetProp(cur, (const xmlChar *) "length");
			rhl->xml_font_size = xmlGetProp(cur, (const xmlChar *) "fontSize");
			rhl->xml_suppress = xmlGetProp(cur, (const xmlChar *) "suppress");
			if(rhl->xml_font_size == NULL)
				rhl->font_point = -1;
			else
				rhl->font_point = atoi(rhl->xml_font_size);
			roa->data = g_realloc(roa->data, sizeof(struct rlib_report_output_array *) * (roa->count + 1));
			roa->data[roa->count++] = report_output_new(RLIB_REPORT_PRESENTATION_DATA_HR, rhl);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "Image"))) {
			struct rlib_report_image *ri = parse_image(cur);
			roa->data = g_realloc(roa->data, sizeof(struct rlib_report_output_array *) * (roa->count + 1));
			roa->data[roa->count++] = report_output_new(RLIB_REPORT_PRESENTATION_DATA_IMAGE, ri);
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			r_error("Unknown element [%s] in <Output>. Expected: Line, HorizontalLine or Image\n", cur->name);
		}
		cur = cur->next;
	}	
	return e;
}

static struct rlib_element * parse_report_outputs(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_element *e = NULL;

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {      
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "Output"))) {
			if(e == NULL) {
				e = parse_report_output(doc, ns, cur);
			} else {
				struct rlib_element *xxx = e;
				for(;xxx->next != NULL; xxx=xxx->next) {};
				xxx->next = parse_report_output(doc, ns, cur);				
			}
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			r_error("Invalid element :<%s>. Expected: <Output>\n", cur->name);
		}
		cur = cur->next;
	}	
	return e;
}

static struct rlib_element * parse_break_field(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_element *e = g_malloc(sizeof(struct rlib_element));
	struct rlib_break_fields *bf = g_new0(struct rlib_break_fields, 1);
	e->next = NULL;
	e->data = bf;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "BreakField"))) {
			bf->rval = NULL;
			bf->xml_value = xmlGetProp(cur, (const xmlChar *) "value");
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			r_error("Unknown element [%s] in <Break>. Expected BreakField\n", cur->name);
		}
		cur = cur->next;
	}
	return e;
}

static struct rlib_element * parse_report_break(struct rlib_report *report, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_element *e = g_malloc(sizeof(struct rlib_element));
	struct rlib_report_break *rb = g_new0(struct rlib_report_break, 1);
	e->next = NULL;
	e->data = rb;
	rb->xml_name = xmlGetProp(cur, (const xmlChar *) "name");
	rb->xml_newpage = xmlGetProp(cur, (const xmlChar *) "newpage");
	rb->xml_headernewpage = xmlGetProp(cur, (const xmlChar *) "headernewpage");
	rb->xml_suppressblank = xmlGetProp(cur, (const xmlChar *) "suppressblank");
	cur = cur->xmlChildrenNode;
	rb->fields = NULL;
	rb->header = NULL;
	rb->footer = NULL;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "BreakHeader"))) {
			rb->header = parse_report_outputs(doc, ns, cur);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "BreakFooter"))) {
			rb->footer = parse_report_outputs(doc, ns, cur);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "BreakFields"))) {
			if(rb->fields == NULL)
				rb->fields = parse_break_field(doc, ns, cur);
			else {
				struct rlib_element *xxx = rb->fields;
				for(;xxx->next != NULL; xxx=xxx->next) {};
				xxx->next = parse_report_break(report, doc, ns, cur);							
			}
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			r_error("Unknown element [%s] in <ReportBreak>. Expected BreakHeader, BreakFooter, BreakFields\n", cur->name);
		}
		cur = cur->next;
	}
	
	return e;
}

static struct rlib_element * parse_report_breaks(struct rlib_report *report, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_element *e = NULL;

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {      
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "Break"))) {
			if(e == NULL) {
				e = parse_report_break(report, doc, ns, cur);
			} else {
				struct rlib_element *xxx = e;
				for(;xxx->next != NULL; xxx=xxx->next) {};
				xxx->next = parse_report_break(report, doc, ns, cur);				
			}
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			r_error("Unknown element [%s] in <Breaks>. Expected Break.\n", cur->name);
		}
		cur = cur->next;
	}	
	return e;
}

static void parse_detail(struct rlib_report *report, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur, struct rlib_report_detail *r) {
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "FieldHeaders"))) {
			r->headers = parse_report_outputs(doc, ns, cur);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "FieldDetails"))) {
			r->fields = parse_report_outputs(doc, ns, cur);
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			r_error("Unknown element [%s] in <Detail>. Expected FieldHeaders or FieldDetails\n", cur->name);
		}
		cur = cur->next;
	}

}

static void parse_alternate(struct rlib_report *report, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur, struct rlib_report_alternate *ra) {
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "NoData"))) {
			ra->nodata = parse_report_outputs(doc, ns, cur);
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			r_error("Unknown element [%s] in <Alternate>. Expected NoData\n", cur->name);
		}
		cur = cur->next;
	}

}

static struct rlib_graph_plot * parse_graph_plots(struct rlib_report *report, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_graph_plot *gp = g_new0(struct rlib_graph_plot, 1);
	gp->xml_axis = xmlGetProp(cur, (const xmlChar *) "axis");
	gp->xml_field = xmlGetProp(cur, (const xmlChar *) "field");
	gp->xml_label = xmlGetProp(cur, (const xmlChar *) "label");
	gp->xml_side = xmlGetProp(cur, (const xmlChar *) "side");
	return gp;
}

static void parse_graph(struct rlib_report *report, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur, struct rlib_graph *graph) {
	graph->xml_type = xmlGetProp(cur, (const xmlChar *) "type");
	graph->xml_subtype = xmlGetProp(cur, (const xmlChar *) "subtype");
	graph->xml_width = xmlGetProp(cur, (const xmlChar *) "width");
	graph->xml_height = xmlGetProp(cur, (const xmlChar *) "height");
	graph->xml_title = xmlGetProp(cur, (const xmlChar *) "title");
	graph->xml_title = xmlGetProp(cur, (const xmlChar *) "title");
	graph->xml_x_axis_title = xmlGetProp(cur, (const xmlChar *) "x_axis_title");
	graph->xml_y_axis_title = xmlGetProp(cur, (const xmlChar *) "y_axis_title");
	graph->xml_y_axis_title_right = xmlGetProp(cur, (const xmlChar *) "y_axis_title_right");
	graph->plots = NULL;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "Plot"))) {
			graph->plots = g_slist_append(graph->plots, parse_graph_plots(report, doc, ns, cur));
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			r_error("Unknown element [%s] in <Graph>. Expected Plot\n", cur->name);
		}
		cur = cur->next;
	}
}

static struct rlib_element * parse_report_variable(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_element *e = g_malloc(sizeof(struct rlib_report));
	struct rlib_report_variable *rv = g_new0(struct rlib_report_variable, 1);
	e->next = NULL;
	e->data = rv;
	rv->xml_name = xmlGetProp(cur, (const xmlChar *) "name");
	rv->xml_str_type = xmlGetProp(cur, (const xmlChar *) "type");
	rv->xml_value = xmlGetProp(cur, (const xmlChar *) "value");
	rv->xml_resetonbreak = xmlGetProp(cur, (const xmlChar *) "resetonbreak");
	rv->type = RLIB_REPORT_VARIABLE_UNDEFINED;
	if(rv->xml_str_type != NULL && rv->xml_str_type[0] != '\0') {
		if(!strcmp(rv->xml_str_type, "expression") || !strcmp(rv->xml_str_type, "static"))
			rv->type = RLIB_REPORT_VARIABLE_EXPRESSION;
		else if(!strcmp(rv->xml_str_type, "count"))
			rv->type = RLIB_REPORT_VARIABLE_COUNT;
		else if(!strcmp(rv->xml_str_type, "sum"))
			rv->type = RLIB_REPORT_VARIABLE_SUM;
		else if(!strcmp(rv->xml_str_type, "average"))
			rv->type = RLIB_REPORT_VARIABLE_AVERAGE;
		else if(!strcmp(rv->xml_str_type, "lowest"))
			rv->type = RLIB_REPORT_VARIABLE_LOWEST;
		else if(!strcmp(rv->xml_str_type, "highest"))
			rv->type = RLIB_REPORT_VARIABLE_HIGHEST;
		else
			r_error("Unknown report variable type [%s] in <Variable>\n", rv->xml_str_type);
	}

	return e;
}

static struct rlib_element * parse_report_variables(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_element *e = NULL;

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {      
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "Variable"))) {
			if(e == NULL) {
				e = parse_report_variable(doc, ns, cur);
			} else {
				struct rlib_element *xxx = e;
				for(;xxx->next != NULL; xxx=xxx->next) {};
				xxx->next = parse_report_variable(doc, ns, cur);				
			}
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			r_error("Unknown element [%s] in <Variables>. Expected Variable.\n", cur->name);
		}
		cur = cur->next;
	}	
	return e;
}

static void parse_metadata(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur, GHashTable *ht) {
	struct rlib_metadata *metadata = g_new0(struct rlib_metadata, 1);
	gchar *name;

	name = xmlGetProp(cur, (const xmlChar *) "name");
	metadata->xml_formula = xmlGetProp(cur, (const xmlChar *) "value");
	if(name != NULL) 
		g_hash_table_insert(ht, g_strdup(name), metadata);
	else
		g_free(metadata);	
	return;
}

static void parse_metadata_item(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur, GHashTable *ht) {
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {      
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "MetaData"))) {
			parse_metadata(doc, ns, cur, ht);
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			r_error("Unknown element [%s] in <MetaData>. Expected MetaData.\n", cur->name);
		}
		cur = cur->next;
	}	
	return;
}

static void parse_report(rlib *r, struct rlib_part *part, struct rlib_report *report, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur, gchar *query) {
	report->doc = doc;
	report->contents = NULL;
	if (doc->encoding) 
		g_strlcpy(report->xml_encoding_name, doc->encoding, sizeof(report->xml_encoding_name));

	while (cur && xmlIsBlankNode (cur)) 
		cur = cur -> next;

	if(cur == 0)
		return;

	report->xml_font_size = xmlGetProp(cur, (const xmlChar *) "fontSize");
	if(query == NULL)
		report->xml_query = xmlGetProp(cur, (const xmlChar *) "query");
	else
		report->xml_query = query;
		
	report->xml_orientation = xmlGetProp(cur, (const xmlChar *) "orientation");
	report->xml_top_margin = xmlGetProp(cur, (const xmlChar *) "topMargin");
	report->xml_left_margin = xmlGetProp(cur, (const xmlChar *) "leftMargin");
	report->xml_bottom_margin = xmlGetProp(cur, (const xmlChar *) "bottomMargin");
	report->xml_height = xmlGetProp(cur, (const xmlChar *) "height");
	report->xml_iterations = xmlGetProp(cur, (const xmlChar *) "iterations");

	if(xmlGetProp(cur, (const xmlChar *) "paperType") != NULL && part->xml_paper_type == NULL)
		part->xml_paper_type = xmlGetProp(cur, (const xmlChar *) "paperType");
	report->xml_pages_across = xmlGetProp(cur, (const xmlChar *) "pagesAcross");
	report->xml_suppress_page_header_first_page = xmlGetProp(cur, (const xmlChar *) "suppressPageHeaderFirstPage");
	
	cur = cur->xmlChildrenNode;
	report->breaks = NULL;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "ReportHeader"))) 
			report->report_header = parse_report_outputs(doc, ns, cur);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "PageHeader"))) 
			report->page_header = parse_report_outputs(doc, ns, cur);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "PageFooter"))) 
			report->page_footer = parse_report_outputs(doc, ns, cur);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "ReportFooter"))) 
			report->report_footer = parse_report_outputs(doc, ns, cur);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "Detail"))) 
			parse_detail(report, doc, ns, cur, &report->detail);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "Alternate"))) 
			parse_alternate(report, doc, ns, cur, &report->alternate);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "Graph"))) 
			parse_graph(report, doc, ns, cur, &report->graph);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "Breaks"))) 
			report->breaks = parse_report_breaks(report, doc, ns, cur);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "Variables"))) 
			report->variables = parse_report_variables(doc, ns, cur);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "MetaData"))) 
			parse_metadata_item(doc, ns, cur, r->input_metadata);
		else if (!ignoreElement(cur->name)) //must be last
			/* ignore comments, etc */
			r_error("Unknown element [%s] in <Report>\n", cur->name);
		cur = cur->next;
	}
}

static struct rlib_report * parse_part_load(rlib *r, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_report *report;
	gchar *name, *query;
	name =  xmlGetProp(cur, (const xmlChar *) "name");
	query =  xmlGetProp(cur, (const xmlChar *) "query");
	report = parse_report_file(r, name, query);
	return report;
}

static struct rlib_part_td * parse_part_td(rlib *r, struct rlib_part *part, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_part_td *td = g_new0(struct rlib_part_td, 1);	
	td->xml_width =  xmlGetProp(cur, (const xmlChar *) "width");
	td->xml_height =  xmlGetProp(cur, (const xmlChar *) "height");
	td->xml_border_width =  xmlGetProp(cur, (const xmlChar *) "border_width");
	td->xml_border_color =  xmlGetProp(cur, (const xmlChar *) "border_color");
	td->reports = NULL;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {      
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "load"))) {
			td->reports = g_slist_append(td->reports, parse_part_load(r, doc, ns, cur));
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "Report"))) {
			struct rlib_report *report;
			report = (struct rlib_report *) g_new0(struct rlib_report, 1);
			parse_report(r, part, report, doc, ns, cur, NULL);
			td->reports = g_slist_append(td->reports, report);
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			r_error("Unknown element [%s] in <tr>. Expected td.\n", cur->name);
		}
		cur = cur->next;
	}	
	return td;
}

static struct rlib_part_tr * parse_part_tr(rlib *r, struct rlib_part *part, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_part_tr *tr = g_new0(struct rlib_part_tr, 1);
	tr->xml_layout = xmlGetProp(cur, (const xmlChar *) "layout");
	tr->xml_newpage = xmlGetProp(cur, (const xmlChar *) "newpage");
	tr->part_deviations = NULL;		
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {      
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "pd"))) {
			tr->part_deviations = g_slist_append(tr->part_deviations, parse_part_td(r, part, doc, ns, cur));
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			r_error("Unknown element [%s] in <tr>. Expected td.\n", cur->name);
		}
		cur = cur->next;
	}	
	return tr;
}

static void parse_part(rlib *r, struct rlib_part *part, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	while (cur && xmlIsBlankNode (cur)) 
		cur = cur -> next;

	if(cur == 0)
		return;

	part->xml_name = xmlGetProp(cur, (const xmlChar *) "name");
	part->xml_pages_across = xmlGetProp(cur, (const xmlChar *) "pages_across");
	part->xml_font_size = xmlGetProp(cur, (const xmlChar *) "fontSize");
	part->xml_orientation = xmlGetProp(cur, (const xmlChar *) "orientation");
	part->xml_top_margin = xmlGetProp(cur, (const xmlChar *) "top_margin");
	part->xml_left_margin = xmlGetProp(cur, (const xmlChar *) "left_margin");
	part->xml_bottom_margin = xmlGetProp(cur, (const xmlChar *) "bottom_margin");
	part->xml_paper_type = xmlGetProp(cur, (const xmlChar *) "paper_type");
	part->xml_iterations = xmlGetProp(cur, (const xmlChar *) "iterations");
	part->part_rows = NULL;
	
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "pr"))) {
			part->part_rows = g_slist_append(part->part_rows, parse_part_tr(r, part, doc, ns, cur));
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "PageHeader"))) {
			part->page_header = parse_report_outputs(doc, ns, cur);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "ReportHeader"))) {
			part->report_header = parse_report_outputs(doc, ns, cur);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "PageFooter"))) {
			part->page_footer = parse_report_outputs(doc, ns, cur);
		} else if (!ignoreElement(cur->name)) //must be last
			/* ignore comments, etc */
			r_error("Unknown element [%s] in <Part>\n", cur->name);
		cur = cur->next;
	}
}

void dump_part_td(gpointer data, gpointer user_data) {
}

void dump_part_tr(gpointer data, gpointer user_data) {
	struct rlib_part_tr *tr_data = data;
	g_slist_foreach(tr_data->part_deviations, dump_part_td, NULL);
}

void dump_part(struct rlib_part *part) {
	g_slist_foreach(part->part_rows, dump_part_tr, NULL);
}

struct rlib_part * parse_part_file(rlib *r, gchar *filename, gchar type) {
	xmlDocPtr doc;
	struct rlib_report *report;
	struct rlib_part *part = NULL;
	xmlNsPtr ns = NULL;
	xmlNodePtr cur;
	int found = FALSE;

#if DISABLE_UTF8
	cd = iconv_open(ICONV_ISO, "UTF-8");
//	cd = (void *)-1;
#endif

	if(type == RLIB_REPORT_TYPE_BUFFER) 
		doc = xmlReadMemory(filename, strlen(filename), NULL, NULL, XML_PARSE_XINCLUDE);
	else
		doc = xmlReadFile(filename, NULL, XML_PARSE_XINCLUDE);
		
	xmlXIncludeProcess(doc);
	
	if (doc == NULL)  {
		r_error("xmlParseError \n");
		return(NULL);
	}
  
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		r_error("xmlParseError \n");
		xmlFreeDoc(doc);
		return(NULL);
	}
	
	report = (struct rlib_report *) g_new0(struct rlib_report, 1);
	if(report == NULL) {
		r_error("Out of Memory :(\n");
		xmlFreeDoc(doc);
		return(NULL);
	}

	part = (struct rlib_part *) g_new0(struct rlib_part, 1);
	if(part == NULL) {
		r_error("Out of Memory :(\n");
		xmlFreeDoc(doc);
		return(NULL);
	}
	
	if((xmlStrcmp(cur->name, (const xmlChar *) "Part"))==0) {
		parse_part(r, part, doc, ns, cur);
		found = TRUE;
	}

/*
	If a report appears by it's self put it in a table w/ 1 row and 1 col 100%
*/	
	if((xmlStrcmp(cur->name, (const xmlChar *) "Report"))==0) {
		struct rlib_part_tr *tr= g_new0(struct rlib_part_tr, 1);
		struct rlib_part_td *td= g_new0(struct rlib_part_td, 1);
		
		part->part_rows = NULL;
		part->part_rows = g_slist_append(part->part_rows, tr);
		tr->part_deviations = NULL;
		tr->part_deviations = g_slist_append(tr->part_deviations, td);
		td->reports = NULL;
		td->reports = g_slist_append(td->reports, report);
		parse_report(r, part, report, doc, ns, cur, NULL);
		part->page_header = report->page_header;
		part->report_header = report->report_header;
		part->page_footer = report->page_footer;
		report->page_header = NULL;
		report->report_header = NULL;
		report->page_footer = NULL;
		
		part->xml_left_margin = report->xml_left_margin;
		part->xml_top_margin = report->xml_top_margin;
		part->xml_bottom_margin = report->xml_bottom_margin;
		part->xml_font_size = report->xml_font_size;
		part->xml_orientation = report->xml_orientation;
		report->is_the_only_report = TRUE;		
		part->has_only_one_report = TRUE;
		report->xml_left_margin = NULL;
		report->xml_top_margin = NULL;
		report->xml_bottom_margin = NULL;
		
		
		part->xml_pages_across = report->xml_pages_across;
		found = TRUE;
	}
	
	if(!found) {
		r_error("document of the wrong type, was '%s', Report or Part expected", cur->name);
		r_error("xmlDocDump follows\n");
		xmlDocDump ( stderr, doc );
		xmlFreeDoc(doc);
		g_free(report);
		g_free(part);
		return(NULL);
	}

	
#if DISABLE_UTF8
	if((long)cd != -1)
		iconv_close(cd);
#endif	

	return part;
}

static struct rlib_report * parse_report_file(rlib *r, gchar *filename, gchar *query) {
	xmlDocPtr doc;
	struct rlib_report *report;
	xmlNsPtr ns = NULL;
	xmlNodePtr cur;
	int found = FALSE;

	doc = xmlReadFile(filename, NULL, XML_PARSE_XINCLUDE);
	xmlXIncludeProcess(doc);
	
	if (doc == NULL)  {
		r_error("xmlParseError \n");
		return(NULL);
	}
  
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		r_error("xmlParseError \n");
		xmlFreeDoc(doc);
		return(NULL);
	}
	
	report = (struct rlib_report *) g_new0(struct rlib_report, 1);
	if(report == NULL) {
		r_error("Out of Memory :(\n");
		xmlFreeDoc(doc);
		return(NULL);
	}

	
	if((xmlStrcmp(cur->name, (const xmlChar *) "Report"))==0) {
		parse_report(r, NULL, report, doc, ns, cur, query);
		found = TRUE;
	}
	
	if(!found) {
		r_error("document of the wrong type, was '%s', Report", cur->name);
		r_error("xmlDocDump follows\n");
		xmlDocDump ( stderr, doc );
		xmlFreeDoc(doc);
		g_free(report);
		return(NULL);
	}

	
	return report;
}
