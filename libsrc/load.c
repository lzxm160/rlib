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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "config.h"
#include "rlib.h"

struct packet {
	gint32 type;
	gint32 size;
};

gchar * read_xml_str(gchar **ptr) {
	struct packet *p;
	gchar *str;
	p = (struct packet *)*ptr;
	*ptr += sizeof(struct packet);
	if(p->type == RLIB_FILE_XML_STR && p->size > 0) {
		str = *ptr;
		*ptr += p->size;
		return str;
	} 
	return NULL;
}

struct report_image * read_image(gchar **ptr) {
	struct report_image *ri;
	ri = g_malloc(sizeof(struct report_image));
	ri->value = read_xml_str(ptr);
	ri->type = read_xml_str(ptr);
	ri->width = read_xml_str(ptr);
	ri->height = read_xml_str(ptr);
	return ri;
}

struct report_horizontal_line * read_hr(gchar **ptr) {
	struct report_horizontal_line *hr;
	hr = g_malloc(sizeof(struct report_horizontal_line));
	hr->bgcolor = read_xml_str(ptr);
	hr->size = read_xml_str(ptr);
	hr->indent = read_xml_str(ptr);
	hr->length = read_xml_str(ptr);
	hr->font_size = read_xml_str(ptr);
	hr->suppress = read_xml_str(ptr);
	return hr;
}

struct report_literal * read_text(gchar **ptr) {
	struct report_literal *rt;
	gchar *tmp;
	rt = g_malloc(sizeof(struct report_literal));
	*ptr += sizeof(gint32);
	tmp = read_xml_str(ptr);
	if(tmp != NULL)
		strcpy(rt->value, tmp);
	else
		rt->value[0] = 0;
	rt->xml_align = read_xml_str(ptr);
	rt->color = read_xml_str(ptr);
	rt->bgcolor = read_xml_str(ptr);
	rt->xml_width = read_xml_str(ptr);
	rt->col = read_xml_str(ptr);
	return rt;
}

struct report_field * read_field(gchar **ptr) {
	struct report_field *rf;
	gchar *tmp;
	rf = g_malloc(sizeof(struct report_field));
	*ptr += sizeof(gint32);
	tmp = read_xml_str(ptr);
	if(tmp != NULL)
		strcpy(rf->value, tmp);
	else
		rf->value[0] = 0;

	rf->xml_align = read_xml_str(ptr);
	rf->bgcolor = read_xml_str(ptr);
	rf->color = read_xml_str(ptr);
	rf->xml_width = read_xml_str(ptr);
	rf->format = read_xml_str(ptr);
	rf->link = read_xml_str(ptr);
	rf->col = read_xml_str(ptr);
	rf->xml_wrapchars = read_xml_str(ptr);
	rf->xml_maxlines = read_xml_str(ptr);
	return rf;
}


struct report_lines * read_line(gchar **ptr) {
	struct report_lines *rl = g_malloc(sizeof(struct report_lines));
	gint32 *type;
	gpointer pointer = NULL;
	struct report_element *current;
	
	rl->bgcolor = read_xml_str(ptr);
	rl->color = read_xml_str(ptr);
	rl->font_size = read_xml_str(ptr);
	rl->suppress = read_xml_str(ptr);
	rl->e = NULL;

	type = (gint32 *)*ptr;
	if(*type == RLIB_FILE_LINE) {
		*ptr += sizeof(gint32);
		while(1) {
			type = (gint32 *)*ptr;
			if(*type == RLIB_FILE_LINE)
				break;
			if(*type == REPORT_ELEMENT_FIELD)
				pointer = read_field(ptr);
			if(*type == REPORT_ELEMENT_LITERAL)
				pointer = read_text(ptr);
			if(*type == REPORT_ELEMENT_FIELD || *type == REPORT_ELEMENT_LITERAL) {
				if(rl->e == NULL) {
					rl->e = g_malloc(sizeof(struct report_element));
					current = rl->e;
				} else {
					struct report_element *e;
					for(e=rl->e;e->next != NULL;e=e->next);
					e->next = g_malloc(sizeof(struct report_element));
					current = e->next;
				}
				current->data = pointer;
				current->type = *type;
				current->next = NULL;
			}
		}
		*ptr += sizeof(gint32);
	}
	return rl;
}

