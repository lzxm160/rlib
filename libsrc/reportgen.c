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
#include <string.h>

#include "rlib.h"
#include "pcode.h"
#include "rlib_input.h"

#define FONTPOINT 	10.0
#define RLIB_GET_LINE(a) ((float)(a/72.0))
#define LENGTH_OF_LINE_ACCROSS_PAGE (r->landscape ? (11-(GET_MARGIN(r)->left_margin*2)) : (8.5-(GET_MARGIN(r)->left_margin*2)))

struct rgb COLOR_BLACK = {0, 0, 0};

gfloat get_page_width(rlib *r) {
	if(!r->landscape)
		return (8.5);
	else
		return (11.0);
}

static gfloat rlib_get_next_line(rlib *r, gfloat x, gfloat fp) {
	gfloat f;
	if(r->landscape)
		f = (8.5 - (x + RLIB_GET_LINE(fp)));
	else
		f = (11.0 - (x + RLIB_GET_LINE(fp)));

	return f;
}

static void rlib_advance_line(rlib *r, gfloat *rlib_position, gint font_point) {
	*rlib_position += RLIB_GET_LINE(font_point);
}

static gfloat rlib_get_estimated_width(rlib *r, gint len) {
	gchar buf[2];
	buf[0] = 'W';
	buf[1] = '\0';
	return (OUTPUT(r)->rlib_get_string_width(r, buf)*len);
}

