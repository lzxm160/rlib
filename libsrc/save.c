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

void write_xml_str(gint fd, gchar *str) {
	gint len;
	struct packet p;
	if(str != NULL)
		len = strlen(str)+1;
	else
		len = 0;
	p.type = RLIB_FILE_XML_STR;
	p.size = len;
	write(fd, &p, sizeof(struct packet));
	if(len > 0)
		write(fd, str, len);
	return;
}

void write_image(gint fd, struct rlib_report_image *ri) {
	write_xml_str(fd, ri->xml_value);
	write_xml_str(fd, ri->xml_type);
	write_xml_str(fd, ri->xml_width);
	write_xml_str(fd, ri->xml_height);
}

void write_hr(gint fd, struct rlib_report_horizontal_line *hr) {
	write_xml_str(fd, hr->xml_bgcolor);
	write_xml_str(fd, hr->xml_size);
	write_xml_str(fd, hr->xml_indent);
	write_xml_str(fd, hr->xml_length);
	write_xml_str(fd, hr->xml_font_size);
	write_xml_str(fd, hr->xml_suppress);
}

void write_field(gint fd, struct rlib_report_field *rf) {
	write_xml_str(fd, rf->value);
	write_xml_str(fd, rf->xml_align);
	write_xml_str(fd, rf->xml_bgcolor);
	write_xml_str(fd, rf->xml_color);
	write_xml_str(fd, rf->xml_width);
	write_xml_str(fd, rf->xml_format);
	write_xml_str(fd, rf->xml_link);
	write_xml_str(fd, rf->xml_col);
	write_xml_str(fd, rf->xml_wrapchars);
	write_xml_str(fd, rf->xml_maxlines);
}

void write_text(gint fd, struct rlib_report_literal *rt) {
	write_xml_str(fd, rt->value);
	write_xml_str(fd, rt->xml_align);
	write_xml_str(fd, rt->xml_color);
	write_xml_str(fd, rt->xml_bgcolor);
	write_xml_str(fd, rt->xml_width);
	write_xml_str(fd, rt->xml_col);
}

void write_line(gint fd, struct rlib_report_lines *rl) {
	struct rlib_element *e;
	gint32 type;
	write_xml_str(fd, rl->xml_bgcolor);
	write_xml_str(fd, rl->xml_color);
	write_xml_str(fd, rl->xml_font_size);
	write_xml_str(fd, rl->xml_suppress);

	type = RLIB_FILE_LINE;
	write(fd, &type, sizeof(gint32));		
	for(e=rl->e; e != NULL; e=e->next) {
		type = e->type;
		write(fd, &type, sizeof(gint32));
		if(e->type == RLIB_ELEMENT_FIELD) {
			struct rlib_report_field *rf = e->data;
			write_field(fd, rf);
		} else if(e->type == RLIB_ELEMENT_LITERAL) {
			struct rlib_report_literal *rt = e->data;
			write_text(fd, rt);
		}
	}
	type = RLIB_FILE_LINE;
	write(fd, &type, sizeof(gint32));		

}

void write_roa(gint fd, struct rlib_report_output_array *roa) {
	gint j;
	gint32 type;
	type = RLIB_FILE_ROA;
	write(fd, &type, sizeof(gint32));
	write_xml_str(fd, roa->xml_page);
	for(j=0;j<roa->count;j++) {
		struct rlib_report_output *ro = roa->data[j];
		type = ro->type;
		write(fd, &type, sizeof(gint32));
		if(ro->type == REPORT_PRESENTATION_DATA_LINE) {			
			struct rlib_report_lines *rl = ro->data;	
			write_line(fd, rl);
		} else if(ro->type == REPORT_PRESENTATION_DATA_HR) {
			struct rlib_report_horizontal_line *hr = ro->data;
			write_hr(fd, hr);
		} else if(ro->type == REPORT_PRESENTATION_DATA_IMAGE) {
			struct rlib_report_image *ri = ro->data;
			write_image(fd, ri);
		}

	}	
	type = RLIB_FILE_ROA+1;
	write(fd, &type, sizeof(gint32));
}

void write_output(gint fd, struct rlib_element *e) {
	struct rlib_report_output_array *roa;
	gint32 type = RLIB_FILE_OUTPUTS;
	write(fd, &type, sizeof(gint32));
	for(; e != NULL; e=e->next) {
		roa = e->data;
		write_roa(fd, roa);
	}
	type++;
	write(fd, &type, sizeof(gint32));
}

void write_variables(gint fd, struct rlib_element *rv) {
	struct rlib_element *e;
	gint32 type = RLIB_FILE_VARIABLES;
	write(fd, &type, sizeof(gint32));

	for(e=rv; e != NULL; e=e->next) {
		struct rlib_report_variable *rv = e->data;
		type = RLIB_FILE_VARIABLE;
		write(fd, &type, sizeof(gint32));
		write_xml_str(fd, rv->xml_name);
		write_xml_str(fd, rv->xml_str_type);
		write_xml_str(fd, rv->xml_value);
		write_xml_str(fd, rv->xml_resetonbreak);
		type = RLIB_FILE_VARIABLE+1;
		write(fd, &type, sizeof(gint32));
	}
	type = RLIB_FILE_VARIABLES+1;
	write(fd, &type, sizeof(gint32));
}

void write_breaks(gint fd, struct rlib_element *breaks) {
	struct rlib_element *e;
	gint32 type = RLIB_FILE_BREAKS;
	write(fd, &type, sizeof(gint32));

	for(e=breaks; e != NULL; e=e->next) {
		struct rlib_report_break *rb = e->data;
		struct rlib_element *be;
		type = RLIB_FILE_BREAK;
		write(fd, &type, sizeof(gint32));
		write_xml_str(fd, rb->xml_name);
		write_xml_str(fd, rb->xml_newpage);
		write_xml_str(fd, rb->xml_headernewpage);
		write_xml_str(fd, rb->xml_suppressblank);
		write_output(fd, rb->header);
		write_output(fd, rb->footer);
		for(be = rb->fields; be != NULL; be=be->next) {
			struct rlib_break_fields *bf = be->data;
			type = RLIB_FILE_BREAK_FIELD;
			write(fd, &type, sizeof(gint32));
			write_xml_str(fd, bf->xml_value);
			type = RLIB_FILE_BREAK_FIELD+1;
			write(fd, &type, sizeof(gint32));
		}		
		type = RLIB_FILE_BREAK+1;
		write(fd, &type, sizeof(gint32));
	}
	type = RLIB_FILE_BREAKS + 1;
	write(fd, &type, sizeof(gint32));
}

gint save_report(struct rlib_report *rep, gchar *filename) {
	gint fd;
	gchar buf[MAXSTRLEN];
	
	unlink(filename);
	fd = open(filename, O_RDWR | O_CREAT, 0664);
	sprintf(buf, "RLIB %s                                ", VERSION);
	write(fd, buf, 20);
	
	write_xml_str(fd, rep->xml_font_size);
	write_xml_str(fd, rep->xml_orientation);
	write_xml_str(fd, rep->xml_top_margin); 
	write_xml_str(fd, rep->xml_left_margin);
	write_xml_str(fd, rep->xml_bottom_margin);
	write_xml_str(fd, rep->xml_pages_across);
	write_xml_str(fd, rep->xml_suppress_page_header_first_page);
	
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
