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
 
#include <SAPI.h>
#include <php.h>

#include "rlib.h"
#include "pcode.h"

#define FONTPOINT 	10.0
#define RLIB_GET_LINE(a) ((float)(a/72.0))
#define LENGTH_OF_LINE_ACCROSS_PAGE (r->landscape ? (11-(GET_MARGIN(r)->left_margin*2)) : (8.5-(GET_MARGIN(r)->left_margin*2)))

struct rgb COLOR_BLACK = {0, 0, 0};

float get_page_width(rlib *r) {
	if(!r->landscape)
		return (8.5);
	else
		return (11.0);
}

static float rlib_get_next_line(rlib *r, float x, float fp) {
	float f;
	if(r->landscape)
		f = (8.5 - (x + RLIB_GET_LINE(fp)));
	else
		f = (11.0 - (x + RLIB_GET_LINE(fp)));

	return f;
}

static void rlib_advance_line(rlib *r, int font_point) {
	r->position_top += RLIB_GET_LINE(font_point);
}

static void rlib_subtract_line(rlib *r, int font_point) {
	r->position_bottom -= RLIB_GET_LINE(font_point);
}

static float rlib_get_estimated_width(rlib *r, int len) {
	char buf[2];
	buf[0] = 'W';
	buf[1] = '\0';
	return (OUTPUT(r)->rlib_get_string_width(r, buf)*len);
}

static float advance_margin(rlib *r, float margin, int chars) {
	char buf[2];
	buf[0] = 'W';
	buf[1] = '\0';
	margin += (OUTPUT(r)->rlib_get_string_width(r, buf)*chars);
	return margin;
}

#define STATE_NONE		0
#define STATE_BGCOLOR	1

#define STATUS_NONE	0
#define STATUS_START 1
#define STATUS_STOP	2
	
static float rlib_output_text(rlib *r, int backwards, float left_origin, float bottom_orgin, struct rlib_line_extra_data *extra_data) {
	float rtn_width;
	char *text;
	
	text = extra_data->formatted_string;

	OUTPUT(r)->rlib_set_font_point(r, extra_data->font_point);
	
	if(extra_data->found_link)
		OUTPUT(r)->rlib_boxurl_start(r, left_origin, bottom_orgin, rlib_get_estimated_width(r, extra_data->width), RLIB_GET_LINE(extra_data->font_point), extra_data->link);

	if(extra_data->running_bgcolor_status & STATUS_START) 
		OUTPUT(r)->rlib_draw_cell_background_start(r, left_origin, bottom_orgin, extra_data->running_running_bg_total, RLIB_GET_LINE(extra_data->font_point), &extra_data->bgcolor);

	if(extra_data->found_color)
		OUTPUT(r)->rlib_set_fg_color(r, extra_data->color.r, extra_data->color.g, extra_data->color.b);

	OUTPUT(r)->rlib_print_text(r, left_origin, bottom_orgin+(extra_data->font_point/300.0), text, backwards, extra_data->col);

	rtn_width = extra_data->output_width;

	if(extra_data->found_color)
		OUTPUT(r)->rlib_set_fg_color(r, 0, 0, 0);

	if(extra_data->running_bgcolor_status & STATUS_STOP)
		OUTPUT(r)->rlib_draw_cell_background_end(r);	

	if(extra_data->found_link)
		OUTPUT(r)->rlib_boxurl_end(r);
		
	OUTPUT(r)->rlib_set_font_point(r, r->font_point);

	return rtn_width;
}

char *align_text(rlib *r, char *rtn, int len, char *src, long align, long width) {
	strcpy(rtn, src);

	if(!OUTPUT(r)->do_align)
		return rtn;

	if(align == RLIB_ALIGN_LEFT || width == -1) {
	} else {
		if(align == RLIB_ALIGN_RIGHT) {
			int x = width-strlen(src);
			memset(rtn, ' ', x);
			strcpy(rtn+x, src);
		}
		if(align == RLIB_ALIGN_CENTER) {
			int x = (width-strlen(src))/2;
			memset(rtn, ' ', x);
			strcpy(rtn+x, src);
		}
	
	}
		
	return rtn;
}
	
