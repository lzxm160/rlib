/*
 *  Copyright (C) 2003-2004 SICOM Systems, INC.
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
 *
 * $Id$
 * 
 * This module generates a report from the information stored in the current
 * report object.
 * The main entry point is called once at report generation time for each
 * report defined in the rlib object.
 *
 */
 
#include <stdlib.h>
#include <string.h>
#include <langinfo.h>

#include "config.h"
#include "rlib.h"
#include "pcode.h"
#include "rlib_input.h"

#define STATE_NONE		0
#define STATE_BGCOLOR	1

#define STATUS_NONE	0
#define STATUS_START 1
#define STATUS_STOP	2

static gchar *truefalses[] = {
	"no",
	"yes",
	"false",
	"true",
	NULL
};

static gchar *aligns[] = {
	"left",
	"right",
	"center",
	NULL
};

static struct rlib_paper paper[] = {
	{RLIB_PAPER_LETTER,612, 792, "LETTER"},
	{RLIB_PAPER_LEGAL, 612, 1008, "LEGAL"},
	{RLIB_PAPER_A4, 595, 842, "A4"},
	{RLIB_PAPER_B5, 499, 708, "B5"},
	{RLIB_PAPER_C5, 459, 649, "C5"},
	{RLIB_PAPER_DL, 312, 624, "DL"},
	{RLIB_PAPER_EXECUTIVE, 522, 756, "EXECUTIVE"},
	{RLIB_PAPER_COMM10, 297, 684, "COMM10"},
	{RLIB_PAPER_MONARCH, 279, 540, "MONARCH"},
	{RLIB_PAPER_FILM35MM, 528, 792, "FILM35MM"},
	{0},
};

struct rlib_paper * rlib_layout_get_paper(rlib *r, gint paper_type) {
	gint i;
	for(i=0;paper[i].type != 0;i++)
		if(paper[i].type == paper_type)
			return &paper[i];
	return NULL;
}

struct rlib_paper * rlib_layout_get_paper_by_name(rlib *r, gchar *paper_name) {
	gint i;
	if(paper_name == NULL)
		return NULL;
		
	for(i=0;paper[i].type != 0;i++)
		if(!strcasecmp(paper[i].name, paper_name))
			return &paper[i];
	return NULL;
}

static gfloat rlib_layout_advance_horizontal_margin(rlib *r, gfloat margin, gint chars) {
	gchar buf[2];
	buf[0] = 'W';
	buf[1] = '\0';
	margin += (OUTPUT(r)->get_string_width(r, buf)*chars);
	return margin;
}

gfloat rlib_layout_estimate_string_width_from_extra_data(rlib *r, struct rlib_line_extra_data *extra_data) {
	gfloat rtn_width;
	OUTPUT(r)->set_font_point(r, extra_data->font_point);
	rtn_width = rlib_layout_advance_horizontal_margin(r, 0, extra_data->width);	
	return rtn_width;
}

static const gchar *rlib_encode_text(rlib *r, const gchar *text) {
	gchar *result = "";

	if (text == NULL) {
		r_error("rlib_encode_text called with NULL text");
		result = "!ERR_ENC1";
	} else {
		result = (gchar *) rlib_char_encoder_encode(r->current_output_encoder, text);
		if (result == NULL) {
			r_error("encode returned NULL result input was[%s], len=%d", text, strlen(text));
			result = "!ERR_ENC2";
		}
	}
	return result;
}

gfloat rlib_layout_get_next_line(rlib *r, struct rlib_part *part, gfloat position, gfloat foint_point) {
	if(part->landscape)
		return ((part->paper->width/RLIB_PDF_DPI) - (position + RLIB_GET_LINE(foint_point)));
	else
		return ((part->paper->height/RLIB_PDF_DPI) - (position + RLIB_GET_LINE(foint_point)));
}


static gint rlib_check_is_not_suppressed(rlib *r, struct rlib_pcode *code) {
	gint result = FALSE;
	if (rlib_execute_as_int_inlist(r, code, &result, truefalses))
		result &= 1;
	return result? FALSE : TRUE;
}

static gfloat rlib_layout_get_report_width(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	if(report == NULL) {
		if(part->landscape)
			return (part->paper->height/RLIB_PDF_DPI) - (part->left_margin*2);
		else
			return (part->paper->width/RLIB_PDF_DPI) - (part->left_margin*2);
	} else {
		return report->page_width;
	}
}

