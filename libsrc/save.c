/*
 *  Copyright (C) 2003-2006 SICOM Systems, INC.
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
#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
 
#include "rlib.h"

struct packet {
	gint32 type;
	gint32 size;
};

void write_xml_str(gint fd, gchar *str) {
	gint len;
	gint result;
	struct packet p;
	if(str != NULL)
		len = strlen(str)+1;
	else
		len = 0;
	p.type = RLIB_FILE_XML_STR;
	p.size = len;
	result = write(fd, &p, sizeof(struct packet));
	if(len > 0)
		result = write(fd, str, len);
	return;
}

void write_image(gint fd, struct rlib_report_image *ri) {
	write_xml_str(fd, (gchar *)ri->xml_value.xml);
	write_xml_str(fd, (gchar *)ri->xml_type.xml);
	write_xml_str(fd, (gchar *)ri->xml_width.xml);
	write_xml_str(fd, (gchar *)ri->xml_height.xml);
}

void write_hr(gint fd, struct rlib_report_horizontal_line *hr) {
	write_xml_str(fd, (gchar *)hr->xml_bgcolor.xml);
	write_xml_str(fd, (gchar *)hr->xml_size.xml);
	write_xml_str(fd, (gchar *)hr->xml_indent.xml);
	write_xml_str(fd, (gchar *)hr->xml_length.xml);
	write_xml_str(fd, (gchar *)hr->xml_font_size.xml);
	write_xml_str(fd, (gchar *)hr->xml_suppress.xml);
}

void write_field(gint fd, struct rlib_report_field *rf) {
	write_xml_str(fd, (gchar *)rf->value);
	write_xml_str(fd, (gchar *)rf->xml_align.xml);
	write_xml_str(fd, (gchar *)rf->xml_bgcolor.xml);
	write_xml_str(fd, (gchar *)rf->xml_color.xml);
	write_xml_str(fd, (gchar *)rf->xml_width.xml);
	write_xml_str(fd, (gchar *)rf->xml_format.xml);
	write_xml_str(fd, (gchar *)rf->xml_link.xml);
	write_xml_str(fd, (gchar *)rf->xml_col.xml);
}

void write_text(gint fd, struct rlib_report_literal *rt) {
	write_xml_str(fd, (gchar *)rt->value);
	write_xml_str(fd, (gchar *)rt->xml_align.xml);
	write_xml_str(fd, (gchar *)rt->xml_color.xml);
	write_xml_str(fd, (gchar *)rt->xml_bgcolor.xml);
	write_xml_str(fd, (gchar *)rt->xml_width.xml);
	write_xml_str(fd, (gchar *)rt->xml_col.xml);
}

void write_line(gint fd, struct rlib_report_lines *rl) {
	struct rlib_element *e;
	gint32 type;
	gint result;
	write_xml_str(fd, (gchar *)rl->xml_bgcolor.xml);
	write_xml_str(fd, (gchar *)rl->xml_color.xml);
	write_xml_str(fd, (gchar *)rl->xml_font_size.xml);
	write_xml_str(fd, (gchar *)rl->xml_suppress.xml);

	type = RLIB_FILE_LINE;
	result = write(fd, &type, sizeof(gint32));
	for(e=rl->e; e != NULL; e=e->next) {
		type = e->type;
		result = write(fd, &type, sizeof(gint32));
		if(e->type == RLIB_ELEMENT_FIELD) {
			struct rlib_report_field *rf = e->data;
			write_field(fd, rf);
		} else if(e->type == RLIB_ELEMENT_LITERAL) {
			struct rlib_report_literal *rt = e->data;
			write_text(fd, rt);
		}
	}
	type = RLIB_FILE_LINE;
	result = write(fd, &type, sizeof(gint32));		

}

void write_roa(gint fd, struct rlib_report_output_array *roa) {
	gint j;
	gint32 type;
	gint result;
	type = RLIB_FILE_ROA;
	result = write(fd, &type, sizeof(gint32));
	write_xml_str(fd, (gchar *)roa->xml_page.xml);
	for(j=0;j<roa->count;j++) {
		struct rlib_report_output *ro = roa->data[j];
		type = ro->type;
		result = write(fd, &type, sizeof(gint32));
		if(ro->type == RLIB_REPORT_PRESENTATION_DATA_LINE) {			
			struct rlib_report_lines *rl = ro->data;	
			write_line(fd, rl);
		} else if(ro->type == RLIB_REPORT_PRESENTATION_DATA_HR) {
			struct rlib_report_horizontal_line *hr = ro->data;
			write_hr(fd, hr);
		} else if(ro->type == RLIB_REPORT_PRESENTATION_DATA_IMAGE) {
			struct rlib_report_image *ri = ro->data;
			write_image(fd, ri);
		}

	}	
	type = RLIB_FILE_ROA+1;
	result = write(fd, &type, sizeof(gint32));
}

void write_output(gint fd, struct rlib_element *e) {
	struct rlib_report_output_array *roa;
	gint32 type = RLIB_FILE_OUTPUTS;
	gint result;
	result = write(fd, &type, sizeof(gint32));
	for(; e != NULL; e=e->next) {
		roa = e->data;
		write_roa(fd, roa);
	}
	type++;
	result = write(fd, &type, sizeof(gint32));
}

void write_variables(gint fd, struct rlib_element *rv) {
	struct rlib_element *e;
	gint32 type = RLIB_FILE_VARIABLES;
	gint result;
	
	result = write(fd, &type, sizeof(gint32));
	for(e=rv; e != NULL; e=e->next) {
		struct rlib_report_variable *rv1 = e->data;
		type = RLIB_FILE_VARIABLE;
		result = write(fd, &type, sizeof(gint32));
		write_xml_str(fd, (gchar *)rv1->xml_name.xml);
		write_xml_str(fd, (gchar *)rv1->xml_str_type.xml);
		write_xml_str(fd, (gchar *)rv1->xml_value.xml);
		write_xml_str(fd, (gchar *)rv1->xml_resetonbreak.xml);
		type = RLIB_FILE_VARIABLE+1;
		result = write(fd, &type, sizeof(gint32));
	}
	type = RLIB_FILE_VARIABLES+1;
	result = write(fd, &type, sizeof(gint32));
}

void write_breaks(gint fd, struct rlib_element *breaks) {
	struct rlib_element *e;
	gint32 type = RLIB_FILE_BREAKS;
	gint result;
	result = write(fd, &type, sizeof(gint32));

	for(e=breaks; e != NULL; e=e->next) {
		struct rlib_report_break *rb = e->data;
		struct rlib_element *be;
		type = RLIB_FILE_BREAK;
		result = write(fd, &type, sizeof(gint32));
		write_xml_str(fd, (gchar *)rb->xml_name.xml);
		write_xml_str(fd, (gchar *)rb->xml_newpage.xml);
		write_xml_str(fd, (gchar *)rb->xml_headernewpage.xml);
		write_xml_str(fd, (gchar *)rb->xml_suppressblank.xml);
		write_output(fd, rb->header);
		write_output(fd, rb->footer);
		for(be = rb->fields; be != NULL; be=be->next) {
			struct rlib_break_fields *bf = be->data;
			type = RLIB_FILE_BREAK_FIELD;
			result = write(fd, &type, sizeof(gint32));
			write_xml_str(fd, (gchar *)bf->xml_value.xml);
			type = RLIB_FILE_BREAK_FIELD+1;
			result = write(fd, &type, sizeof(gint32));
		}		
		type = RLIB_FILE_BREAK+1;
		result = write(fd, &type, sizeof(gint32));
	}
	type = RLIB_FILE_BREAKS + 1;
	result = write(fd, &type, sizeof(gint32));
}

gint save_report(struct rlib_report *rep, gchar *filename) {
	gint fd;
	gchar buf[MAXSTRLEN];
	gint result;
	
	unlink(filename);
	fd = open(filename, O_RDWR | O_CREAT, 0664);
	sprintf(buf, "RLIB %s                                ", VERSION);
	result = write(fd, buf, 20);
	
	write_xml_str(fd, (gchar *)rep->xml_font_size.xml);
	write_xml_str(fd, (gchar *)rep->xml_orientation.xml);
	write_xml_str(fd, (gchar *)rep->xml_top_margin.xml); 
	write_xml_str(fd, (gchar *)rep->xml_left_margin.xml);
	write_xml_str(fd, (gchar *)rep->xml_bottom_margin.xml);
	write_xml_str(fd, (gchar *)rep->xml_pages_across.xml);
	write_xml_str(fd, (gchar *)rep->xml_suppress_page_header_first_page.xml);
	
	write_output(fd, rep->report_header);
	write_output(fd, rep->page_header);
	write_output(fd, rep->detail.headers);
	write_output(fd, rep->detail.fields);
	write_output(fd, rep->page_footer);
	write_output(fd, rep->report_footer);
	write_variables(fd, rep->variables);
	write_breaks(fd, rep->breaks);
	
	close(fd);
	return 0;
}