static gfloat advance_margin(rlib *r, gfloat margin, gint chars) {
	gchar buf[2];
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

static gfloat rlib_output_extras_start(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_orgin, 
struct rlib_line_extra_data *extra_data) {
	if(extra_data->found_link)
		OUTPUT(r)->rlib_boxurl_start(r, left_origin, bottom_orgin, rlib_get_estimated_width(r, extra_data->width), 
			RLIB_GET_LINE(extra_data->font_point), extra_data->link);

	if(extra_data->running_bgcolor_status & STATUS_START) 
		OUTPUT(r)->rlib_draw_cell_background_start(r, left_origin, bottom_orgin, extra_data->running_running_bg_total, 
			RLIB_GET_LINE(extra_data->font_point), &extra_data->bgcolor);

	return extra_data->output_width;
}

static gfloat rlib_output_extras_end(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_orgin, 
struct rlib_line_extra_data *extra_data) {
	if(extra_data->running_bgcolor_status & STATUS_STOP)
		OUTPUT(r)->rlib_draw_cell_background_end(r);	

	if(extra_data->found_link)
		OUTPUT(r)->rlib_boxurl_end(r);
		
	return extra_data->output_width;
}


static gfloat rlib_output_extras(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_orgin, 
struct rlib_line_extra_data *extra_data) {
	if(extra_data->found_link)
		OUTPUT(r)->rlib_boxurl_start(r, left_origin, bottom_orgin, rlib_get_estimated_width(r, extra_data->width), 
			RLIB_GET_LINE(extra_data->font_point), extra_data->link);

	if(extra_data->running_bgcolor_status & STATUS_START) 
		OUTPUT(r)->rlib_draw_cell_background_start(r, left_origin, bottom_orgin, extra_data->running_running_bg_total, 
			RLIB_GET_LINE(extra_data->font_point), &extra_data->bgcolor);

	if(extra_data->running_bgcolor_status & STATUS_STOP)
		OUTPUT(r)->rlib_draw_cell_background_end(r);	

	if(extra_data->found_link)
		OUTPUT(r)->rlib_boxurl_end(r);
		
	return extra_data->output_width;
}
	
static gfloat rlib_output_text(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_orgin, 
struct rlib_line_extra_data *extra_data) {
	gfloat rtn_width;
	gchar *text;
	
	text = extra_data->formatted_string;

	OUTPUT(r)->rlib_set_font_point(r, extra_data->font_point);
	
	if(extra_data->found_color)
		OUTPUT(r)->rlib_set_fg_color(r, extra_data->color.r, extra_data->color.g, extra_data->color.b);

	OUTPUT(r)->rlib_print_text(r, left_origin, bottom_orgin+(extra_data->font_point/300.0), text, backwards, extra_data->col);

	rtn_width = extra_data->output_width;

	if(extra_data->found_color)
		OUTPUT(r)->rlib_set_fg_color(r, 0, 0, 0);

	OUTPUT(r)->rlib_set_font_point(r, r->font_point);

	return rtn_width;
}

static gfloat rlib_output_text_text(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_orgin, 
struct rlib_line_extra_data *extra_data, gchar *text) {
	gfloat rtn_width;
	OUTPUT(r)->rlib_set_font_point(r, extra_data->font_point);
	
	if(extra_data->found_color)
		OUTPUT(r)->rlib_set_fg_color(r, extra_data->color.r, extra_data->color.g, extra_data->color.b);

	OUTPUT(r)->rlib_print_text(r, left_origin, bottom_orgin+(extra_data->font_point/300.0), text, backwards, extra_data->col);

	rtn_width = extra_data->output_width;

	if(extra_data->found_color)
		OUTPUT(r)->rlib_set_fg_color(r, 0, 0, 0);

	OUTPUT(r)->rlib_set_font_point(r, r->font_point);

	return rtn_width;
}

gchar *align_text(rlib *r, gchar *rtn, gint len, gchar *src, gint align, gint width) {
	strcpy(rtn, src);

	if(!OUTPUT(r)->do_align)
		return rtn;

	if(align == RLIB_ALIGN_LEFT || width == -1) {
	} else {
		if(align == RLIB_ALIGN_RIGHT) {
			gint x = width-strlen(src);
			if(x > 0) {
				memset(rtn, ' ', x);
				strcpy(rtn+x, src);
			}
		}
		if(align == RLIB_ALIGN_CENTER) {
			gint x = (width-strlen(src))/2;
			if(x > 0) {
				memset(rtn, ' ', x);
				strcpy(rtn+x, src);
			}
		}
	}
	return rtn;
}
	
static gint get_font_point(rlib *r, struct report_lines *rl) {
	if(rl->font_point == -1)
		return r->font_point;
	else
		return rl->font_point;
}	

gfloat estimate_string_width_from_extra_data(rlib *r, struct rlib_line_extra_data *extra_data) {
	gfloat rtn_width;
	OUTPUT(r)->rlib_set_font_point(r, extra_data->font_point);
	rtn_width = advance_margin(r, 0, extra_data->width);	
	return rtn_width;
}
	
void execute_pcodes_for_line(rlib *r, struct report_lines *rl, struct rlib_line_extra_data *extra_data) {
	gint i=0;
	gchar *text;
	struct report_field *rf;
	struct report_text *rt;
	struct report_element *e = rl->e;
	struct rlib_value line_rval_color;
	struct rlib_value line_rval_bgcolor;

	RLIB_VALUE_TYPE_NONE(&line_rval_color);
	RLIB_VALUE_TYPE_NONE(&line_rval_bgcolor);

	if(rl->color_code != NULL)
		rlib_execute_pcode(r, &line_rval_color, rl->color_code, NULL);
	if(rl->bgcolor_code != NULL)
		rlib_execute_pcode(r, &line_rval_bgcolor, rl->bgcolor_code, NULL);

	for(; e != NULL; e=e->next) {
		RLIB_VALUE_TYPE_NONE(&extra_data[i].rval_bgcolor);
		if(e->type == REPORT_ELEMENT_FIELD) {
			gchar buf[MAXSTRLEN];

			rf = e->data;

			rlib_execute_pcode(r, &extra_data[i].rval_code, rf->code, NULL);	

			if(rf->link_code != NULL)	
				rlib_execute_pcode(r, &extra_data[i].rval_link, rf->link_code, NULL);

			if(rf->color_code != NULL)
				rlib_execute_pcode(r, &extra_data[i].rval_color, rf->color_code, NULL);
			else if(rl->color_code != NULL) {
				extra_data[i].rval_color = line_rval_color;
				rlib_value_dup_contents(&extra_data[i].rval_color);
			}

			if(rf->bgcolor_code != NULL)
				rlib_execute_pcode(r, &extra_data[i].rval_bgcolor, rf->bgcolor_code, NULL);
			else if(rl->bgcolor_code != NULL) {
				extra_data[i].rval_bgcolor = line_rval_bgcolor;
				rlib_value_dup_contents(&extra_data[i].rval_bgcolor);
			}
			
			rlib_format_string(r, rf, &extra_data[i].rval_code, buf);
			align_text(r, extra_data[i].formatted_string, MAXSTRLEN, buf, rf->align, rf->width);
			
			extra_data[i].width = rf->width;
			
			rlib_execute_pcode(r, &extra_data[i].rval_col, rf->col_code, NULL);
		}
		if(e->type == REPORT_ELEMENT_TEXT) {
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
			gint slen = strlen(text);
			if(slen > extra_data[i].width)
				text[extra_data[i].width] = '\0';
			else if(slen < extra_data[i].width && MAXSTRLEN != slen) {
				gint xx;
				for(xx=0;xx<extra_data[i].width-slen;xx++) {
					text[slen+xx] = ' ';
				}
				text[extra_data[i].width] = '\0';
			}
		}
				
		extra_data[i].found_bgcolor = FALSE;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_bgcolor))) {
			gchar *colorstring, *ocolor;
			if(!RLIB_VALUE_IS_STRING((&extra_data[i].rval_bgcolor))) {
				rlogit("RLIB ENCOUNTERED AN ERROR PROCESSING THE BGCOLOR FOR THIS VALUE [%s].. BGCOLOR VALUE WAS NOT OF TYPE STRING\n", text);
			} else {
				gchar *idx;
				ocolor = colorstring = g_strdup(RLIB_VALUE_GET_AS_STRING((&extra_data[i].rval_bgcolor)));
				if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_code)) && RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_code)) && index(colorstring, ':')) {
					colorstring = g_strdup(colorstring);
					idx = index(colorstring, ':');
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
				rlogit("RLIB ENCOUNTERED AN ERROR PROCESSING THE COLOR FOR THIS VALUE [%s].. COLOR VALUE WAS NOT OF TYPE STRING\n", text);
			} else {
				gchar *idx;
				ocolor = colorstring = g_strdup(RLIB_VALUE_GET_AS_STRING((&extra_data[i].rval_color)));
				if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_code)) && RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_code)) && index(colorstring, ':')) {
					idx = index(colorstring, ':');
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

		extra_data[i].col = 0;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_col))) {
			if(!RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_col))) {
				rlogit("RLIB EXPECTS A expN FOR A COLUMN... text=[%s]\n", text);
			} else {
				extra_data[i].col = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER((&extra_data[i].rval_col)));
			}
		}
	
		extra_data[i].found_link = FALSE;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_link))) {
			if(!RLIB_VALUE_IS_STRING((&extra_data[i].rval_link))) {
				rlogit("RLIB ENCOUNTERED AN ERROR PROCESSING THE LINK FOR THIS VALUE [%s].. LINK VALUE WAS NOT OF TYPE STRING [%d]\n", text, RLIB_VALUE_GET_TYPE((&extra_data[i].rval_link)));
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
	
	rlib_value_free(&line_rval_color);
	rlib_value_free(&line_rval_bgcolor);
	
}	

void find_stuff_in_common(rlib *r, struct rlib_line_extra_data *extra_data, gint count) {
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
 		e_ptr->running_bgcolor_status |= STATUS_STOP;
	}
}