struct report_output_array * read_roa(gchar **ptr) {
	gint32 *type;
	struct report_output_array *roa = g_new0(struct report_output_array, 1);
	roa->count = 0;
	roa->data = NULL;

	type = (gint32 *)*ptr;
	if(*type == RLIB_FILE_ROA) {
		*ptr += sizeof(gint32);
		roa->xml_page = read_xml_str(ptr);
		while(1) {
			type = (gint32 *)*ptr;
			*ptr += sizeof(gint32);
			
			if(*type == RLIB_FILE_ROA+1)
				break;
			roa->data = g_realloc(roa->data, sizeof(struct report_output_array *) * (roa->count + 1));

			if(*type == REPORT_PRESENTATION_DATA_IMAGE)
				roa->data[roa->count++] = report_output_new(REPORT_PRESENTATION_DATA_IMAGE, read_image(ptr));				
			else if(*type == REPORT_PRESENTATION_DATA_HR)
				roa->data[roa->count++] = report_output_new(REPORT_PRESENTATION_DATA_HR, read_hr(ptr));				
			else if(*type == REPORT_PRESENTATION_DATA_LINE)
				roa->data[roa->count++] = report_output_new(REPORT_PRESENTATION_DATA_LINE, read_line(ptr));				
		
		}
		return roa;
	}
	return NULL;
}

struct report_element * read_output(gchar **ptr) {
	struct report_element *e = NULL;
	gint32 *type;
	type = (gint32 *)*ptr;
	if(*type == RLIB_FILE_OUTPUTS) {
		*ptr += sizeof(gint32);
		while(1) {
			type = (gint32 *)*ptr;
			if(*type == RLIB_FILE_OUTPUTS+1)
				break;
			if(*type == RLIB_FILE_ROA) {
				if(e == NULL) {
					e = g_new0(struct report_element, 1);
					e->data = read_roa(ptr);
					e->next = NULL;
				} else {
					struct report_element *xxx, *newe;
					for(xxx=e;xxx->next != NULL; xxx=xxx->next) {};
					newe = g_new0(struct report_element, 1);
					xxx->next = newe;
					newe->data = read_roa(ptr);
					newe->next = NULL;
				}
			}
		}
		*ptr += sizeof(gint32);
	}
	return e;
}

struct report_variable * read_variable(gchar **ptr) {
	struct report_variable *rv = g_malloc(sizeof(struct report_variable));
	gint32 *type;
	type = (gint32 *)*ptr;
	if(*type == RLIB_FILE_VARIABLE) {
		*ptr += sizeof(gint32);
		rv->name = read_xml_str(ptr);
		rv->str_type = read_xml_str(ptr);
		rv->value = read_xml_str(ptr);
		rv->resetonbreak = read_xml_str(ptr);
		*ptr += sizeof(gint32);
	}	
	return rv;
}

struct report_element * read_variables(gchar **ptr) {
	struct report_element *e=NULL, *current=NULL, *moving=NULL;
	gint32 *type;
	type = (gint32 *)*ptr;
	if(*type == RLIB_FILE_VARIABLES) {
		*ptr += sizeof(gint32);
		while(1) {
			type = (gint32 *)*ptr;
			if(*type == RLIB_FILE_VARIABLES+1)
				break;
			if(*type == RLIB_FILE_VARIABLE) {
				if(e == NULL) {
					current = e = g_malloc(sizeof(struct report_element));	
				} else {
					for(moving=e;moving->next != NULL;moving = moving->next);
					current = moving->next = g_malloc(sizeof(struct report_element));	
				}
				current->data = read_variable(ptr);
				current->next = NULL;
			}
		}
		*ptr += sizeof(gint32);
	}

	return e;
}

