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

static struct rlib_report * parse_report_file(gchar *filename);

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

static struct rlib_element * parse_line_array(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_element *e, *current;
	e = NULL;
	
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		current = NULL;
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "field"))) {
			struct rlib_report_field *f = g_new0(struct rlib_report_field, 1);
			current = (void *)g_new0(struct rlib_element, 1);
			utf8_to_8813(f->value, xmlGetProp(cur, (const xmlChar *) "value"));
//Nevermind			//TODO: we need to utf to 8813 all string values in single quotes
			strcpy(f->value, xmlGetProp(cur, (const xmlChar *) "value"));
			f->xml_align = xmlGetProp(cur, (const xmlChar *) "align");
			f->bgcolor = xmlGetProp(cur, (const xmlChar *) "bgcolor");
			f->color = xmlGetProp(cur, (const xmlChar *) "color");
			f->xml_width = xmlGetProp(cur, (const xmlChar *) "width");
			f->format = xmlGetProp(cur, (const xmlChar *) "format");
			f->link = xmlGetProp(cur, (const xmlChar *) "link");
			f->col = xmlGetProp(cur, (const xmlChar *) "col");
			f->xml_wrapchars = xmlGetProp(cur, (const xmlChar *) "wrapchars");
			f->xml_maxlines = xmlGetProp(cur, (const xmlChar *) "maxlines");
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
			t->bgcolor = xmlGetProp(cur, (const xmlChar *) "bgcolor");
			t->color = xmlGetProp(cur, (const xmlChar *) "color");
			t->xml_width = xmlGetProp(cur, (const xmlChar *) "width");
			t->col = xmlGetProp(cur, (const xmlChar *) "col");
			current->data = t;
			current->type = RLIB_ELEMENT_LITERAL;
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			rlogit("Unknown element [%s] in <Line>\n", cur->name);
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
			rl->bgcolor = xmlGetProp(cur, (const xmlChar *) "bgcolor");
			rl->color = xmlGetProp(cur, (const xmlChar *) "color");
			rl->font_size = xmlGetProp(cur, (const xmlChar *) "fontSize");
			rl->suppress = xmlGetProp(cur, (const xmlChar *) "suppress");
			if(rl->font_size == NULL)
				rl->font_point = -1;
			else
				rl->font_point = atoi(rl->font_size);
				
			rl->e = parse_line_array(doc, ns, cur);
			roa->data = g_realloc(roa->data, sizeof(struct rlib_report_output_array *) * (roa->count + 1));
			roa->data[roa->count++] = report_output_new(REPORT_PRESENTATION_DATA_LINE, rl);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "HorizontalLine"))) {
			struct rlib_report_horizontal_line *rhl = g_new0(struct rlib_report_horizontal_line, 1);
			rhl->bgcolor = xmlGetProp(cur, (const xmlChar *) "bgcolor");
			rhl->size = xmlGetProp(cur, (const xmlChar *) "size");
			rhl->indent = xmlGetProp(cur, (const xmlChar *) "indent");
			rhl->length = xmlGetProp(cur, (const xmlChar *) "length");
			rhl->font_size = xmlGetProp(cur, (const xmlChar *) "fontSize");
			rhl->suppress = xmlGetProp(cur, (const xmlChar *) "suppress");
			if(rhl->font_size == NULL)
				rhl->font_point = -1;
			else
				rhl->font_point = atoi(rhl->font_size);
			roa->data = g_realloc(roa->data, sizeof(struct rlib_report_output_array *) * (roa->count + 1));
			roa->data[roa->count++] = report_output_new(REPORT_PRESENTATION_DATA_HR, rhl);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "Image"))) {
			struct rlib_report_image *ri = g_new0(struct rlib_report_image, 1);
			ri->value = xmlGetProp(cur, (const xmlChar *) "value");
			ri->type = xmlGetProp(cur, (const xmlChar *) "type");
			ri->width = xmlGetProp(cur, (const xmlChar *) "width");
			ri->height = xmlGetProp(cur, (const xmlChar *) "height");
			roa->data = g_realloc(roa->data, sizeof(struct rlib_report_output_array *) * (roa->count + 1));
			roa->data[roa->count++] = report_output_new(REPORT_PRESENTATION_DATA_IMAGE, ri);
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			rlogit("Unknown element [%s] in <Output>. Expected: Line, HorizontalLine or Image\n", cur->name);
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
			rlogit("Invalid element :<%s>. Expected: <Output>\n", cur->name);
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
			bf->value = xmlGetProp(cur, (const xmlChar *) "value");
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			rlogit("Unknown element [%s] in <Break>. Expected BreakField\n", cur->name);
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
	rb->name = xmlGetProp(cur, (const xmlChar *) "name");
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
			rlogit("Unknown element [%s] in <ReportBreak>. Expected BreakHeader, BreakFooter, BreakFields\n", cur->name);
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
			rlogit("Unknown element [%s] in <Breaks>. Expected Break.\n", cur->name);
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
			rlogit("Unknown element [%s] in <Detail>. Expected FieldHeaders or FieldDetails\n", cur->name);
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
			rlogit("Unknown element [%s] in <Alternate>. Expected NoData\n", cur->name);
		}
		cur = cur->next;
	}

}