static gfloat rlib_layout_estimate_string_width(rlib *r, gint len) {
	gchar buf[2];
	buf[0] = 'W';
	buf[1] = '\0';
	return (OUTPUT(r)->get_string_width(r, buf)*len);
}

static gfloat rlib_layout_output_extras_start(rlib *r, struct rlib_part *part, gint backwards, gfloat left_origin, gfloat bottom_orgin, 
struct rlib_line_extra_data *extra_data) {
	
	if(extra_data->is_bold)
		OUTPUT(r)->start_bold(r);
	if(extra_data->is_italics)
		OUTPUT(r)->start_italics(r);

	if(extra_data->found_link)
		OUTPUT(r)->start_boxurl(r, part, left_origin, bottom_orgin, rlib_layout_estimate_string_width(r, extra_data->width), 
			RLIB_GET_LINE(extra_data->font_point), extra_data->link);

	if(extra_data->running_bgcolor_status & STATUS_START) 
		OUTPUT(r)->start_draw_cell_background(r, left_origin, bottom_orgin, extra_data->running_running_bg_total, 
			RLIB_GET_LINE(extra_data->font_point), &extra_data->bgcolor);

	return extra_data->output_width;
}

static gfloat rlib_layout_output_extras_end(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_orgin, 
struct rlib_line_extra_data *extra_data) {


	if(extra_data->running_bgcolor_status & STATUS_STOP)
		OUTPUT(r)->end_draw_cell_background(r);	

	if(extra_data->found_link)
		OUTPUT(r)->end_boxurl(r);

	if(extra_data->is_italics)
		OUTPUT(r)->end_italics(r);
	if(extra_data->is_bold)
		OUTPUT(r)->end_bold(r);
		
	return extra_data->output_width;
}

static gfloat rlib_layout_output_extras(rlib *r, struct rlib_part *part, gint backwards, gfloat left_origin, gfloat bottom_orgin, 
struct rlib_line_extra_data *extra_data) {
	if(extra_data->found_link)
		OUTPUT(r)->start_boxurl(r, part, left_origin, bottom_orgin, rlib_layout_estimate_string_width(r, extra_data->width), 
			RLIB_GET_LINE(extra_data->font_point), extra_data->link);

	if(extra_data->running_bgcolor_status & STATUS_START) 
		OUTPUT(r)->start_draw_cell_background(r, left_origin, bottom_orgin, extra_data->running_running_bg_total, 
			RLIB_GET_LINE(extra_data->font_point), &extra_data->bgcolor);

	if(extra_data->running_bgcolor_status & STATUS_STOP)
		OUTPUT(r)->end_draw_cell_background(r);	

	if(extra_data->found_link)
		OUTPUT(r)->end_boxurl(r);
		
	return extra_data->output_width;
}

static gfloat rlib_layout_text_from_extra_data(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_orgin, struct rlib_line_extra_data *extra_data) {
	gfloat rtn_width;
	gchar *text = extra_data->formatted_string;

	OUTPUT(r)->set_font_point(r, extra_data->font_point);
	if(extra_data->found_color)
		OUTPUT(r)->set_fg_color(r, extra_data->color.r, extra_data->color.g, extra_data->color.b);
	if(extra_data->is_bold)
		OUTPUT(r)->start_bold(r);
	if(extra_data->is_italics)
		OUTPUT(r)->start_italics(r);
	OUTPUT(r)->print_text(r, left_origin, bottom_orgin+(extra_data->font_point/300.0), (gchar *) rlib_encode_text(r, text), backwards, extra_data->col);
	rtn_width = extra_data->output_width;
	if(extra_data->found_color)
		OUTPUT(r)->set_fg_color(r, 0, 0, 0);
	if(extra_data->is_bold)
		OUTPUT(r)->end_bold(r);
	if(extra_data->is_italics)
		OUTPUT(r)->end_italics(r);
	OUTPUT(r)->set_font_point(r, r->font_point);
	return rtn_width;
}