static int get_font_point(rlib *r, struct report_lines *rl) {
	if(rl->font_point == -1)
		return r->font_point;
	else
		return rl->font_point;
}	

float estimate_string_width_from_extra_data(rlib *r, struct rlib_line_extra_data *extra_data) {
	float rtn_width;
	OUTPUT(r)->rlib_set_font_point(r, extra_data->font_point);
	rtn_width = advance_margin(r, 0, extra_data->width);	
	return rtn_width;
}
	
	
void execute_pcodes_for_line(rlib *r, struct report_lines *rl, struct rlib_line_extra_data *extra_data) {
	int i=0;
	struct report_field *rf;
	struct report_text *rt;
	struct report_element *e = rl->e;
	struct rlib_pcode *tmp=NULL;
	char *text;
	
	for(; e != NULL; e=e->next) {
		if(e->type == REPORT_ELEMENT_FIELD) {
			char buf[MAXSTRLEN];

			rf = e->data;
			rlib_execute_pcode(r, &extra_data[i].rval_code, rf->code, NULL);	

			if(rf->link_code != NULL)	
				rlib_execute_pcode(r, &extra_data[i].rval_link, rf->link_code, NULL);

			tmp = NULL;

			if(rf->color_code != NULL)	
				tmp = rf->color_code;
			else if(rl->color_code != NULL)
				tmp = rl->color_code;
			if(tmp != NULL)
				rlib_execute_pcode(r, &extra_data[i].rval_color, tmp, NULL);

			tmp = NULL;

			if(rf->bgcolor_code != NULL)
				tmp = rf->bgcolor_code;
			else if(rl->bgcolor_code != NULL)
				tmp = rl->bgcolor_code;
			if(tmp != NULL)	
				rlib_execute_pcode(r, &extra_data[i].rval_bgcolor, tmp, NULL);

			rlib_format_string(r, rf, &extra_data[i].rval_code, buf);
			align_text(r, extra_data[i].formatted_string, MAXSTRLEN, buf, rf->align, rf->width);
			
			extra_data[i].width = rf->width;
			
			rlib_execute_pcode(r, &extra_data[i].rval_col, rf->col_code, NULL);
		}
		
		if(e->type == REPORT_ELEMENT_TEXT) {
			char buf[MAXSTRLEN];
			rt = e->data;
			if(rt->bgcolor_code != NULL)
				tmp = rt->bgcolor_code;
			else if(rl->bgcolor_code != NULL)
				tmp = rl->bgcolor_code;
			if(tmp != NULL)	
				rlib_execute_pcode(r, &extra_data[i].rval_bgcolor, tmp, NULL);	
				
			if(rt->color_code != NULL)	
				rlib_execute_pcode(r, &extra_data[i].rval_color, rt->color_code, NULL);

			if(rt->value == NULL)
				extra_data[i].formatted_string[0] = '\0';
			else
				strcpy(extra_data[i].formatted_string, rt->value);
				
			strcpy(buf, extra_data[i].formatted_string);
			align_text(r, extra_data[i].formatted_string, MAXSTRLEN, buf, rt->align, rt->width);
				
			extra_data[i].width = rt->width;
			rlib_execute_pcode(r, &extra_data[i].rval_col, rt->col_code, NULL);	
		}
		
		if(rl->font_point == -1)
			extra_data[i].font_point = r->font_point;
		else
			extra_data[i].font_point = rl->font_point;


		text = extra_data[i].formatted_string;

		if(text == NULL)
			text = "";

		if(extra_data[i].width == -1)
			extra_data[i].width = strlen(text);
		else {
			int slen = strlen(text);
			if(slen > extra_data[i].width)
				text[extra_data[i].width] = '\0';
			else if(slen < extra_data[i].width && MAXSTRLEN != slen) {
				int xx;
				for(xx=0;xx<extra_data[i].width-slen;xx++) {
					text[slen+xx] = ' ';
				}
				text[extra_data[i].width] = '\0';
			}
		}
				
		extra_data[i].found_bgcolor = FALSE;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_bgcolor))) {
			char *colorstring;
			if(!RLIB_VALUE_IS_STRING((&extra_data[i].rval_bgcolor))) {
				debugf("RLIB ENCOUNTERED AN ERROR PROCESSING THE BGCOLOR FOR THIS VALUE [%s].. BGCOLOR VALUE WAS NOT OF TYPE STRING\n", text);
			} else {
				char *idx;
				colorstring = RLIB_VALUE_GET_AS_STRING((&extra_data[i].rval_bgcolor));
				if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_code)) && RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_code)) && index(colorstring, ':')) {
					colorstring = estrdup(colorstring);
					idx = index(colorstring, ':');
					if(RLIB_VALUE_GET_AS_NUMBER((&extra_data[i].rval_code)) >= 0)
						idx[0] = '\0';
					else
						colorstring = idx+1;	
				}
				parsecolor(&extra_data[i].bgcolor, colorstring);
				extra_data[i].found_bgcolor = TRUE;
			}

		}
		
		extra_data[i].found_color = FALSE;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_color))) {
			char *colorstring;
			if(!RLIB_VALUE_IS_STRING((&extra_data[i].rval_color))) {
				debugf("RLIB ENCOUNTERED AN ERROR PROCESSING THE COLOR FOR THIS VALUE [%s].. COLOR VALUE WAS NOT OF TYPE STRING\n", text);
			} else {
				char *idx;
				colorstring = RLIB_VALUE_GET_AS_STRING((&extra_data[i].rval_color));
				if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_code)) && RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_code)) && index(colorstring, ':')) {
					colorstring = estrdup(colorstring);
					idx = index(colorstring, ':');
					if(RLIB_VALUE_GET_AS_NUMBER((&extra_data[i].rval_code)) >= 0)
						idx[0] = '\0';
					else
						colorstring = idx+1;	
				}
				parsecolor(&extra_data[i].color, colorstring);
				extra_data[i].found_color = TRUE;
			}
		}

		extra_data[i].col = 0;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_col))) {
			if(!RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_col))) {
				debugf("RLIB EXPECTS A expN FOR A COLUMN... text=[%s]\n", text);
			} else {
				extra_data[i].col = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER((&extra_data[i].rval_col)));
			}
		}
	
		extra_data[i].found_link = FALSE;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_link))) {
			if(!RLIB_VALUE_IS_STRING((&extra_data[i].rval_link))) {
				debugf("RLIB ENCOUNTERED AN ERROR PROCESSING THE LINK FOR THIS VALUE [%s].. LINK VALUE WAS NOT OF TYPE STRING [%d]\n", text, RLIB_VALUE_GET_TYPE((&extra_data[i].rval_link)));
			} else {
				extra_data[i].link = RLIB_VALUE_GET_AS_STRING((&extra_data[i].rval_link));
				if(extra_data[i].link != NULL && strcmp(extra_data[i].link, "")) {
					extra_data[i].found_link = TRUE;
				}
			}
		}
		
		extra_data[i].output_width = estimate_string_width_from_extra_data(r, &extra_data[i]);
		
		i++;
	}
}	