static struct rlib_element * parse_report_variable(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_element *e = g_malloc(sizeof(struct rlib_report));
	struct rlib_report_variable *rv = g_new0(struct rlib_report_variable, 1);
	e->next = NULL;
	e->data = rv;
	rv->name = xmlGetProp(cur, (const xmlChar *) "name");
	rv->str_type = xmlGetProp(cur, (const xmlChar *) "type");
	rv->value = xmlGetProp(cur, (const xmlChar *) "value");
	rv->resetonbreak = xmlGetProp(cur, (const xmlChar *) "resetonbreak");
	rv->type = REPORT_VARIABLE_UNDEFINED;
	if(rv->str_type != NULL && rv->str_type[0] != '\0') {
		if(!strcmp(rv->str_type, "expression") || !strcmp(rv->str_type, "static"))
			rv->type = REPORT_VARIABLE_EXPRESSION;
		else if(!strcmp(rv->str_type, "count"))
			rv->type = REPORT_VARIABLE_COUNT;
		else if(!strcmp(rv->str_type, "sum"))
			rv->type = REPORT_VARIABLE_SUM;
		else if(!strcmp(rv->str_type, "average"))
			rv->type = REPORT_VARIABLE_AVERAGE;
		else if(!strcmp(rv->str_type, "lowest"))
			rv->type = REPORT_VARIABLE_LOWEST;
		else if(!strcmp(rv->str_type, "highest"))
			rv->type = REPORT_VARIABLE_HIGHEST;
		else
			rlogit("Unknown report variable type [%s] in <Variable>\n", rv->str_type);
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
			rlogit("Unknown element [%s] in <Variables>. Expected Variable.\n", cur->name);
		}
		cur = cur->next;
	}	
	return e;
}

static void parse_report(struct rlib_part *part, struct rlib_report *report, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	report->doc = doc;
	report->contents = NULL;
	if (doc->encoding) g_strlcpy(report->xml_encoding_name, doc->encoding, sizeof(report->xml_encoding_name));
#if DISABLE_UTF8
	cd = iconv_open(ICONV_ISO, "UTF-8");
	cd = (void *)-1;
#endif
	while (cur && xmlIsBlankNode (cur)) 
		cur = cur -> next;

	if(cur == 0)
		return;

	report->xml_font_size = xmlGetProp(cur, (const xmlChar *) "fontSize");
	report->xml_query = xmlGetProp(cur, (const xmlChar *) "query");
	report->xml_orientation = xmlGetProp(cur, (const xmlChar *) "orientation");
	report->xml_top_margin = xmlGetProp(cur, (const xmlChar *) "topMargin");
	report->xml_left_margin = xmlGetProp(cur, (const xmlChar *) "leftMargin");
	report->xml_bottom_margin = xmlGetProp(cur, (const xmlChar *) "bottomMargin");
	report->xml_height = xmlGetProp(cur, (const xmlChar *) "height");

	if(xmlGetProp(cur, (const xmlChar *) "paperType") != NULL)
		part->xml_paper_type = xmlGetProp(cur, (const xmlChar *) "paperType");
	report->xml_pages_accross = xmlGetProp(cur, (const xmlChar *) "pagesAcross");
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
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "Breaks"))) 
			report->breaks = parse_report_breaks(report, doc, ns, cur);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "Variables"))) 
			report->variables = parse_report_variables(doc, ns, cur);
		else if (!ignoreElement(cur->name)) //must be last
			/* ignore comments, etc */
			rlogit("Unknown element [%s] in <Report>\n", cur->name);
		cur = cur->next;
	}
}