static gfloat rlib_layout_text_string(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_orgin, struct rlib_line_extra_data *extra_data, 
gchar *text) {
	gfloat rtn_width;
	OUTPUT(r)->set_font_point(r, extra_data->font_point);
	if(extra_data->found_color)
		OUTPUT(r)->set_fg_color(r, extra_data->color.r, extra_data->color.g, extra_data->color.b);
	if(extra_data->is_bold)
		OUTPUT(r)->start_bold(r);
	if(extra_data->is_italics)
		OUTPUT(r)->start_italics(r);
	OUTPUT(r)->print_text(r, left_origin, bottom_orgin+(extra_data->font_point/300.0), (gchar *) rlib_encode_text(r, text), backwards, extra_data->col);
	rtn_width = extra_data->output_width;
	if(extra_data->found_color)
		OUTPUT(r)->set_fg_color(r, 0, 0, 0);
	if(extra_data->is_bold)
		OUTPUT(r)->end_bold(r);
	if(extra_data->is_italics)
		OUTPUT(r)->end_italics(r);
	OUTPUT(r)->set_font_point(r, r->font_point);
	return rtn_width;
}

static void rlib_advance_vertical_position(rlib *r, gfloat *rlib_position, gint font_point) {
	*rlib_position += RLIB_GET_LINE(font_point);
}