void find_stuff_in_common(rlib *r, struct rlib_line_extra_data *extra_data, int count) {
	int i;
	struct rlib_line_extra_data *e_ptr, *save_ptr, *previous_ptr;
	int state = STATE_NONE;
	
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
				if(!memcmp(&save_ptr->bgcolor, &e_ptr->bgcolor, sizeof(struct rgb))) {
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
		//save_ptr->running_running_bg_total += e_ptr->output_width;
 		e_ptr->running_bgcolor_status |= STATUS_STOP;
	}
}

static void print_detail_line_private(rlib *r, struct report_output_array *roa, int backwards) {
	struct report_element *e=NULL;
	int j=0;
	float margin=0, width=0;
	struct rlib_line_extra_data *extra_data;

	if(roa == NULL)
		return;

	r->current_line_number++;
	
	for(j=0;j<roa->count;j++) {
		struct report_output *ro = roa->data[j];
		margin = GET_MARGIN(r)->left_margin;
		
		if(ro->type == REPORT_PRESENTATION_DATA_LINE) {
			struct report_lines *rl = ro->data;
			int count=0;
			
			OUTPUT(r)->rlib_start_line(r, backwards);
			
			for(e = rl->e; e != NULL; e=e->next)
				count++;

			extra_data = ecalloc(sizeof(struct rlib_line_extra_data), count);
			execute_pcodes_for_line(r, rl, extra_data);
			find_stuff_in_common(r, extra_data, count);
			count = 0;

			for(e = rl->e; e != NULL; e=e->next) {
				if(e->type == REPORT_ELEMENT_FIELD) {
					struct report_field *rf = ((struct report_field *)e->data);
					rf->rval = &extra_data[count].rval_code;
					width = rlib_output_text(r, backwards, margin, rlib_get_next_line(r, backwards ? r->position_bottom : r->position_top, 
							get_font_point(r, rl)),  &extra_data[count]);
				}

				if(e->type == REPORT_ELEMENT_TEXT) {
						width = rlib_output_text(r, backwards, margin, rlib_get_next_line(r, backwards ? r->position_bottom : r->position_top, 
							get_font_point(r, rl)), &extra_data[count]);				
				}
				margin += width;
				count++;
			}

			efree(extra_data);

			if(backwards)
				rlib_subtract_line(r,get_font_point(r, rl));
			else
				rlib_advance_line(r, get_font_point(r, rl));
				
			OUTPUT(r)->rlib_end_line(r, backwards);	

		} else if(ro->type == REPORT_PRESENTATION_DATA_HR) {
			struct rlib_value rval2, *rval=&rval2;
			char *colorstring;
			struct report_horizontal_line *rhl = ro->data;
			rlib_execute_pcode(r, &rval2, rhl->bgcolor_code, NULL);
			if(!RLIB_VALUE_IS_STRING(rval)) {
				debugf("RLIB ENCOUNTERED AN ERROR PROCESSING THE BGCOLOR FOR A HR.. COLOR VALUE WAS NOT OF TYPE STRING\n");
			} else {
				struct rgb bgcolor;
				float font_point;
				float indent;
				float length;
				colorstring = RLIB_VALUE_GET_AS_STRING(rval);
				parsecolor(&bgcolor, colorstring);
				if(rhl->font_point == -1)
					font_point = r->font_point;
				else
					font_point = rhl->font_point;
//				OUTPUT(r)->rlib_start_line(r, backwards);	
				OUTPUT(r)->rlib_set_font_point(r, font_point);
				indent = rlib_get_estimated_width(r, rhl->realindent);			
				length = rlib_get_estimated_width(r, rhl->reallength);			
				OUTPUT(r)->rlib_set_font_point(r, r->font_point);
				if(length == 0)
					OUTPUT(r)->rlib_hr(r, backwards, GET_MARGIN(r)->left_margin+indent, rlib_get_next_line(r, r->position_top, rhl->realsize),
						 LENGTH_OF_LINE_ACCROSS_PAGE-indent, rhl->realsize, &bgcolor, indent, length);
				else
					OUTPUT(r)->rlib_hr(r, backwards, GET_MARGIN(r)->left_margin+indent, rlib_get_next_line(r, r->position_top, rhl->realsize),
						length, rhl->realsize, &bgcolor, indent, length);
//				OUTPUT(r)->rlib_end_line(r, backwards);	
				rlib_advance_line(r, rhl->realsize);

			}
		} else if(ro->type == REPORT_PRESENTATION_DATA_IMAGE) {
			struct rlib_value rval2, rval3, rval4, rval5, *rval_value=&rval2, *rval_width=&rval3, *rval_height=&rval4, *rval_type=&rval5;
			struct report_image *ri = ro->data;
			rlib_execute_pcode(r, &rval2, ri->value_code, NULL);
			rlib_execute_pcode(r, &rval5, ri->type_code, NULL);
			rlib_execute_pcode(r, &rval3, ri->width_code, NULL);
			rlib_execute_pcode(r, &rval4, ri->height_code, NULL);
			if(!RLIB_VALUE_IS_STRING(rval_value) || !RLIB_VALUE_IS_NUMBER(rval_width) || !RLIB_VALUE_IS_NUMBER(rval_height)) {
				debugf("RLIB ENCOUNTERED AN ERROR PROCESSING THE BGCOLOR FOR A IMAGE\n");
			} else {
				float height = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(rval_height));
				float width = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(rval_width));
				char *name = RLIB_VALUE_GET_AS_STRING(rval_value);
				char *type = RLIB_VALUE_GET_AS_STRING(rval_type);
				OUTPUT(r)->rlib_drawimage(r, GET_MARGIN(r)->left_margin, rlib_get_next_line(r, r->position_top, height), name, type, width, height);
			}
		}		
	}
}	