static struct rlib_element * parse_part_load(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_element *e = g_new0(struct rlib_element, 1);
	gchar *name;
	name =  xmlGetProp(cur, (const xmlChar *) "name");
	e->data = parse_report_file(name);
	e->type = RLIB_ELEMENT_REPORT;
	return e;
}

static struct rlib_element * parse_part_td(struct rlib_part *part, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_element *e = g_new0(struct rlib_element, 1);
	struct rlib_part_td *td = g_new0(struct rlib_part_td, 1);	
	td->xml_width =  xmlGetProp(cur, (const xmlChar *) "width");
	e->data = td;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {      
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "load"))) {
			if(td->e == NULL) {
				td->e = parse_part_load(doc, ns, cur);
			} else {
				struct rlib_element *xxx = td->e;
				for(;xxx->next != NULL; xxx=xxx->next) {};
				xxx->next = parse_part_load(doc, ns, cur);				
			}
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "Report"))) {
				struct rlib_report *report;
				struct rlib_element *e_report;
				report = (struct rlib_report *) g_new0(struct rlib_report, 1);
				parse_report(part, report, doc, ns, cur);
				e_report = g_new0(struct rlib_element, 1);
				e_report->data = report;
				e_report->type = RLIB_ELEMENT_REPORT;				
			if(td->e == NULL) {
				td->e = e_report;
			} else {
				struct rlib_element *xxx = td->e;
				for(;xxx->next != NULL; xxx=xxx->next) {};
				xxx->next = e_report;
			}		
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			rlogit("Unknown element [%s] in <tr>. Expected td.\n", cur->name);
		}
		cur = cur->next;
	}	
	return e;
}

static struct rlib_element * parse_part_tr(struct rlib_part *part, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct rlib_element *e = g_new0(struct rlib_element, 1);
	struct rlib_part_tr *tr = g_new0(struct rlib_part_tr, 1);
	tr->xml_layout = xmlGetProp(cur, (const xmlChar *) "layout");
	tr->xml_newpage = xmlGetProp(cur, (const xmlChar *) "newpage");
		
	cur = cur->xmlChildrenNode;
	e->data = tr;
	while (cur != NULL) {      
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "td"))) {
			if(tr->e == NULL) {
				tr->e = parse_part_td(part, doc, ns, cur);
			} else {
				struct rlib_element *xxx = tr->e;
				for(;xxx->next != NULL; xxx=xxx->next) {};
				xxx->next = parse_part_td(part, doc, ns, cur);				
			}
		} else if (ignoreElement(cur->name)) {
			/* ignore comments, etc */
		} else {
			rlogit("Unknown element [%s] in <tr>. Expected td.\n", cur->name);
		}
		cur = cur->next;
	}	
	return e;
}

static void parse_part(struct rlib_part *part, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	while (cur && xmlIsBlankNode (cur)) 
		cur = cur -> next;

	if(cur == 0)
		return;

	part->xml_name = xmlGetProp(cur, (const xmlChar *) "name");
	part->xml_pages_accross = xmlGetProp(cur, (const xmlChar *) "pages_accross");
	part->xml_font_size = xmlGetProp(cur, (const xmlChar *) "fontSize");
	part->xml_orientation = xmlGetProp(cur, (const xmlChar *) "orientation");
	part->xml_top_margin = xmlGetProp(cur, (const xmlChar *) "top_margin");
	part->xml_left_margin = xmlGetProp(cur, (const xmlChar *) "left_margin");
	part->xml_bottom_margin = xmlGetProp(cur, (const xmlChar *) "bottom_margin");
	part->xml_paper_type = xmlGetProp(cur, (const xmlChar *) "paper_type");
	
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "tr"))) {
		
			if(part->tr_elements == NULL) {
				part->tr_elements = parse_part_tr(part, doc, ns, cur);
			} else {
				struct rlib_element *xxx = part->tr_elements;
				for(;xxx->next != NULL; xxx=xxx->next) {};
				xxx->next = parse_part_tr(part, doc, ns, cur);				
			}
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "PageHeader"))) {
			part->page_header = parse_report_outputs(doc, ns, cur);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "PageFooter"))) {
			part->page_footer = parse_report_outputs(doc, ns, cur);
		} else if (!ignoreElement(cur->name)) //must be last
			/* ignore comments, etc */
			rlogit("Unknown element [%s] in <Part>\n", cur->name);
		cur = cur->next;
	}
}