static void rlib_layout_execute_pcodes_for_line(rlib *r, struct rlib_report_lines *rl, struct rlib_line_extra_data *extra_data) {
	gint i=0;
	gchar *text;
	struct rlib_report_field *rf;
	struct rlib_report_literal *rt;
	struct rlib_element *e = rl->e;
	struct rlib_value line_rval_color;
	struct rlib_value line_rval_bgcolor;
	struct rlib_value line_rval_bold;
	struct rlib_value line_rval_italics;

	RLIB_VALUE_TYPE_NONE(&line_rval_color);
	RLIB_VALUE_TYPE_NONE(&line_rval_bgcolor);
	RLIB_VALUE_TYPE_NONE(&line_rval_bold);
	RLIB_VALUE_TYPE_NONE(&line_rval_italics);
	if(rl->color_code != NULL)
		rlib_execute_pcode(r, &line_rval_color, rl->color_code, NULL);
	if(rl->bgcolor_code != NULL)
		rlib_execute_pcode(r, &line_rval_bgcolor, rl->bgcolor_code, NULL);
	if(rl->bold_code != NULL) {
		rlib_execute_pcode(r, &line_rval_bold, rl->bold_code, NULL);
		
		if(RLIB_VALUE_GET_TYPE((&line_rval_bold)) > 99999) {
			rlib_pcode_dump(rl->bold_code, 0);
			exit(1);
		}
	}
	if(rl->italics_code != NULL)
		rlib_execute_pcode(r, &line_rval_italics, rl->italics_code, NULL);

	for(; e != NULL; e=e->next) {
		RLIB_VALUE_TYPE_NONE(&extra_data[i].rval_bgcolor);
		RLIB_VALUE_TYPE_NONE(&extra_data[i].rval_bold);
		RLIB_VALUE_TYPE_NONE(&extra_data[i].rval_italics);
		if (e->type == RLIB_ELEMENT_FIELD) {
			gchar buf[MAXSTRLEN];
			rf = e->data;
			if (rf == NULL) 
				r_error("report_field is NULL ... will crash");
			else if 
				(rf->code == NULL) r_error("There is no code for field");
			
			rlib_execute_pcode(r, &extra_data[i].rval_code, rf->code, NULL);	
			if(rf->link_code != NULL) {	
				rlib_execute_pcode(r, &extra_data[i].rval_link, rf->link_code, NULL);
			}
			if(rf->color_code != NULL) {
				rlib_execute_pcode(r, &extra_data[i].rval_color, rf->color_code, NULL);
			} else if(rl->color_code != NULL) {
				extra_data[i].rval_color = line_rval_color;
				rlib_value_dup_contents(&extra_data[i].rval_color);
			}
			if(rf->bgcolor_code != NULL) {
				rlib_execute_pcode(r, &extra_data[i].rval_bgcolor, rf->bgcolor_code, NULL);
			} else if(rl->bgcolor_code != NULL) {
				extra_data[i].rval_bgcolor = line_rval_bgcolor;
				rlib_value_dup_contents(&extra_data[i].rval_bgcolor);
			}
			if(rf->bold_code != NULL) {
				rlib_execute_pcode(r, &extra_data[i].rval_bold, rf->bold_code, NULL);
			} else if(rl->bold_code != NULL) {
				extra_data[i].rval_bold = line_rval_bold;
				rlib_value_dup_contents(&extra_data[i].rval_bold);
			}
			if(rf->italics_code != NULL) {
				rlib_execute_pcode(r, &extra_data[i].rval_italics, rf->italics_code, NULL);
			} else if(rl->italics_code != NULL) {
				extra_data[i].rval_italics = line_rval_italics;
				rlib_value_dup_contents(&extra_data[i].rval_italics);
			}

			if (rf->align_code) {
				gint t;
				if (rlib_execute_as_int_inlist(r, rf->align_code, &t, aligns)) {
					if (t < 3) rf->align = t;
				}
			}
			if (rf->width_code) {
				gint t;
			if (rlib_execute_as_int(r, rf->width_code, &t)) { 
					rf->width = t;
				}
			}
			rlib_format_string(r, rf, &extra_data[i].rval_code, buf);
			align_text(r, extra_data[i].formatted_string, MAXSTRLEN, buf, rf->align, rf->width);
			
			extra_data[i].width = rf->width;
			
			rlib_execute_pcode(r, &extra_data[i].rval_col, rf->col_code, NULL);
		} else if(e->type == RLIB_ELEMENT_LITERAL) {
			gchar buf[MAXSTRLEN];
			rt = e->data;
			if(rt->color_code != NULL)	
				rlib_execute_pcode(r, &extra_data[i].rval_color, rt->color_code, NULL);
			else if(rl->color_code != NULL) {
				extra_data[i].rval_color = line_rval_color;
				rlib_value_dup_contents(&extra_data[i].rval_color);
			}
			if(rt->bgcolor_code != NULL)
				rlib_execute_pcode(r, &extra_data[i].rval_bgcolor, rt->bgcolor_code, NULL);
			else if(rl->bgcolor_code != NULL) {
				extra_data[i].rval_bgcolor = line_rval_bgcolor;
				rlib_value_dup_contents(&extra_data[i].rval_bgcolor);				
			}
			if(rt->bold_code != NULL)	
				rlib_execute_pcode(r, &extra_data[i].rval_bold, rt->bold_code, NULL);
			else if(rl->bold_code != NULL) {
				extra_data[i].rval_bold = line_rval_bold;
				rlib_value_dup_contents(&extra_data[i].rval_bold);
			}
			if(rt->italics_code != NULL)	
				rlib_execute_pcode(r, &extra_data[i].rval_italics, rt->italics_code, NULL);
			else if(rl->italics_code != NULL) {
				extra_data[i].rval_italics = line_rval_italics;
				rlib_value_dup_contents(&extra_data[i].rval_italics);
			}

			if(rt->value == NULL)
				extra_data[i].formatted_string[0] = '\0';
			else
				strcpy(extra_data[i].formatted_string, rt->value);
				
			if (rt->align_code) {
				gint t;
				if (rlib_execute_as_int_inlist(r, rt->align_code, &t, aligns)) {
					if (t < 3) rt->align = t;
				}
			}
			if (rt->width_code) {
				gint t;
				if (rlib_execute_as_int(r, rt->width_code, &t)) {
					rt->width = t;
				}
			}
			strcpy(buf, extra_data[i].formatted_string);
			align_text(r, extra_data[i].formatted_string, MAXSTRLEN, buf, rt->align, rt->width);
				
			extra_data[i].width = rt->width;
			rlib_execute_pcode(r, &extra_data[i].rval_col, rt->col_code, NULL);	
		} else {
			r_error("Line has invalid content");
		}
		if(rl->font_point == -1)
			extra_data[i].font_point = r->font_point;
		else
			extra_data[i].font_point = rl->font_point;
		text = extra_data[i].formatted_string;

		if(text == NULL)
			text = "";
		if(extra_data[i].width == -1)
			extra_data[i].width = r_charcount(text);
		else {
			gint slen = r_charcount(text);
			if(slen > extra_data[i].width)
				*r_ptrfromindex(text, extra_data[i].width) = '\0';
			else if(slen < extra_data[i].width && MAXSTRLEN != slen) {
				gint lim = extra_data[i].width - slen;
				gchar *ptr = r_ptrfromindex(text, slen);
				while (lim-- > 0) {
					*ptr++ = ' ';
				}
				*ptr = '\0';
			}
		}
		extra_data[i].found_bgcolor = FALSE;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_bgcolor))) {
			gchar *colorstring, *ocolor;
			if(!RLIB_VALUE_IS_STRING((&extra_data[i].rval_bgcolor))) {
				r_error("RLIB ENCOUNTERED AN ERROR PROCESSING THE BGCOLOR FOR THIS VALUE [%s].. BGCOLOR VALUE WAS NOT OF TYPE STRING\n", text);
			} else {
				gchar *idx;
				ocolor = colorstring = g_strdup(RLIB_VALUE_GET_AS_STRING((&extra_data[i].rval_bgcolor)));
				if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_code)) && RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_code)) && strchr(colorstring, ':')) {
					colorstring = g_strdup(colorstring);
					idx = strchr(colorstring, ':');
					if(RLIB_VALUE_GET_AS_NUMBER((&extra_data[i].rval_code)) >= 0)
						idx[0] = '\0';
					else
						colorstring = idx+1;	
				}
				parsecolor(&extra_data[i].bgcolor, colorstring);
				extra_data[i].found_bgcolor = TRUE;
				g_free(ocolor);
			}

		}
		extra_data[i].found_color = FALSE;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_color))) {
			gchar *colorstring, *ocolor;
			if(!RLIB_VALUE_IS_STRING((&extra_data[i].rval_color))) {
				r_error("RLIB ENCOUNTERED AN ERROR PROCESSING THE COLOR FOR THIS VALUE [%s].. COLOR VALUE WAS NOT OF TYPE STRING\n", text);
			} else {
				gchar *idx;
				ocolor = colorstring = g_strdup(RLIB_VALUE_GET_AS_STRING((&extra_data[i].rval_color)));
				if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_code)) && RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_code)) && strchr(colorstring, ':')) {
					idx = strchr(colorstring, ':');
					if(RLIB_VALUE_GET_AS_NUMBER((&extra_data[i].rval_code)) >= 0)
						idx[0] = '\0';
					else
						colorstring = idx+1;	
				}
				parsecolor(&extra_data[i].color, colorstring);
				extra_data[i].found_color = TRUE;
				g_free(ocolor);
			}
		}
		extra_data[i].is_bold = FALSE;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_bold))) {
			if(!RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_bold))) {
				r_error("RLIB ENCOUNTERED AN ERROR PROCESSING BOLD FOR THIS VALUE [%s].. BOLD VALUE WAS NOT OF TYPE NUMBER TYPE=%d\n", text, RLIB_VALUE_GET_TYPE((&extra_data[i].rval_bold)));
			} else {
				if(RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER((&extra_data[i].rval_bold))))
					extra_data[i].is_bold = TRUE;
			}
		}
		extra_data[i].is_italics = FALSE;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_italics))) {
			if(!RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_italics))) {
				r_error("RLIB ENCOUNTERED AN ERROR PROCESSING ITALICS FOR THIS VALUE [%s].. ITALICS VALUE WAS NOT OF TYPE NUMBER\n", text);
			} else {
				if(RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER((&extra_data[i].rval_italics))))
					extra_data[i].is_italics = TRUE;
			}
		}

		extra_data[i].col = 0;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_col))) {
			if(!RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_col))) {
				r_error("RLIB EXPECTS A expN FOR A COLUMN... text=[%s]\n", text);
			} else {
				extra_data[i].col = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER((&extra_data[i].rval_col)));
			}
		}
		extra_data[i].found_link = FALSE;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_link))) {
			if(!RLIB_VALUE_IS_STRING((&extra_data[i].rval_link))) {
				r_error("RLIB ENCOUNTERED AN ERROR PROCESSING THE LINK FOR THIS VALUE [%s].. LINK VALUE WAS NOT OF TYPE STRING [%d]\n", text, RLIB_VALUE_GET_TYPE((&extra_data[i].rval_link)));
			} else {
				extra_data[i].link = RLIB_VALUE_GET_AS_STRING((&extra_data[i].rval_link));
				if(extra_data[i].link != NULL && strcmp(extra_data[i].link, "")) {
					extra_data[i].found_link = TRUE;
				}
			}
		}
		extra_data[i].output_width = rlib_layout_estimate_string_width_from_extra_data(r, &extra_data[i]);		
		i++;
	}
	rlib_value_free(&line_rval_color);
	rlib_value_free(&line_rval_bgcolor);
	rlib_value_free(&line_rval_bold);
	rlib_value_free(&line_rval_italics);
}	