static gint rlib_check_is_not_suppressed(rlib *r, struct rlib_pcode *code) {
	struct rlib_value suppress;

	if(code != NULL) {
		rlib_execute_pcode(r, &suppress, code, NULL);

		if(!RLIB_VALUE_IS_NONE((&suppress))) {
			if(!RLIB_VALUE_IS_STRING((&suppress))) {
				rlogit("RLIB ENCOUNTERED AN ERROR PROCESSING SURPRESS... VALUE WAS NOT OF TYPE STRING\n");
			} else {
				gchar *value = RLIB_VALUE_GET_AS_STRING((&suppress));
				if(value != NULL) {
					if(strcasecmp(value, "yes") == 0) {
						rlib_value_free(&suppress);
						return FALSE;
					}
				}
				rlib_value_free(&suppress);
			}	
		}
	}
	
	return TRUE;
}

#if 0
/**
 * break the string txt into a RVector of individual strings that will fit
 * the width. Use RVector_size to count the # of lines returned.
 */
RVector *wrap_memo_lines(gchar *txt, gint width, const gchar *wrapchars) {
	gint len;
	gchar *tptr, *endptr, *ptr;
	RVector *v = RVector_new();
	
	do {
		if (strlen(txt) < width) {
			RVector_add(v, g_strdup(txt));
			break;
		} else {
			endptr = ptr = txt + width;
			while (ptr > txt) {
				if ((tptr = strchr(wrapchars, *ptr))) {
					len = ptr - txt;
					tptr = g_malloc(len + 1);
					strncpy(tptr, txt, len);
					tptr[len] = '\0';
					RVector_add(v, tptr);
					endptr = ptr;
				}
				--ptr;
			}
			txt = endptr;
		}
	} while (TRUE);
	return v;
}