struct report_break * read_break(gchar **ptr) {
	struct report_break *rb = g_malloc(sizeof(struct report_break));
	struct report_element *current=NULL, *moving=NULL;
	struct break_fields *bf=NULL;
	gint32 *type;
	type = (gint32 *)*ptr;
	if(*type == RLIB_FILE_BREAK) {
		*ptr += sizeof(gint32);
		rb->name = read_xml_str(ptr);
		rb->xml_newpage = read_xml_str(ptr);
		rb->xml_headernewpage = read_xml_str(ptr);
		rb->xml_suppressblank = read_xml_str(ptr);
		rb->header = read_output(ptr);
		rb->footer = read_output(ptr);
		rb->fields = NULL;
		while(1) {
			type = (gint32 *)*ptr;
			if(*type == RLIB_FILE_BREAK_FIELD) {
				*ptr += sizeof(gint32);
				if(rb->fields == NULL) {
					current = rb->fields = g_malloc(sizeof(struct report_element));	
				} else {
					for(moving=rb->fields;moving->next != NULL;moving = moving->next);
					current = moving->next = g_malloc(sizeof(struct report_element));	
				}
				bf = g_malloc(sizeof(struct break_fields));
				bf->rval = NULL;
				bf->value = read_xml_str(ptr);
				current->data = bf;
				current->next = NULL;
				*ptr += sizeof(gint32);
			} else if(*type == RLIB_FILE_BREAK+1)
				break;
		}
		*ptr += sizeof(gint32);
	}	
	return rb;
}

struct report_element * read_breaks(gchar **ptr) {
	struct report_element *e=NULL, *moving=NULL, *current=NULL;
	gint32 *type;
	type = (gint32 *)*ptr;
	if(*type == RLIB_FILE_BREAKS) {
		*ptr += sizeof(gint32);
		while(1) {
			type = (gint32 *)*ptr;
			if(*type == RLIB_FILE_BREAKS+1)
				break;
			if(*type == RLIB_FILE_BREAK) {
				if(e == NULL) {
					current = e = g_malloc(sizeof(struct report_element));	
				} else {
					for(moving=e;moving->next != NULL;moving = moving->next);
					current = moving->next = g_malloc(sizeof(struct report_element));	
				}
				current->data = read_break(ptr);
				current->next = NULL;
				
			}

		}
		*ptr += sizeof(gint32);
	}

	return e;
}

struct rlib_report * load_report(gchar *filename) {
	gint fd, size;
	gchar *contents;
	gchar *ptr;
	gchar buf[MAXSTRLEN];
	struct rlib_report *rep;

	fd = open(filename, O_RDONLY, 0664);
	if(fd <= 0)
		return NULL;
	size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	contents = g_malloc(size);
	read(fd, contents, size);

	sprintf(buf, "RLIB %s                                ", VERSION);
	buf[20] = 0;
	if(memcmp(contents, buf, 20)) {
		g_free(contents);
		close(fd);
		return NULL;
	
	}

	rep = g_malloc(sizeof(struct rlib_report));
	memset(rep, 0, sizeof(struct rlib_report));

	ptr = contents+20;
	rep->xml_font_size = read_xml_str(&ptr);
	rep->xml_orientation = read_xml_str(&ptr);
	rep->xml_top_margin = read_xml_str(&ptr);
	rep->xml_left_margin = read_xml_str(&ptr);
	rep->xml_bottom_margin = read_xml_str(&ptr);
	rep->xml_paper_type = read_xml_str(&ptr);
	rep->xml_pages_accross = read_xml_str(&ptr);
	rep->xml_suppress_page_header_first_page = read_xml_str(&ptr);

	rep->report_header = read_output(&ptr);
	rep->page_header = read_output(&ptr);
	rep->detail.textlines = read_output(&ptr);
	rep->detail.fields = read_output(&ptr);
	rep->page_footer = read_output(&ptr);
	rep->report_footer = read_output(&ptr);
	rep->variables = read_variables(&ptr);
	rep->breaks = read_breaks(&ptr);
	
	rep->contents = contents;
	rep->doc = NULL;
	//g_free(contents);
	close(fd);
	return rep;
}
