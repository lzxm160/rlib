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
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libxml/xmlversion.h>
#include <libxml/xmlmemory.h>

#include "ralloc.h"
#include "rlib.h"

void utf8_to_8813(struct rlib_report *rep, char *dest, char *str) {
	size_t len = MAXSTRLEN;
	size_t slen;
	char *olddest = dest;
	if(str != NULL && str[0] != 0) {
		if(rep->cd != NULL) {
			slen = strlen(str);
			bzero(dest, MAXSTRLEN);
			iconv(rep->cd, &str, &slen, &olddest, &len);
		} else {
			strcpy(dest, str);
		}
	} else {
		dest[0] = 0;
	}
}

struct report_element * parse_line_array(struct rlib_report *rep, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct report_element *e, *current;
	e = NULL;
	
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {      
		current = rmalloc(sizeof(struct report_element));
		current->next = NULL;
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "field"))) {
			struct report_field *f = rmalloc(sizeof(struct report_field));
			//utf8_to_8813(rep, f->value, xmlGetProp(cur, (const xmlChar *) "value"));
			//TODO: we need to utf to 8813 all string values in single quotes
			strcpy(f->value, xmlGetProp(cur, (const xmlChar *) "value"));
			f->xml_align = xmlGetProp(cur, (const xmlChar *) "align");
			f->bgcolor = xmlGetProp(cur, (const xmlChar *) "bgcolor");
			f->color = xmlGetProp(cur, (const xmlChar *) "color");
			f->xml_width = xmlGetProp(cur, (const xmlChar *) "width");
			f->format = xmlGetProp(cur, (const xmlChar *) "format");
			f->link = xmlGetProp(cur, (const xmlChar *) "link");
			f->col = xmlGetProp(cur, (const xmlChar *) "col");
			current->data = f;
			current->type = REPORT_ELEMENT_FIELD;
		}

		if ((!xmlStrcmp(cur->name, (const xmlChar *) "text"))) {
			struct report_text *t = rmalloc(sizeof(struct report_text));
			utf8_to_8813(rep, t->value, xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
			t->xml_align = xmlGetProp(cur, (const xmlChar *) "align");
			t->bgcolor = xmlGetProp(cur, (const xmlChar *) "bgcolor");
			t->color = xmlGetProp(cur, (const xmlChar *) "color");
			t->xml_width = xmlGetProp(cur, (const xmlChar *) "width");
			t->col = xmlGetProp(cur, (const xmlChar *) "col");
			current->data = t;
			current->type = REPORT_ELEMENT_TEXT;
		}
		
		
		if(e == NULL)
			e = current;
		else {
			struct report_element *xxx;
			xxx = e;
			while(xxx->next != NULL) xxx =  xxx->next;
			xxx->next = current;
		}
		cur = cur->next;
	}

	return e;
}

struct report_output * report_output_new(int type, void *data) {
	struct report_output *ro = rmalloc(sizeof(struct report_output));
	ro->type = type;
	ro->data = data;
	return ro;
}