void print_detail_line(rlib *r, struct report_output_array *roa, int backwards) {
	OUTPUT(r)->rlib_start_output_section(r);
	print_detail_line_private(r, roa, backwards);
	OUTPUT(r)->rlib_end_output_section(r);
}


void hack_print_detail_lines(rlib *r) {
	struct report_element *e;

	OUTPUT(r)->rlib_start_output_section(r);

	if(r->reports[r->current_report]->breaks != NULL) {
		for(e = r->reports[r->current_report]->breaks; e != NULL; e=e->next) {
			struct report_break *rb = e->data;
			print_detail_line_private(r, rb->header, FALSE);
		}		
	}
	if(r->reports[r->current_report]->detail != NULL)
		print_detail_line_private(r, r->reports[r->current_report]->detail->fields, FALSE);
	OUTPUT(r)->rlib_end_output_section(r);
}

void rlib_init_page(rlib *r, char report_header) {
	r->position_top = GET_MARGIN(r)->top_margin;
	OUTPUT(r)->rlib_start_new_page(r);
	OUTPUT(r)->rlib_set_font_point(r, r->font_point);
	if(report_header)
		print_detail_line(r, r->reports[r->current_report]->report_header, FALSE);	
	
	print_detail_line(r, r->reports[r->current_report]->page_header, FALSE);
	if(r->reports[r->current_report]->detail != NULL)
		print_detail_line(r, r->reports[r->current_report]->detail->textlines, FALSE);		
	print_detail_line(r, r->reports[r->current_report]->page_footer, TRUE);

	OUTPUT(r)->rlib_init_end_page(r);
}