void dump_part_td(struct rlib_element *e_td) {
	struct rlib_element *e, *td_contents;
	for(e=e_td;e != NULL;e=e->next) {
		struct rlib_part_td *td = e->data;
		rlogit("    TD %s\n", td->xml_width);
		for(td_contents=td->e;td_contents != NULL;td_contents=td_contents->next) {
			if(td_contents->type == RLIB_ELEMENT_PART) {
				struct rlib_part *part = td_contents->data;
				dump_part(part);
			} else if(td_contents->type == RLIB_ELEMENT_REPORT) {
				rlogit("-- REPORT --\n");
			} else {
				rlogit("UNKNOWN\n");
			}
		}			
	}	
}

void dump_part_tr(struct rlib_element *e_tr) {
	struct rlib_element *e;
	for(e=e_tr;e != NULL;e=e->next) {
		struct rlib_part_tr *tr = e->data;
		rlogit("  TR Layout = [%s] NewPage = [%s]\n", tr->xml_layout, tr->xml_newpage);
		dump_part_td(tr->e);
	}	
}

void dump_part(struct rlib_part *part) {
	rlogit("Name: [%s] Pages Accross [%s]\n ", part->xml_name, part->xml_pages_accross);
	dump_part_tr(part->tr_elements);
	rlogit("DONE!\n");
}

struct rlib_part * parse_part_file(gchar *filename) {
	xmlDocPtr doc;
	struct rlib_report *report;
	struct rlib_part *part = NULL;
	xmlNsPtr ns = NULL;
	xmlNodePtr cur;
	int found = FALSE;

	doc = xmlReadFile(filename, NULL, XML_PARSE_XINCLUDE);
	xmlXIncludeProcess(doc);
	
	if (doc == NULL)  {
		rlogit("xmlParseError \n");
		return(NULL);
	}
  
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		rlogit("xmlParseError \n");
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
		parse_part(part, doc, ns, cur);
		found = TRUE;
	}

/*
	If a report appears by it's self put it in a table w/ 1 row and 1 col 100%
*/	
	if((xmlStrcmp(cur->name, (const xmlChar *) "Report"))==0) {
		struct rlib_part_tr *tr= g_new0(struct rlib_part_tr, 1);
		struct rlib_part_td *td= g_new0(struct rlib_part_td, 1);
		struct rlib_element *tr_element = g_new0(struct rlib_element, 1);
		struct rlib_element *td_element = g_new0(struct rlib_element, 1);
		struct rlib_element *part_element = g_new0(struct rlib_element, 1);
		
		part->tr_elements = tr_element;
		tr_element->data = tr;
		tr->e = td_element;
		td_element->data = td;
		td->e = part_element;
		part_element->data = report;
		part_element->type = RLIB_ELEMENT_REPORT;
		parse_report(part, report, doc, ns, cur);
		found = TRUE;
	}
	
	if(!found) {
		rlogit("document of the wrong type, was '%s', Report or Part expected", cur->name);
		rlogit("xmlDocDump follows\n");
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

static struct rlib_report * parse_report_file(gchar *filename) {
	xmlDocPtr doc;
	struct rlib_report *report;
	xmlNsPtr ns = NULL;
	xmlNodePtr cur;
	int found = FALSE;

	doc = xmlReadFile(filename, NULL, XML_PARSE_XINCLUDE);
	xmlXIncludeProcess(doc);
	
	if (doc == NULL)  {
		rlogit("xmlParseError \n");
		return(NULL);
	}
  
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		rlogit("xmlParseError \n");
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
		parse_report(NULL, report, doc, ns, cur);
		found = TRUE;
	}
	
	if(!found) {
		rlogit("document of the wrong type, was '%s', Report", cur->name);
		rlogit("xmlDocDump follows\n");
		xmlDocDump ( stderr, doc );
		xmlFreeDoc(doc);
		g_free(report);
		return(NULL);
	}

	
#if DISABLE_UTF8
	if((long)cd != -1)
		iconv_close(cd);
#endif	

	return report;
}