static struct report_output_array * parse_report_element(struct rlib_report *rep, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct report_output_array *roa = rmalloc(sizeof(struct report_output_array));
	roa->count = 0;
	roa->data = NULL;

	cur = cur->xmlChildrenNode;
	if(cur != NULL && (!xmlStrcmp(cur->name, (const xmlChar *) "Output")))
		cur = cur->xmlChildrenNode;
	
	while (cur != NULL) {      
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "Line"))) {
			struct report_lines *rl = rmalloc(sizeof(struct report_lines));
			rl->bgcolor = xmlGetProp(cur, (const xmlChar *) "bgcolor");
			rl->color = xmlGetProp(cur, (const xmlChar *) "color");
			rl->fontSize = xmlGetProp(cur, (const xmlChar *) "fontSize");
			if(rl->fontSize == NULL)
				rl->font_point = -1;
			else
				rl->font_point = atoi(rl->fontSize);
				
			rl->e = parse_line_array(rep, doc, ns, cur);
			roa->data = rrealloc(roa->data, sizeof(struct report_output_array *) * (roa->count + 1));
			roa->data[roa->count++] = report_output_new(REPORT_PRESENTATION_DATA_LINE, rl);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "HorizontalLine"))) {
			struct report_horizontal_line *rhl = rmalloc(sizeof(struct report_horizontal_line));
			rhl->bgcolor = xmlGetProp(cur, (const xmlChar *) "bgcolor");
			rhl->size = xmlGetProp(cur, (const xmlChar *) "size");
			rhl->indent = xmlGetProp(cur, (const xmlChar *) "indent");
			rhl->length = xmlGetProp(cur, (const xmlChar *) "length");
			rhl->fontSize = xmlGetProp(cur, (const xmlChar *) "fontSize");
			if(rhl->fontSize == NULL)
				rhl->font_point = -1;
			else
				rhl->font_point = atoi(rhl->fontSize);
			roa->data = rrealloc(roa->data, sizeof(struct report_output_array *) * (roa->count + 1));
			roa->data[roa->count++] = report_output_new(REPORT_PRESENTATION_DATA_HR, rhl);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "Image"))) {
			struct report_image *ri = rmalloc(sizeof(struct report_image));
			ri->value = xmlGetProp(cur, (const xmlChar *) "value");
			ri->type = xmlGetProp(cur, (const xmlChar *) "type");
			ri->width = xmlGetProp(cur, (const xmlChar *) "width");
			ri->height = xmlGetProp(cur, (const xmlChar *) "height");
			roa->data = rrealloc(roa->data, sizeof(struct report_output_array *) * (roa->count + 1));
			roa->data[roa->count++] = report_output_new(REPORT_PRESENTATION_DATA_IMAGE, ri);
		}
		cur = cur->next;
	}	
	return roa;
}

static struct report_element * parse_break_field(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct report_element *e = rmalloc(sizeof(struct report_element));
	struct break_fields *bf = rmalloc(sizeof(struct break_fields));
	e->next = NULL;
	e->data = bf;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "BreakField"))) {
			bf->rval = NULL;
			bf->value = xmlGetProp(cur, (const xmlChar *) "value");
		}
		cur = cur->next;
	}
	return e;
}

static struct report_element * parse_report_break(struct rlib_report *rep, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct report_element *e = rmalloc(sizeof(struct report_element));
	struct report_break *rb = rmalloc(sizeof(struct report_break));
	e->next = NULL;
	e->data = rb;
	rb->name = xmlGetProp(cur, (const xmlChar *) "name");
	rb->xml_newpage = xmlGetProp(cur, (const xmlChar *) "newpage");
	rb->xml_headernewpage = xmlGetProp(cur, (const xmlChar *) "headernewpage");
	rb->xml_surpressblank = xmlGetProp(cur, (const xmlChar *) "surpressblank");
	cur = cur->xmlChildrenNode;
	rb->fields = NULL;
	rb->header = NULL;
	rb->footer = NULL;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "BreakHeader"))) {
			rb->header = parse_report_element(rep, doc, ns, cur);
		}
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "BreakFooter"))) {
			rb->footer = parse_report_element(rep, doc, ns, cur);
		}
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "BreakFields"))) {
			if(rb->fields == NULL)
				rb->fields = parse_break_field(doc, ns, cur);
			else {
				struct report_element *xxx = rb->fields;
				for(;xxx->next != NULL; xxx=xxx->next) {};
				xxx->next = parse_report_break(rep, doc, ns, cur);							
			}
		}
		cur = cur->next;
	}
	
	return e;
}

static struct report_element * parse_report_breaks(struct rlib_report *rep, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct report_element *e = NULL;

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {      
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "Break"))) {
			if(e == NULL) {
				e = parse_report_break(rep, doc, ns, cur);
			} else {
				struct report_element *xxx = e;
				for(;xxx->next != NULL; xxx=xxx->next) {};
				xxx->next = parse_report_break(rep, doc, ns, cur);				
			}
		}
		cur = cur->next;
	}	
	return e;
}

static void parse_detail(struct rlib_report *rep, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur, struct report_detail *r) {
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "FieldHeaders"))) {
			r->textlines = parse_report_element(rep, doc, ns, cur);
		}
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "FieldDetails"))) {
			r->fields = parse_report_element(rep, doc, ns, cur);
		}
		cur = cur->next;
	}

}