float get_output_size(rlib *r, struct report_output_array *roa) {
	int j;
	float total=0;
	for(j=0;j<roa->count;j++) {
		struct report_output *rd = roa->data[j];
		if(rd->type == REPORT_PRESENTATION_DATA_LINE) {
			struct report_lines *rl = rd->data;
			total += RLIB_GET_LINE(get_font_point(r, rl));
		} else if(rd->type == REPORT_PRESENTATION_DATA_HR) {
			struct report_horizontal_line *rhl = rd->data;
			total += RLIB_GET_LINE(rhl->realsize);		
		}
	}
	return total;
}

int will_this_fit(rlib *r, float total) {
	if(OUTPUT(r)->rlib_is_single_page(r))
		return TRUE;

	if(r->position_top+total > r->position_bottom)
		return 0;
	else
		return 1;			
}

int will_line_fit(rlib *r, struct report_output_array *roa) {
	if(OUTPUT(r)->rlib_is_single_page(r))
		return TRUE;
		
	return will_this_fit(r, get_output_size(r, roa));
}

void rlib_end_page_if_line_wont_fit(rlib *r, struct report_output_array *roa) {
	if(!will_line_fit(r,roa))
		OUTPUT(r)->rlib_end_page(r);
}

void rlib_print_report_footer(rlib *r) {
	if(r->reports[r->current_report]->report_footer != NULL) {
		rlib_end_page_if_line_wont_fit(r, r->reports[r->current_report]->report_footer);
		print_detail_line(r, r->reports[r->current_report]->report_footer, FALSE);
	}
}