/**
 * Frees all memory allocated for a memo lines vector
 */
void free_memo_lines(RVector *v) {
	gint i, lim = RVector_size(v);

	for (i = 0; i < lim; ++i) {
		g_free(RVector_get(v, i));
	}
	RVector_free(v);
}
#endif


gint calc_memo_lines(struct report_lines *rl) {
	struct report_element *e;
//	int hasmemo;
	gint nlines = 0;
//	RVector *v;
	
	for (e = rl->e; e != NULL; e = e->next) {
		if (e->type == REPORT_ELEMENT_FIELD) {
			//
//			if (e->xml_maxlines)
//			v = wrapMemoLines(
//			if (e->maxlines != 1) {
//				hasmemo = TRUE;
//			}
		}
	}
	return nlines;
}

static gint print_report_output_private(rlib *r, struct report_output_array *roa, gint backwards, gint page) {
	struct report_element *e=NULL;
	gint j=0;
	gfloat margin=0, width=0;
	gfloat *rlib_position;
	struct rlib_line_extra_data *extra_data;
	gint output_count = 0;

	if(roa == NULL)
		return 0;
	
	r->current_line_number++;
	
	if(backwards)
		rlib_position = &r->reports[r->current_report]->position_bottom[page-1];
	else
		rlib_position = &r->reports[r->current_report]->position_top[page-1];
		
	for(j=0;j<roa->count;j++) {
		struct report_output *ro = roa->data[j];
		margin = GET_MARGIN(r)->left_margin;

		if(ro->type == REPORT_PRESENTATION_DATA_LINE) {
			struct report_lines *rl = ro->data;
			gint count=0;
			
			
			if(rlib_check_is_not_suppressed(r, rl->suppress_code)) {
				output_count++;
				OUTPUT(r)->rlib_start_line(r, backwards);

				for(e = rl->e; e != NULL; e=e->next)
					count++;

				extra_data = g_new0(struct rlib_line_extra_data, count);
				execute_pcodes_for_line(r, rl, extra_data);
				find_stuff_in_common(r, extra_data, count);
				count = 0;

				if(OUTPUT(r)->do_grouptext) {
					gchar buf[MAXSTRLEN];
					gfloat fun_width=0;
					gint start_count=-1;
					for(e = rl->e; e != NULL; e=e->next) {
						if(e->type == REPORT_ELEMENT_FIELD) {
							struct report_field *rf = ((struct report_field *)e->data);
							rf->rval = &extra_data[count].rval_code;
							width = rlib_output_extras(r, backwards, margin, rlib_get_next_line(r, *rlib_position, get_font_point(r, rl)),  
								&extra_data[count]);
						}

						if(e->type == REPORT_ELEMENT_TEXT) {
							width = rlib_output_extras(r, backwards, margin, rlib_get_next_line(r, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);
						}
						margin += width;
						count++;
					}
					count=0;
					margin = GET_MARGIN(r)->left_margin;				
					buf[0] = 0;
					width = 0;
					start_count = -1;
					for(e = rl->e; e != NULL; e=e->next) {
						if(!extra_data[count].found_color) {
							if(start_count == -1)
								start_count = count;
							sprintf(buf, "%s%s", buf, extra_data[count].formatted_string);
							fun_width += extra_data[count].output_width;
						} else {
							if(start_count != -1) {
								rlib_output_text_text(r, backwards, margin, rlib_get_next_line(r, *rlib_position, get_font_point(r, rl)),  
									&extra_data[start_count], buf);
								start_count = -1;
								margin += fun_width;
								fun_width = 0;
								buf[0] = 0;
							}
							width = rlib_output_text(r, backwards, margin, rlib_get_next_line(r, *rlib_position, get_font_point(r, rl)),  
								&extra_data[count]);
							margin += width;

						}
						count++;					
					}
					if(start_count != -1) {
						width += fun_width;
						rlib_output_text_text(r, backwards, margin, rlib_get_next_line(r, *rlib_position, get_font_point(r, rl)), 
							&extra_data[start_count], buf);
					}
				} else {
					for(e = rl->e; e != NULL; e=e->next) {
						if(e->type == REPORT_ELEMENT_FIELD) {
							struct report_field *rf = ((struct report_field *)e->data);
							rf->rval = &extra_data[count].rval_code;
							rlib_output_extras_start(r, backwards, margin, rlib_get_next_line(r, *rlib_position, get_font_point(r, rl)),  
								&extra_data[count]);
							width = rlib_output_text(r, backwards, margin, rlib_get_next_line(r, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);
							rlib_output_extras_end(r, backwards, margin, rlib_get_next_line(r, *rlib_position, get_font_point(r, rl)),  
								&extra_data[count]);
						}

						if(e->type == REPORT_ELEMENT_TEXT) {
							rlib_output_extras_start(r, backwards, margin, rlib_get_next_line(r, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);
							width = rlib_output_text(r, backwards, margin, rlib_get_next_line(r, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);				
							rlib_output_extras_end(r, backwards, margin, rlib_get_next_line(r, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);
						}
						margin += width;
						count++;
					}
				}
				rlib_advance_line(r, rlib_position, get_font_point(r, rl));

				OUTPUT(r)->rlib_end_line(r, backwards);	

				count=0;
				for(e = rl->e; e != NULL; e=e->next) {
					rlib_value_free(&extra_data[count].rval_code);
					rlib_value_free(&extra_data[count].rval_link);
					rlib_value_free(&extra_data[count].rval_bgcolor);
					rlib_value_free(&extra_data[count].rval_color);
					rlib_value_free(&extra_data[count].rval_col);
					count++;
				}

				g_free(extra_data);
			}
		} else if(ro->type == REPORT_PRESENTATION_DATA_HR) {
			gchar *colorstring;
			struct rlib_value rval2, *rval=&rval2;
			struct report_horizontal_line *rhl = ro->data;
			if(rlib_check_is_not_suppressed(r, rhl->suppress_code)) {
				rlib_execute_pcode(r, &rval2, rhl->bgcolor_code, NULL);
				if(!RLIB_VALUE_IS_STRING(rval)) {
					rlogit("RLIB ENCOUNTERED AN ERROR PROCESSING THE BGCOLOR FOR A HR.. COLOR VALUE WAS NOT OF TYPE STRING\n");
				} else {
					gfloat font_point;
					gfloat indent;
					gfloat length;
					struct rgb bgcolor;
					output_count++;
					colorstring = RLIB_VALUE_GET_AS_STRING(rval);
					parsecolor(&bgcolor, colorstring);
					if(rhl->font_point == -1)
						font_point = r->font_point;
					else
						font_point = rhl->font_point;
					OUTPUT(r)->rlib_set_font_point(r, font_point);
					indent = rlib_get_estimated_width(r, rhl->realindent);			
					length = rlib_get_estimated_width(r, rhl->reallength);			
					OUTPUT(r)->rlib_set_font_point(r, r->font_point);

					if(length == 0)
						OUTPUT(r)->rlib_hr(r, backwards, GET_MARGIN(r)->left_margin+indent, rlib_get_next_line(r, *rlib_position, 
							rhl->realsize),LENGTH_OF_LINE_ACCROSS_PAGE-indent, rhl->realsize, &bgcolor, indent, length);
					else
						OUTPUT(r)->rlib_hr(r, backwards, GET_MARGIN(r)->left_margin+indent, rlib_get_next_line(r, *rlib_position, rhl->realsize),
							length, rhl->realsize, &bgcolor, indent, length);

					rlib_advance_line(r, rlib_position, rhl->realsize);
					rlib_value_free(rval);
				}
			}
		} else if(ro->type == REPORT_PRESENTATION_DATA_IMAGE) {
			struct rlib_value rval2, rval3, rval4, rval5, *rval_value=&rval2, *rval_width=&rval3, *rval_height=&rval4, *rval_type=&rval5;
			struct report_image *ri = ro->data;
			rlib_execute_pcode(r, &rval2, ri->value_code, NULL);
			rlib_execute_pcode(r, &rval5, ri->type_code, NULL);
			rlib_execute_pcode(r, &rval3, ri->width_code, NULL);
			rlib_execute_pcode(r, &rval4, ri->height_code, NULL);
			if(!RLIB_VALUE_IS_STRING(rval_value) || !RLIB_VALUE_IS_NUMBER(rval_width) || !RLIB_VALUE_IS_NUMBER(rval_height)) {
				rlogit("RLIB ENCOUNTERED AN ERROR PROCESSING THE BGCOLOR FOR A IMAGE\n");
			} else {
				gfloat height = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(rval_height));
				gfloat width = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(rval_width));
				gchar *name = RLIB_VALUE_GET_AS_STRING(rval_value);
				gchar *type = RLIB_VALUE_GET_AS_STRING(rval_type);
				output_count++;
				OUTPUT(r)->rlib_drawimage(r, GET_MARGIN(r)->left_margin, rlib_get_next_line(r, *rlib_position, height), name, 
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

gint print_report_outputs_private(rlib *r, struct report_element *e, gint backwards) {
	struct report_output_array *roa;
	gint page;
	gint i;
	gint output_count = 0;
	for(; e != NULL; e=e->next) {
		roa = e->data;
		page = roa->page;
		if(page >= 1) {
			OUTPUT(r)->rlib_set_working_page(r, roa->page);
			output_count += print_report_output_private(r, roa, backwards, roa->page);
		} else {
			for(i=0;i<r->reports[r->current_report]->pages_accross;i++) {
				OUTPUT(r)->rlib_set_working_page(r, i+1);
				output_count = print_report_output_private(r, roa, backwards, i+1);			
			}
		}
	}
	return output_count;
}

gint print_report_output(rlib *r, struct report_element *e, gint backwards) {
	gint output_count = 0;
	OUTPUT(r)->rlib_start_output_section(r);
	output_count = print_report_outputs_private(r, e, backwards);
	OUTPUT(r)->rlib_end_output_section(r);
	return output_count;
}


gint hack_print_report_outputs(rlib *r) {
	struct report_element *e;
	gint output_count = 0;
	OUTPUT(r)->rlib_start_output_section(r);

	if(r->reports[r->current_report]->breaks != NULL) {
		for(e = r->reports[r->current_report]->breaks; e != NULL; e=e->next) {
			struct report_break *rb = e->data;
			output_count += print_report_outputs_private(r, rb->header, FALSE);
		}		
	}
	print_report_outputs_private(r, r->reports[r->current_report]->detail.fields, FALSE);
	OUTPUT(r)->rlib_end_output_section(r);
	return output_count;
}

void rlib_handle_page_footer(rlib *r) {
	gint i;

	for(i=0; i < r->reports[r->current_report]->pages_accross; i++) {
		r->reports[r->current_report]->bottom_size[i] = get_outputs_size(r, r->reports[r->current_report]->page_footer, i);
		r->reports[r->current_report]->position_bottom[i] -= r->reports[r->current_report]->bottom_size[i];
	}

	print_report_output(r, r->reports[r->current_report]->page_footer, TRUE);
	
	for(i=0; i < r->reports[r->current_report]->pages_accross; i++)
		r->reports[r->current_report]->position_bottom[i] -= r->reports[r->current_report]->bottom_size[i];
}

void rlib_init_page(rlib *r, gchar report_header) {
	gint i;
	for(i=0;i<r->reports[r->current_report]->pages_accross;i++)
		r->reports[r->current_report]->position_top[i] = GET_MARGIN(r)->top_margin;
	r->current_font_point = -1;
	OUTPUT(r)->rlib_start_new_page(r);
	OUTPUT(r)->rlib_set_font_point(r, r->font_point);
	if(report_header)
		print_report_output(r, r->reports[r->current_report]->report_header, FALSE);	
	
	if(!(r->current_page_number == 1 && r->reports[r->current_report]->suppress_page_header_first_page == TRUE))
		print_report_output(r, r->reports[r->current_report]->page_header, FALSE);
	print_report_output(r, r->reports[r->current_report]->detail.textlines, FALSE);		
	rlib_handle_page_footer(r);

	OUTPUT(r)->rlib_init_end_page(r);
}

gfloat get_output_size(rlib *r, struct report_output_array *roa) {
	gint j;
	gfloat total=0;
	for(j=0;j<roa->count;j++) {
		struct report_output *rd = roa->data[j];
		if(rd->type == REPORT_PRESENTATION_DATA_LINE) {
			struct report_lines *rl = rd->data;
			total += RLIB_GET_LINE(get_font_point(r, rl));
			
//Here to adjust size of memo field output.			
		} else if(rd->type == REPORT_PRESENTATION_DATA_HR) {
			struct report_horizontal_line *rhl = rd->data;
			total += RLIB_GET_LINE(rhl->realsize);		
		}
	}
	return total;
}

gfloat get_outputs_size(rlib *r, struct report_element *e, gint page) {
	gfloat total=0;
	struct report_output_array *roa;

	for(; e != NULL; e=e->next) {
		roa = e->data;
		if(roa->page == -1 || roa->page == page || roa->page == -1)
			total += get_output_size(r, roa);
	}			

	return total;
}


gint will_this_fit(rlib *r, gfloat total, gint page) {
	if(OUTPUT(r)->rlib_is_single_page(r))
		return TRUE;
	if(r->reports[r->current_report]->position_top[page-1]+total > r->reports[r->current_report]->position_bottom[page-1])
		return FALSE;
	else
		return TRUE;
}

gint will_outputs_fit(rlib *r, struct report_element *e, gint page) {
	gfloat size = 0;
	struct report_output_array *roa;

	if(OUTPUT(r)->rlib_is_single_page(r))
		return TRUE;
	if(e == NULL)
		return TRUE;
	for(; e != NULL; e=e->next) {
		roa = e->data;
		if(page == -1 || page == roa->page || roa->page == -1)
			size += get_output_size(r, roa);
	}			
	return will_this_fit(r, size, page);
}


gint rlib_end_page_if_line_wont_fit(rlib *r, struct report_element *e) {
	gint i, fits=TRUE;	
	for(i=0;i<r->reports[r->current_report]->pages_accross;i++) {
		if(!will_outputs_fit(r,e, i+1))
			fits=FALSE;
	}
	if(!fits)
		OUTPUT(r)->rlib_end_page(r);
	return !fits;
}

void rlib_print_report_footer(rlib *r) {
	gint i;
	if(r->reports[r->current_report]->report_footer != NULL) {
		for(i=0;i<r->reports[r->current_report]->pages_accross;i++)
			rlib_end_page_if_line_wont_fit(r, r->reports[r->current_report]->report_footer);
		print_report_output(r, r->reports[r->current_report]->report_footer, FALSE);
	}
}

gint rlib_fetch_first_rows(rlib *r) {
	gint i;
	for(i=0;i<r->queries_count;i++)
		INPUT(r,i)->first(INPUT(r,i), r->results[i].result);
	return 0;
}

void rlib_init_variables(rlib *r) {
	struct report_element *e;
	for(e = r->reports[r->current_report]->variables; e != NULL; e=e->next) {
		struct report_variable *rv = e->data;
		if(rv->type == REPORT_VARIABLE_EXPRESSION) {
			RLIB_VARIABLE_CA(rv) = g_malloc(sizeof(struct count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_COUNT) {
			RLIB_VARIABLE_CA(rv) = g_malloc(sizeof(struct count_amount));
			RLIB_VARIABLE_CA(rv)->count = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->count, 0);
		} else if(rv->type == REPORT_VARIABLE_SUM) {
			RLIB_VARIABLE_CA(rv) = g_malloc(sizeof(struct count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_AVERAGE) {
			RLIB_VARIABLE_CA(rv) = g_malloc(sizeof(struct count_amount));
			RLIB_VARIABLE_CA(rv)->count = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->count, 0);
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_LOWEST) {
			RLIB_VARIABLE_CA(rv) = g_malloc(sizeof(struct count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_HIGHEST) {
			RLIB_VARIABLE_CA(rv) = g_malloc(sizeof(struct count_amount));
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
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_EXPRESSION\n");
		} else if(rv->type == REPORT_VARIABLE_SUM) {
			if(RLIB_VALUE_IS_NUMBER(er))
				RLIB_VALUE_GET_AS_NUMBER(amount) += RLIB_VALUE_GET_AS_NUMBER(er);
			else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_SUM\n");
		} else if(rv->type == REPORT_VARIABLE_AVERAGE) {
			RLIB_VALUE_GET_AS_NUMBER(count) += RLIB_DECIMAL_PERCISION;
			if(RLIB_VALUE_IS_NUMBER(er))
				RLIB_VALUE_GET_AS_NUMBER(amount) += RLIB_VALUE_GET_AS_NUMBER(er);
			else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_AVERAGE\n");
		} else if(rv->type == REPORT_VARIABLE_LOWEST) {
			if(RLIB_VALUE_IS_NUMBER(er)) {
				if(RLIB_VALUE_GET_AS_NUMBER(er) < RLIB_VALUE_GET_AS_NUMBER(amount) || RLIB_VALUE_GET_AS_NUMBER(amount) == 0) //TODO: EVIL HACK
					RLIB_VALUE_GET_AS_NUMBER(amount) = RLIB_VALUE_GET_AS_NUMBER(er);
			} else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_LOWEST\n");
		} else if(rv->type == REPORT_VARIABLE_HIGHEST) {
			if(RLIB_VALUE_IS_NUMBER(er)) {
				if(RLIB_VALUE_GET_AS_NUMBER(er) > RLIB_VALUE_GET_AS_NUMBER(amount) || RLIB_VALUE_GET_AS_NUMBER(amount) == 0) //TODO: EVIL HACK
					RLIB_VALUE_GET_AS_NUMBER(amount) = RLIB_VALUE_GET_AS_NUMBER(er);
			} else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_HIGHEST\n");
		}
	}
	
}

gint make_report(rlib *r) {
	gint i = 0;
	gint report = 0;
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

	OUTPUT(r)->rlib_init_output(r);
	rlib_fetch_first_rows(r);

	for(report=0;report<r->reports_count;report++) {
		r->current_report = report;
		if(report > 0) {
			if(r->reports[r->current_report]->mainloop_query != -1)
				r->current_result = r->reports[r->current_report]->mainloop_query;
		}

		rlib_resolve_fields(r);
		if(r->reports[r->current_report]->font_size != -1)
			r->font_point = r->reports[r->current_report]->font_size;
		OUTPUT(r)->rlib_start_report(r);
		rlib_init_variables(r);
		rlib_init_page(r, TRUE);		
		OUTPUT(r)->rlib_begin_text(r);
		if(INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result) != TRUE) {
			while (1) {
				gint page;
				gint output_count = 0;
				rlib_process_variables(r);
				rlib_handle_break_headers(r);
			
				if(rlib_end_page_if_line_wont_fit(r, r->reports[r->current_report]->detail.fields))
					for(page=0;page<r->reports[r->current_report]->pages_accross;page++)
						rlib_force_break_headers(r);

				if(OUTPUT(r)->do_break)
					output_count = print_report_output(r, r->reports[r->current_report]->detail.fields, FALSE);
				else
					output_count = hack_print_report_outputs(r);

				if(output_count > 0)
					r->detail_line_count++;
				i++;

				if(rlib_navigate_next(r, r->current_result) == FALSE) {
					rlib_navigate_last(r, r->current_result);
					rlib_handle_break_footers(r);
					break;
				} 

				rlib_handle_break_footers(r);
			}
		}

		rlib_navigate_last(r, r->current_result);

		rlib_print_report_footer(r);
		OUTPUT(r)->rlib_end_report(r);

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

gint rlib_finalize(rlib *r) {
	OUTPUT(r)->rlib_finalize_private(r);
	return 0;
}