static struct report_element * parse_report_variable(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct report_element *e = rmalloc(sizeof(struct report_element));
	struct report_variable *rv = rmalloc(sizeof(struct report_variable));
	e->next = NULL;
	e->data = rv;
	rv->name = xmlGetProp(cur, (const xmlChar *) "name");
	rv->str_type = xmlGetProp(cur, (const xmlChar *) "type");
	rv->value = xmlGetProp(cur, (const xmlChar *) "value");
	rv->resetonbreak = xmlGetProp(cur, (const xmlChar *) "resetonbreak");

	rv->type = REPORT_VARIABLE_UNDEFINED;
	if(rv->str_type != NULL && rv->str_type[0] != '\0') {
		if(!strcmp(rv->str_type, "expression"))
			rv->type = REPORT_VARIABLE_EXPRESSION;
		if(!strcmp(rv->str_type, "count"))
			rv->type = REPORT_VARIABLE_COUNT;
		if(!strcmp(rv->str_type, "sum"))
			rv->type = REPORT_VARIABLE_SUM;
		if(!strcmp(rv->str_type, "average"))
			rv->type = REPORT_VARIABLE_AVERAGE;
		if(!strcmp(rv->str_type, "lowest"))
			rv->type = REPORT_VARIABLE_LOWEST;
		if(!strcmp(rv->str_type, "highest"))
			rv->type = REPORT_VARIABLE_HIGHEST;
	}

	return e;
}

static struct report_element * parse_report_variables(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
	struct report_element *e = NULL;

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {      
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "Variable"))) {
			if(e == NULL) {
				e = parse_report_variable(doc, ns, cur);
			} else {
				struct report_element *xxx = e;
				for(;xxx->next != NULL; xxx=xxx->next) {};
				xxx->next = parse_report_variable(doc, ns, cur);				
			}
		}
		cur = cur->next;
	}	
	return e;
}

struct rlib_report * parse_report_file(char *filename) {
	xmlDocPtr doc;
	struct rlib_report *ret;
	xmlNsPtr ns = NULL;
	xmlNodePtr cur;

	doc = xmlParseFile(filename);

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
	
	if (xmlStrcmp(cur->name, (const xmlChar *) "Report")) {
		rlogit("Report Node Expected.. C ya!\n");
		xmlFreeDoc(doc);
		return(NULL);
	}

	ret = (struct rlib_report *) rmalloc(sizeof(struct rlib_report));
	if (ret == NULL) {
		rlogit("Out of Memory :(\n");
		xmlFreeDoc(doc);
		return(NULL);
	}
	
	ret->report_header = NULL;

	bzero(ret, sizeof(struct rlib_report));

	ret->doc = doc;
	
	ret->cd = iconv_open("ISO8859-1", "UTF-8");

	while ( cur && xmlIsBlankNode ( cur ) ) {
		cur = cur -> next;
   }

	if ( cur == 0 )
   	return ( NULL );

	if ((xmlStrcmp(cur->name, (const xmlChar *) "Report"))) {
		rlogit("document of the wrong type, was '%s', Report expected", cur->name);
		rlogit("xmlDocDump follows\n");
		xmlDocDump ( stderr, doc );
		xmlFreeDoc(doc);
		rfree(ret);
		return(NULL);
	}
	ret->xml_fontsize = xmlGetProp(cur, (const xmlChar *) "fontSize");
	ret->xml_orientation = xmlGetProp(cur, (const xmlChar *) "orientation");
	ret->defaultResult = xmlGetProp(cur, (const xmlChar *) "defaultResult");
	ret->xml_top_margin = xmlGetProp(cur, (const xmlChar *) "topMargin");
	ret->xml_left_margin = xmlGetProp(cur, (const xmlChar *) "leftMargin");
	ret->xml_bottom_margin = xmlGetProp(cur, (const xmlChar *) "bottomMargin");

	
	cur = cur->xmlChildrenNode;
	ret->breaks = NULL;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "ReportHeader"))) {
			ret->report_header = parse_report_element(ret, doc, ns, cur);
		}
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "PageHeader"))) {
			ret->page_header = parse_report_element(ret, doc, ns, cur);
		}
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "PageFooter"))) {
			ret->page_footer = parse_report_element(ret, doc, ns, cur);
		}
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "ReportFooter"))) {
			ret->report_footer = parse_report_element(ret, doc, ns, cur);
		}
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "Detail"))) {
			parse_detail(ret, doc, ns, cur, &ret->detail);
		}
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "Breaks"))) {
			ret->breaks = parse_report_breaks(ret, doc, ns, cur);
		}
		
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "Variables"))) {
			ret->variables = parse_report_variables(doc, ns, cur);
		}

		cur = cur->next;
	}
	
	iconv_close(ret->cd);
	
	return(ret);
}