int rlib_fetch_first_rows(rlib *r) {
	int i;
	for(i=0;i<r->results_count;i++)
		r->results[i].row = mysql_fetch_row(r->results[i].result);
	return 0;
}

void rlib_init_variables(rlib *r) {
	struct report_element *e;
	for(e = r->reports[r->current_report]->variables; e != NULL; e=e->next) {
		struct report_variable *rv = e->data;
		if(rv->type == REPORT_VARIABLE_EXPRESSION) {
			RLIB_VARIABLE_CA(rv) = emalloc(sizeof(struct count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_COUNT) {
			RLIB_VARIABLE_CA(rv) = emalloc(sizeof(struct count_amount));
			RLIB_VARIABLE_CA(rv)->count = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->count, 0);
		} else if(rv->type == REPORT_VARIABLE_SUM) {
			RLIB_VARIABLE_CA(rv) = emalloc(sizeof(struct count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_AVERAGE) {
			RLIB_VARIABLE_CA(rv) = emalloc(sizeof(struct count_amount));
			RLIB_VARIABLE_CA(rv)->count = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->count, 0);
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_LOWEST) {
			RLIB_VARIABLE_CA(rv) = emalloc(sizeof(struct count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_HIGHEST) {
			RLIB_VARIABLE_CA(rv) = emalloc(sizeof(struct count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		}
	}
	
}

void rlib_process_variables(rlib *r) {
	struct report_element *e;
	for(e = r->reports[r->current_report]->variables; e != NULL; e=e->next) {
		struct report_variable *rv = e->data;
		struct rlib_value *count = &RLIB_VARIABLE_CA(rv)->count;
		struct rlib_value *amount = &RLIB_VARIABLE_CA(rv)->amount;
		struct rlib_value execute_result, *er = &execute_result;
		if(rv->code != NULL)
			 rlib_execute_pcode(r, &execute_result, rv->code, NULL);
		if(rv->type == REPORT_VARIABLE_COUNT) {
			RLIB_VALUE_GET_AS_NUMBER(count) += RLIB_DECIMAL_PERCISION;
		} else if(rv->type == REPORT_VARIABLE_EXPRESSION) {
			if(RLIB_VALUE_IS_NUMBER(er))
				RLIB_VALUE_GET_AS_NUMBER(amount) = RLIB_VALUE_GET_AS_NUMBER(er);
			else
				debugf("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_EXPRESSION\n");
		} else if(rv->type == REPORT_VARIABLE_SUM) {
			if(RLIB_VALUE_IS_NUMBER(er))
				RLIB_VALUE_GET_AS_NUMBER(amount) += RLIB_VALUE_GET_AS_NUMBER(er);
			else
				debugf("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_SUM\n");
		} else if(rv->type == REPORT_VARIABLE_AVERAGE) {
			RLIB_VALUE_GET_AS_NUMBER(count) += RLIB_DECIMAL_PERCISION;
			if(RLIB_VALUE_IS_NUMBER(er))
				RLIB_VALUE_GET_AS_NUMBER(amount) += RLIB_VALUE_GET_AS_NUMBER(er);
			else
				debugf("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_AVERAGE\n");
		} else if(rv->type == REPORT_VARIABLE_LOWEST) {
			if(RLIB_VALUE_IS_NUMBER(er)) {
				if(RLIB_VALUE_GET_AS_NUMBER(er) < RLIB_VALUE_GET_AS_NUMBER(amount) || RLIB_VALUE_GET_AS_NUMBER(amount) == 0) //TODO: EVIL HACK
					RLIB_VALUE_GET_AS_NUMBER(amount) = RLIB_VALUE_GET_AS_NUMBER(er);
			} else
				debugf("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_LOWEST\n");
		} else if(rv->type == REPORT_VARIABLE_HIGHEST) {
			if(RLIB_VALUE_IS_NUMBER(er)) {
				if(RLIB_VALUE_GET_AS_NUMBER(er) > RLIB_VALUE_GET_AS_NUMBER(amount) || RLIB_VALUE_GET_AS_NUMBER(amount) == 0) //TODO: EVIL HACK
					RLIB_VALUE_GET_AS_NUMBER(amount) = RLIB_VALUE_GET_AS_NUMBER(er);
			} else
				debugf("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_HIGHEST\n");
		}
	}
	
}

int make_report(rlib *r) {
	int i = 0;
	int report = 0;
	MYSQL_ROW last;
	
	if(r->format == RLIB_FORMAT_HTML)
		rlib_html_new_output_filter(r);
	else if(r->format == RLIB_FORMAT_TXT)
		rlib_txt_new_output_filter(r);
	else if(r->format == RLIB_FORMAT_CSV)
		rlib_csv_new_output_filter(r);
	else
		rlib_pdf_new_output_filter(r);
	
	r->current_font_point = -1;

	OUTPUT(r)->rlib_set_fg_color(r, -1, -1, -1);
	OUTPUT(r)->rlib_set_bg_color(r, -1, -1, -1);

	r->current_page_number = 1;
	r->current_report = 0;
	r->current_result = 0;
	r->start_of_new_report = TRUE;
	r->results[r->current_result].row = NULL;

	OUTPUT(r)->rlib_init_output(r);
	rlib_fetch_first_rows(r);

	if(r->reports[r->current_report]->defaultResult != NULL && r->reports[r->current_report]->defaultResult[0] != '\0') {
		int index;
		index = rlib_lookup_result(r, r->reports[r->current_report]->defaultResult);
		if(index >= 0)
			r->current_result = index;
	}

	for(report=0;report<r->reports_count;report++) {
		r->current_report = report;
		if(report > 0) {
			if(r->reports[r->current_report]->mainloop_query != -1)
				r->current_result = r->reports[r->current_report]->mainloop_query;
		}

		rlib_resolve_fields(r);
		if(r->reports[r->current_report]->fontsize != -1)
			r->font_point = r->reports[r->current_report]->fontsize;
			
		rlib_init_variables(r);
		rlib_init_page(r, TRUE);		
		OUTPUT(r)->rlib_begin_text(r);
		while (r->results[r->current_result].row) {
			MYSQL_ROW temp=NULL;
			rlib_handle_break_headers(r);
			
			if(r->reports[r->current_report]->detail != NULL) {
				if(!will_line_fit(r, r->reports[r->current_report]->detail->fields)) {
					OUTPUT(r)->rlib_end_page(r);
					rlib_force_break_headers(r);
				}
			}
			rlib_process_variables(r);
			if(OUTPUT(r)->do_break) {
				if(r->reports[r->current_report]->detail != NULL)
					print_detail_line(r, r->reports[r->current_report]->detail->fields, FALSE);
			} else
				hack_print_detail_lines(r);

			r->detail_line_count++;
			i++;
			last = temp;
			temp = mysql_fetch_row(r->results[r->current_result].result);
			r->results[r->current_result].last_row = r->results[r->current_result].row;
			if(temp == NULL) {
				r->results[r->current_result].row = temp;
				rlib_handle_break_footers(r);
				break;
			} else
				r->results[r->current_result].row = temp;

			rlib_handle_break_footers(r);

		}


		r->results[r->current_result].row = r->results[r->current_result].last_row;
		if(r->results[r->current_result].row != NULL)
			rlib_print_report_footer(r);
	
		if(report+1 < r->reports_count) {
			OUTPUT(r)->rlib_end_text(r);
			r->current_page_number++;
			r->start_of_new_report = TRUE;
			r->current_line_number = 1;
			r->detail_line_count = 0;
			r->font_point = FONTPOINT;
		}


	}	
	OUTPUT(r)->rlib_end_text(r);
	
	return 0;
}

int rlib_finalize(rlib *r) {
	OUTPUT(r)->rlib_finalize_private(r);
	return 0;
}

int rlib_spool(rlib *r) {
	OUTPUT(r)->rlib_spool_private(r);
	return 0;
}