static void rlib_layout_find_common_properties_in_a_line(rlib *r, struct rlib_line_extra_data *extra_data, gint count) {
	gint i = 0;
	struct rlib_line_extra_data *e_ptr = NULL, *save_ptr = NULL, *previous_ptr = NULL;
	gint state = STATE_NONE;
	
	previous_ptr = &extra_data[i];
	for(i=0;i<count;i++) {
		e_ptr = &extra_data[i];
		if(e_ptr->found_bgcolor) {
			if(state == STATE_NONE) {
				save_ptr = e_ptr;
				state = STATE_BGCOLOR;
				e_ptr->running_bgcolor_status |= STATUS_START;
				e_ptr->running_running_bg_total = e_ptr->output_width;
			} else {
				if(!memcmp(&save_ptr->bgcolor, &e_ptr->bgcolor, sizeof(struct rlib_rgb))) {
					save_ptr->running_running_bg_total += e_ptr->output_width;
				} else {
					save_ptr = e_ptr;
					previous_ptr->running_bgcolor_status |= STATUS_STOP;
					e_ptr->running_bgcolor_status |= STATUS_START;
					e_ptr->running_running_bg_total = e_ptr->output_width;
				}
			}
		} else {
			if(state == STATE_BGCOLOR) {
				previous_ptr->running_bgcolor_status |= STATUS_STOP;
				state = STATE_NONE;
			}
		}
		previous_ptr = e_ptr;
	}
	if(state == STATE_BGCOLOR) {
 		e_ptr->running_bgcolor_status |= STATUS_STOP;
	}
}

