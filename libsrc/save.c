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
 
#include "rlib.h"

#define XML_STR	500
#define OUTPUTS	600

struct packet {
	gint32 type;
	gint32 size;
};

void write_xml_str(gint fd, gchar *str) {
	gint len;
	struct packet p;
	if(str != NULL)
		len = strlen(str);
	else
		len = 0;
	p.type = XML_STR;
	p.size = len;
	write(fd, &p, sizeof(struct packet));
	if(len > 0)
		write(fd, str, len);
	return;
}

void write_image(gint fd, struct report_image *ri) {
	write_xml_str(fd, ri->value);
	write_xml_str(fd, ri->type);
	write_xml_str(fd, ri->width);
	write_xml_str(fd, ri->height);
}

void write_hr(gint fd, struct report_horizontal_line *hr) {
	write_xml_str(fd, hr->bgcolor);
	write_xml_str(fd, hr->size);
	write_xml_str(fd, hr->indent);
	write_xml_str(fd, hr->length);
	write_xml_str(fd, hr->font_size);
	write_xml_str(fd, hr->suppress);
}

void write_field(gint fd, struct report_field *rf) {
	write_xml_str(fd, rf->value);
	write_xml_str(fd, rf->xml_align);
	write_xml_str(fd, rf->bgcolor);
	write_xml_str(fd, rf->color);
	write_xml_str(fd, rf->xml_width);
	write_xml_str(fd, rf->format);
	write_xml_str(fd, rf->link);
	write_xml_str(fd, rf->col);
	write_xml_str(fd, rf->xml_wrapchars);
	write_xml_str(fd, rf->xml_maxlines);
}

void write_text(gint fd, struct report_text *rt) {
	write_xml_str(fd, rt->value);
	write_xml_str(fd, rt->xml_align);
	write_xml_str(fd, rt->color);
	write_xml_str(fd, rt->bgcolor);
	write_xml_str(fd, rt->xml_width);
	write_xml_str(fd, rt->col);
}

void write_line(gint fd, struct report_lines *rl) {
	struct report_element *e;
	gint32 type;
	write_xml_str(fd, rl->bgcolor);
	write_xml_str(fd, rl->color);
	write_xml_str(fd, rl->font_size);
	write_xml_str(fd, rl->suppress);

	for(e=rl->e; e != NULL; e=e->next) {
		type = e->type;
		write(fd, &type, sizeof(gint32));		
		if(e->type == REPORT_ELEMENT_FIELD) {
			struct report_field *rf = e->data;
			write_field(fd, rf);
		} else if(e->type == REPORT_ELEMENT_TEXT) {
			struct report_text *rt = e->data;
			write_text(fd, rt);
		}
	}

}

void write_roa(gint fd, struct report_output_array *roa) {
	gint j;
	gint32 type;
	write_xml_str(fd, roa->xml_page);
	for(j=0;j<roa->count;j++) {
		struct report_output *ro = roa->data[j];
		type = ro->type;
		write(fd, &type, sizeof(gint32));
		if(ro->type == REPORT_PRESENTATION_DATA_LINE) {			
			struct report_lines *rl = ro->data;	
			write_line(fd, rl);
		} else if(ro->type == REPORT_PRESENTATION_DATA_HR) {
			struct report_horizontal_line *hr = ro->data;
			write_hr(fd, hr);
		} else if(ro->type == REPORT_PRESENTATION_DATA_IMAGE) {
			struct report_image *ri = ro->data;
			write_image(fd, ri);
		}

	}	

}

void write_output(gint fd, struct report_element *e) {
	struct report_output_array *roa;
	gint32 type = OUTPUTS;
	write(fd, &type, sizeof(gint32));
	for(; e != NULL; e=e->next) {
		roa = e->data;
		write_roa(fd, roa);
	}
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
	write_xml_str(fd, rep->xml_pages_accross);
	write_xml_str(fd, rep->xml_suppress_page_header_first_page);
	
	write_output(fd, rep->report_header);
	
	close(fd);
	return 0;
}