static gint rlib_layout_report_output_array(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_report_output_array *roa, gint backwards, gint page) {
	struct rlib_element *e=NULL;
	gint j=0;
	gfloat margin=0, width=0;
	gfloat *rlib_position;
	struct rlib_line_extra_data *extra_data;
	gint output_count = 0;
	gfloat my_left_margin;

	if(roa == NULL)
		return 0;
	
	r->current_line_number++;
	
	if(report != NULL) {
		if(backwards)
			rlib_position = &report->position_bottom[page-1];
		else
			rlib_position = &report->position_top[page-1];	
		my_left_margin = report->left_margin;
	} else {
		if(backwards)
			rlib_position = &part->position_bottom[page-1];
		else
			rlib_position = &part->position_top[page-1];
		my_left_margin = part->left_margin;
	}

	for(j=0;j<roa->count;j++) {
		struct rlib_report_output *ro = roa->data[j];
		margin = my_left_margin;

		if(ro->type == RLIB_REPORT_PRESENTATION_DATA_LINE) {
			struct rlib_report_lines *rl = ro->data;
			gint count=0;
			
			
			if(rlib_check_is_not_suppressed(r, rl->suppress_code)) {
				output_count++;
				OUTPUT(r)->start_line(r, backwards);

				for(e = rl->e; e != NULL; e=e->next)
					count++;

				extra_data = g_new0(struct rlib_line_extra_data, count);
				rlib_layout_execute_pcodes_for_line(r, rl, extra_data);
				rlib_layout_find_common_properties_in_a_line(r, extra_data, count);
				count = 0;

				if(OUTPUT(r)->do_grouptext) {
					gchar buf[MAXSTRLEN];
					gfloat fun_width=0;
					gint start_count=-1;
					for(e = rl->e; e != NULL; e=e->next) {
						if(e->type == RLIB_ELEMENT_FIELD) {
							struct rlib_report_field *rf = ((struct rlib_report_field *)e->data);
							rf->rval = &extra_data[count].rval_code;
							width = rlib_layout_output_extras(r, part, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)),  
								&extra_data[count]);
						}

						if(e->type == RLIB_ELEMENT_LITERAL) {
							width = rlib_layout_output_extras(r, part, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);
						}
						margin += width;
						count++;
					}
					count=0;
					margin = my_left_margin;				
					buf[0] = 0;
					width = 0;
					start_count = -1;
					for(e = rl->e; e != NULL; e=e->next) {
						if(extra_data[count].found_color == FALSE && extra_data[count].is_bold == FALSE && extra_data[count].is_italics == FALSE) {
							if(start_count == -1)
								start_count = count;
							sprintf(buf, "%s%s", buf, extra_data[count].formatted_string);
							fun_width += extra_data[count].output_width;
						} else {
							if(start_count != -1) {
								rlib_layout_text_string(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)),  
									&extra_data[start_count], buf);
								start_count = -1;
								margin += fun_width;
								fun_width = 0;
								buf[0] = 0;
							}
							width = rlib_layout_text_from_extra_data(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)),  
								&extra_data[count]);
							margin += width;

						}
						count++;					
					}
					if(start_count != -1) {
						width += fun_width;
						rlib_layout_text_string(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)), 
							&extra_data[start_count], buf);
					}
				} else {
					for(e = rl->e; e != NULL; e=e->next) {
						if(e->type == RLIB_ELEMENT_FIELD) {
							struct rlib_report_field *rf = ((struct rlib_report_field *)e->data);
							rf->rval = &extra_data[count].rval_code;
							rlib_layout_output_extras_start(r, part, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)),  
								&extra_data[count]);
							width = rlib_layout_text_from_extra_data(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);
							rlib_layout_output_extras_end(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)),  
								&extra_data[count]);
						}

						if(e->type == RLIB_ELEMENT_LITERAL) {
							rlib_layout_output_extras_start(r, part, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);
							width = rlib_layout_text_from_extra_data(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);				
							rlib_layout_output_extras_end(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);
						}
						margin += width;
						count++;
					}
				}
				rlib_advance_vertical_position(r, rlib_position, get_font_point(r, rl));

				OUTPUT(r)->end_line(r, backwards);	

				count=0;
				for(e = rl->e; e != NULL; e=e->next) {
					rlib_value_free(&extra_data[count].rval_code);
					rlib_value_free(&extra_data[count].rval_link);
					rlib_value_free(&extra_data[count].rval_bgcolor);
					rlib_value_free(&extra_data[count].rval_color);
					rlib_value_free(&extra_data[count].rval_col);
					rlib_value_free(&extra_data[count].rval_bold);
					rlib_value_free(&extra_data[count].rval_italics);
					count++;
				}

				g_free(extra_data);
			}
		} else if(ro->type == RLIB_REPORT_PRESENTATION_DATA_HR) {
			gchar *colorstring;
			struct rlib_value rval2, *rval=&rval2;
			struct rlib_report_horizontal_line *rhl = ro->data;
			if(rlib_check_is_not_suppressed(r, rhl->suppress_code)) {
				rlib_execute_pcode(r, &rval2, rhl->bgcolor_code, NULL);
				if(!RLIB_VALUE_IS_STRING(rval)) {
					r_error("RLIB ENCOUNTERED AN ERROR PROCESSING THE BGCOLOR FOR A HR.. COLOR VALUE WAS NOT OF TYPE STRING\n");
				} else {
					gfloat font_point;
					gfloat indent;
					gfloat length;
					struct rlib_rgb bgcolor;
					output_count++;
					colorstring = RLIB_VALUE_GET_AS_STRING(rval);
					parsecolor(&bgcolor, colorstring);
					if(rhl->font_point == -1)
						font_point = r->font_point;
					else
						font_point = rhl->font_point;
					OUTPUT(r)->set_font_point(r, font_point);
					indent = rlib_layout_estimate_string_width(r, rhl->indent);			
					length = rlib_layout_estimate_string_width(r, rhl->length);			
					OUTPUT(r)->set_font_point(r, r->font_point);

					if(length == 0)
						OUTPUT(r)->hr(r, backwards, my_left_margin+indent, rlib_layout_get_next_line(r, part, *rlib_position, 
							rhl->size),rlib_layout_get_report_width(r, part, report)-indent, rhl->size, &bgcolor, indent, length);
					else
						OUTPUT(r)->hr(r, backwards, my_left_margin+indent, rlib_layout_get_next_line(r, part, *rlib_position, rhl->size),
							length, rhl->size, &bgcolor, indent, length);

					rlib_advance_vertical_position(r, rlib_position, rhl->size);
					rlib_value_free(rval);
				}
			}
		} else if(ro->type == RLIB_REPORT_PRESENTATION_DATA_IMAGE) {
			struct rlib_value rval2, rval3, rval4, rval5, *rval_value=&rval2, *rval_width=&rval3, *rval_height=&rval4, *rval_type=&rval5;
			struct rlib_report_image *ri = ro->data;
			rlib_execute_pcode(r, &rval2, ri->value_code, NULL);
			rlib_execute_pcode(r, &rval5, ri->type_code, NULL);
			rlib_execute_pcode(r, &rval3, ri->width_code, NULL);
			rlib_execute_pcode(r, &rval4, ri->height_code, NULL);
			if(!RLIB_VALUE_IS_STRING(rval_value) || !RLIB_VALUE_IS_NUMBER(rval_width) || !RLIB_VALUE_IS_NUMBER(rval_height)) {
				r_error("RLIB ENCOUNTERED AN ERROR PROCESSING THE BGCOLOR FOR A IMAGE\n");
			} else {
				gfloat height = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(rval_height));
				gfloat width = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(rval_width));
				gchar *name = RLIB_VALUE_GET_AS_STRING(rval_value);
				gchar *type = RLIB_VALUE_GET_AS_STRING(rval_type);
				output_count++;
				OUTPUT(r)->drawimage(r, my_left_margin, rlib_layout_get_next_line(r, part, *rlib_position, height), name, 
					type, width, height);
				rlib_value_free(rval_value);
				rlib_value_free(rval_width);
				rlib_value_free(rval_height);
				rlib_value_free(rval_type);
			}
		}
	}
	return output_count;
}	

static gint rlib_layout_report_outputs_across_pages(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_element *report_outputs, gint backwards) {
	struct rlib_report_output_array *roa;
	gint page;
	gint i;
	gint output_count = 0;
	for(; report_outputs != NULL; report_outputs=report_outputs->next) {
		roa = report_outputs->data;
		page = roa->page;
		if(page >= 1) {
			OUTPUT(r)->set_working_page(r, part, roa->page);
			output_count += rlib_layout_report_output_array(r, part, report, roa, backwards, roa->page);
		} else {
			for(i=0;i<part->pages_across;i++) {
				OUTPUT(r)->set_working_page(r, part, i+1);
				output_count = rlib_layout_report_output_array(r, part, report, roa, backwards, i+1);			
			}
		}
	}
	return output_count;
}

gint rlib_layout_report_output(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_element *e, gint backwards) {
	gint output_count = 0;
	OUTPUT(r)->start_output_section(r);
	output_count = rlib_layout_report_outputs_across_pages(r, part, report, e, backwards);
	OUTPUT(r)->end_output_section(r);
	return output_count;
}

gint rlib_layout_report_output_with_break_headers(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	struct rlib_element *e;
	gint output_count = 0;
	OUTPUT(r)->start_output_section(r);

	if(report->breaks != NULL) {
		for(e = report->breaks; e != NULL; e=e->next) {
			struct rlib_report_break *rb = e->data;
			output_count += rlib_layout_report_outputs_across_pages(r, part, report, rb->header, FALSE);
		}		
	}
	rlib_layout_report_outputs_across_pages(r, part, report, report->detail.fields, FALSE);
	OUTPUT(r)->end_output_section(r);
	return output_count;
}


gfloat rlib_layout_get_page_width(rlib *r, struct rlib_part *part) {
	if(!part->landscape)
		return (part->paper->width/RLIB_PDF_DPI);
	else
		return (part->paper->height/RLIB_PDF_DPI);
}


void rlib_layout_report_footer(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	gint i;
	if(report->report_footer != NULL) {
		for(i=0;i<report->pages_across;i++)
			rlib_end_page_if_line_wont_fit(r, part, report, report->report_footer);
		rlib_layout_report_output(r, part, report, report->report_footer, FALSE);
	}
}


void rlib_layout_init_report_page(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	rlib_layout_report_output(r, part, report, report->detail.headers, FALSE);
}

gint rlib_layout_end_page(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	if(report->raw_page_number < r->current_page_number) {
		OUTPUT(r)->end_page_again(r, part, report);
		report->raw_page_number++;
		OUTPUT(r)->set_raw_page(r, part, report->raw_page_number);
	} else {
		OUTPUT(r)->end_page(r, part);
		if(report->font_size != -1)
			r->font_point = report->font_size;		
		rlib_layout_init_part_page(r, part, FALSE);
		report->raw_page_number++;
	}
	rlib_set_report_from_part(r, part, report, 0);
	rlib_layout_init_report_page(r, part, report);
	return TRUE;
}

void rlib_layout_init_part_page(rlib *r, struct rlib_part *part, gboolean first) {
	gint i;
	if(part->font_size != -1)
		r->font_point = part->font_size;

	for(i=0;i<part->pages_across;i++) {
		part->position_top[i] = part->top_margin;
		part->bottom_size[i] = get_outputs_size(r, part->page_footer, i);
	}		
	r->current_font_point = -1;
	OUTPUT(r)->start_new_page(r, part);
	OUTPUT(r)->set_font_point(r, r->font_point);
	
	if(first)
		rlib_layout_report_output(r, part, NULL, part->report_header, FALSE);
	rlib_layout_report_output(r, part, NULL, part->page_header, FALSE);

	for(i=0; i<part->pages_across; i++)
		part->position_bottom[i] -= part->bottom_size[i];

	rlib_layout_report_output(r, part, NULL, part->page_footer, TRUE);

	for(i=0; i<part->pages_across; i++) {
		part->position_bottom[i] -= part->bottom_size[i];

	}

	OUTPUT(r)->init_end_page(r);
}
