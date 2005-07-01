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
 *
 * $Id$
 */

#include <stdlib.h>
#include <gmodule.h>

#include "config.h"
#include "rlib.h"
#include "pcode.h"
#include "rlib_input.h"

static void rlib_image_free_pcode(rlib *r, struct rlib_report_image * ri) {
	rlib_pcode_free(ri->value_code);
	rlib_pcode_free(ri->type_code);
	rlib_pcode_free(ri->width_code);
	rlib_pcode_free(ri->height_code);
	xmlFree(ri->xml_value);
	xmlFree(ri->xml_type);
	xmlFree(ri->xml_width);
	xmlFree(ri->xml_height);
	g_free(ri);
}

static void rlib_hr_free_pcode(rlib *r, struct rlib_report_horizontal_line * rhl) {
	rlib_pcode_free(rhl->bgcolor_code);
	rlib_pcode_free(rhl->suppress_code);
	rlib_pcode_free(rhl->indent_code);
	rlib_pcode_free(rhl->length_code);
	rlib_pcode_free(rhl->font_size_code);
	rlib_pcode_free(rhl->size_code);
	xmlFree(rhl->xml_bgcolor);
	xmlFree(rhl->xml_size);
	xmlFree(rhl->xml_indent);
	xmlFree(rhl->xml_length);
	xmlFree(rhl->xml_font_size);
	xmlFree(rhl->xml_suppress);
	g_free(rhl);
}

static void rlib_text_free_pcode(rlib *r, struct rlib_report_literal *rt) {
	rlib_pcode_free(rt->color_code);
	rlib_pcode_free(rt->bgcolor_code);
	rlib_pcode_free(rt->col_code);
	rlib_pcode_free(rt->width_code);
	rlib_pcode_free(rt->bold_code);
	rlib_pcode_free(rt->italics_code);
	rlib_pcode_free(rt->align_code);
	rlib_pcode_free(rt->link_code);
	xmlFree(rt->xml_align);
	xmlFree(rt->xml_bgcolor);
	xmlFree(rt->xml_color);
	xmlFree(rt->xml_width);
	xmlFree(rt->xml_bold);
	xmlFree(rt->xml_italics);
	xmlFree(rt->xml_link);
	xmlFree(rt->xml_col);
	g_free(rt);
}

static void rlib_field_free_pcode(rlib *r, struct rlib_report_field *rf) {
	rlib_pcode_free(rf->code);
	rlib_pcode_free(rf->format_code);
	rlib_pcode_free(rf->link_code);
	rlib_pcode_free(rf->color_code);
	rlib_pcode_free(rf->bgcolor_code);
	rlib_pcode_free(rf->col_code);
	rlib_pcode_free(rf->delayed_code);
	rlib_pcode_free(rf->width_code);
	rlib_pcode_free(rf->bold_code);
	rlib_pcode_free(rf->italics_code);
	rlib_pcode_free(rf->align_code);
	rlib_pcode_free(rf->memo_code);
	rlib_pcode_free(rf->memo_height_code);
	rlib_pcode_free(rf->memo_wrap_chars_code);
	xmlFree(rf->xml_align);
	xmlFree(rf->xml_bgcolor);
	xmlFree(rf->xml_color);
	xmlFree(rf->xml_width);
	xmlFree(rf->xml_bold);
	xmlFree(rf->xml_italics);
	xmlFree(rf->xml_format);
	xmlFree(rf->xml_link);
	xmlFree(rf->xml_col);
	xmlFree(rf->xml_delayed);
	xmlFree(rf->xml_memo);
	xmlFree(rf->xml_memo_height);
	xmlFree(rf->xml_memo_wrap_chars);

	g_free(rf);
}

static void rlib_free_fields(rlib *r, struct rlib_report_output_array *roa) {
	struct rlib_element *e, *save;
	gint j;
	
	if(roa == NULL)
		return;
	for(j=0;j<roa->count;j++) {
		struct rlib_report_output *ro = roa->data[j];
		if(ro->type == RLIB_REPORT_PRESENTATION_DATA_LINE) {
			struct rlib_report_lines *rl = ro->data;	
			e = rl->e;
			rlib_pcode_free(rl->bgcolor_code);
			rlib_pcode_free(rl->color_code);
			rlib_pcode_free(rl->suppress_code);
			rlib_pcode_free(rl->font_size_code);
			rlib_pcode_free(rl->bold_code);
			rlib_pcode_free(rl->italics_code);
			for(; e != NULL; e=e->next) {
				if(e->type == RLIB_ELEMENT_FIELD) {
					rlib_field_free_pcode(r, ((struct rlib_report_field *)e->data));
				} else if(e->type == RLIB_ELEMENT_LITERAL) {
				        rlib_text_free_pcode(r, ((struct rlib_report_literal *)e->data));
				}
			}
			for(e=rl->e; e != NULL; ) {
				save = e;
				e=e->next;
				g_free(save);
			}
			xmlFree(rl->xml_bgcolor);
			xmlFree(rl->xml_color);
			xmlFree(rl->xml_bold);
			xmlFree(rl->xml_italics);
			xmlFree(rl->xml_font_size);
			xmlFree(rl->xml_suppress);
			g_free(rl);
		} else if(ro->type == RLIB_REPORT_PRESENTATION_DATA_HR) {
			rlib_hr_free_pcode(r, ((struct rlib_report_horizontal_line *)ro->data));
		} else if(ro->type == RLIB_REPORT_PRESENTATION_DATA_IMAGE) {
			rlib_image_free_pcode(r, ((struct rlib_report_image *)ro->data));
		}
		g_free(ro);
	}
	g_free(roa->data);
	xmlFree(roa->xml_page);
	g_free(roa);
}


static void rlib_break_free_pcode(rlib *r, struct rlib_break_fields *bf) {
	rlib_pcode_free(bf->code);
	xmlFree(bf->xml_value);
}

static void rlib_free_output(rlib *r, struct rlib_element *e) {
	struct rlib_report_output_array *roa;
	struct rlib_element *save;
	while(e != NULL) {
		save = e;
		roa = e->data;
		rlib_free_fields(r, roa);
		e=e->next;
		g_free(save);
	}	
}

static void rlib_free_graph(rlib *r, struct rlib_graph *graph) {
	struct rlib_graph_plot *plot;
	GSList *list;
	
	rlib_pcode_free(graph->name_code);
	rlib_pcode_free(graph->type_code);
	rlib_pcode_free(graph->subtype_code);
	rlib_pcode_free(graph->width_code);
	rlib_pcode_free(graph->height_code);
	rlib_pcode_free(graph->bold_titles_code);
	rlib_pcode_free(graph->title_code);
	rlib_pcode_free(graph->legend_bg_color_code);
	rlib_pcode_free(graph->legend_orientation_code);
	rlib_pcode_free(graph->draw_x_line_code);
	rlib_pcode_free(graph->draw_y_line_code);
	rlib_pcode_free(graph->grid_color_code);
	rlib_pcode_free(graph->x_axis_title_code);
	rlib_pcode_free(graph->y_axis_title_code);
	rlib_pcode_free(graph->y_axis_mod_code);
	rlib_pcode_free(graph->y_axis_title_right_code);
	rlib_pcode_free(graph->y_axis_decimals_code);
	rlib_pcode_free(graph->y_axis_decimals_code_right);

	xmlFree(graph->xml_name);
	xmlFree(graph->xml_type);
	xmlFree(graph->xml_subtype);
	xmlFree(graph->xml_width);
	xmlFree(graph->xml_height);
	xmlFree(graph->xml_bold_titles);
	xmlFree(graph->xml_title);
	xmlFree(graph->xml_legend_bg_color);
	xmlFree(graph->xml_legend_orientation);
	xmlFree(graph->xml_draw_x_line);
	xmlFree(graph->xml_draw_y_line);
	xmlFree(graph->xml_grid_color);
	xmlFree(graph->xml_x_axis_title);
	xmlFree(graph->xml_y_axis_title);
	xmlFree(graph->xml_y_axis_mod);
	xmlFree(graph->xml_y_axis_title_right);
	xmlFree(graph->xml_y_axis_decimals);
	xmlFree(graph->xml_y_axis_decimals_right);

	for(list=graph->plots;list != NULL; list = g_slist_next(list)) {
		plot = list->data;
		rlib_pcode_free(plot->axis_code);
		rlib_pcode_free(plot->field_code);
		rlib_pcode_free(plot->label_code);
		rlib_pcode_free(plot->side_code);
		rlib_pcode_free(plot->disabled_code);	
		rlib_pcode_free(plot->color_code);	
		xmlFree(plot->xml_axis);
		xmlFree(plot->xml_field);
		xmlFree(plot->xml_label);
		xmlFree(plot->xml_side);
		xmlFree(plot->xml_disabled);
		xmlFree(plot->xml_color);
	}
}

void rlib_free_report(rlib *r, struct rlib_report *report) {
	struct rlib_element *e, *prev;

	if(report->doc != NULL) {
//		xmlFreeDoc(report->doc);
	} else {
		g_free(report->contents);
	}
	
	rlib_pcode_free(report->font_size_code);
	rlib_pcode_free(report->orientation_code);
	rlib_pcode_free(report->top_margin_code);
	rlib_pcode_free(report->left_margin_code);
	rlib_pcode_free(report->bottom_margin_code);
	rlib_pcode_free(report->pages_across_code);
	rlib_pcode_free(report->suppress_page_header_first_page_code);
	rlib_pcode_free(report->detail_columns_code);
	rlib_pcode_free(report->column_pad_code);
	
	rlib_free_output(r, report->report_header);
	rlib_free_output(r, report->page_header);
	rlib_free_output(r, report->page_footer);
	rlib_free_output(r, report->report_footer);
	rlib_free_output(r, report->detail.fields);
	rlib_free_output(r, report->detail.headers);
	rlib_free_output(r, report->alternate.nodata);

	rlib_free_graph(r, &report->graph);

	g_free(report->position_top);
	g_free(report->position_bottom);
	g_free(report->bottom_size);
	
	if(report->breaks != NULL) {
		for(e = report->breaks; e != NULL; e=e->next) {
			struct rlib_report_break *rb = e->data;
			struct rlib_element *be;
			rlib_free_output(r, rb->header);
			rlib_free_output(r, rb->footer);
			for(be = rb->fields; be != NULL; be=be->next) {
				struct rlib_break_fields *bf = be->data;
				rlib_break_free_pcode(r, bf);
				g_free(bf);
			}
			
			while(rb->fields) {
				prev = NULL;
				for(be = rb->fields; be->next != NULL; be=be->next) {
					prev = be;
				}
				g_free(be);
				if(prev != NULL)
					prev->next = NULL;
				else
					break;
			}
			rlib_pcode_free(rb->newpage_code);
			rlib_pcode_free(rb->headernewpage_code);
			rlib_pcode_free(rb->suppressblank_code);
			xmlFree(rb->xml_name);
			xmlFree(rb->xml_newpage);
			xmlFree(rb->xml_headernewpage);
			xmlFree(rb->xml_suppressblank);
			g_free(rb);
		}

		while(report->breaks) {
			prev = NULL;
			for(e = report->breaks; e->next != NULL; e=e->next) {
				prev = e;
			}
			g_free(e);
			if(prev != NULL)
				prev->next = NULL;
			else
				break;
		}

	}	

	if(report->variables != NULL) {
		for(e = report->variables; e != NULL; e=e->next) {
			struct rlib_report_variable *rv = e->data;

			rlib_pcode_free(rv->code);

			if(rv->data != NULL) {
				if(rv->type == RLIB_REPORT_VARIABLE_EXPRESSION) {
					rlib_value_free(&RLIB_VARIABLE_CA(rv)->amount);
					g_free(RLIB_VARIABLE_CA(rv));
				} else if(rv->type == RLIB_REPORT_VARIABLE_COUNT)
					g_free(RLIB_VARIABLE_CA(rv));
				else if(rv->type == RLIB_REPORT_VARIABLE_SUM)
					g_free(RLIB_VARIABLE_CA(rv));
				else if(rv->type == RLIB_REPORT_VARIABLE_AVERAGE)
					g_free(RLIB_VARIABLE_CA(rv));
				else if(rv->type == RLIB_REPORT_VARIABLE_LOWEST)
					g_free(RLIB_VARIABLE_CA(rv));
				else if(rv->type == RLIB_REPORT_VARIABLE_HIGHEST)
					g_free(RLIB_VARIABLE_CA(rv));
			}
			xmlFree(rv->xml_name);
			xmlFree(rv->xml_str_type);
			xmlFree(rv->xml_value);
			xmlFree(rv->xml_resetonbreak);
			g_free(rv);
		}
		
/*		while(report->variables) {
			prev = NULL;
			for(e = report->variables; e->next != NULL; e=e->next) {
				prev = e;
			}
			g_free(e);
			if(prev != NULL)
				prev->next = NULL;
			else
				break;
		}*/
	}
	xmlFree(report->xml_font_size);
	xmlFree(report->xml_query);
	xmlFree(report->xml_orientation);
	xmlFree(report->xml_top_margin);
	xmlFree(report->xml_left_margin);
	xmlFree(report->xml_detail_columns);
	xmlFree(report->xml_column_pad);
	xmlFree(report->xml_bottom_margin);
	xmlFree(report->xml_height);
	xmlFree(report->xml_iterations);
	xmlFree(report->xml_pages_across);
	xmlFree(report->xml_suppress_page_header_first_page);
}

void rlib_free_part_td(rlib *r, struct rlib_part *part, GSList *part_deviations) {
	GSList *element;
	GSList *reports;
	for(element = part_deviations;element != NULL;element = g_slist_next(element)) {
		struct rlib_part_td *td = element->data;
		rlib_pcode_free(td->width_code);
		rlib_pcode_free(td->height_code);
		rlib_pcode_free(td->border_width_code);
		rlib_pcode_free(td->border_color_code);
		xmlFree(td->xml_width);
		xmlFree(td->xml_height);
		xmlFree(td->xml_border_width);
		xmlFree(td->xml_border_color);

		for(reports=td->reports;reports != NULL;reports = g_slist_next(reports)) {
			struct rlib_report *report = reports->data;
			rlib_free_report(r, report);
		}
		g_slist_free(reports);
		g_free(td);
	}
}

static void rlib_free_part_tr(rlib *r, struct rlib_part *part) {
	GSList *element;
	
	for(element = part->part_rows;element != NULL;element = g_slist_next(element)) {
		struct rlib_part_tr *tr = element->data;
		rlib_pcode_free(tr->layout_code);
		rlib_pcode_free(tr->newpage_code);
		xmlFree(tr->xml_layout);
		xmlFree(tr->xml_newpage);
		rlib_free_part_td(r, part, tr->part_deviations);
		g_slist_free(tr->part_deviations);
		g_free(tr);
	}	
	g_slist_free(part->part_rows);
}

void rlib_free_part(rlib *r, struct rlib_part *part) {
	rlib_pcode_free(part->orientation_code);
	rlib_pcode_free(part->font_size_code);
	rlib_pcode_free(part->top_margin_code);
	rlib_pcode_free(part->left_margin_code);
	rlib_pcode_free(part->bottom_margin_code);
	rlib_pcode_free(part->paper_type_code);
	rlib_pcode_free(part->pages_across_code);

	xmlFree(part->xml_name);
	xmlFree(part->xml_pages_across);
	xmlFree(part->xml_font_size);
	xmlFree(part->xml_orientation);
	xmlFree(part->xml_top_margin);
	xmlFree(part->xml_left_margin);
	xmlFree(part->xml_bottom_margin);
	xmlFree(part->xml_paper_type);
	xmlFree(part->xml_iterations);
	
	g_free(part->position_top);
	g_free(part->position_bottom);
	g_free(part->bottom_size);

	rlib_free_output(r, part->page_header);
	rlib_free_output(r, part->page_footer);
	rlib_free_output(r, part->report_header);

	rlib_free_part_tr(r, part);
}

void rlib_free_tree(rlib *r) {
	int i;
	if(r->queries_count > 0) {
		for(i=0;i<r->parts_count;i++) {
			struct rlib_part *part = r->parts[i];
			rlib_free_part(r, part);
			g_free(r->reportstorun[i].name);
			g_free(part);
			r->parts[i] = NULL;
		}
	}
}

void rlib_free_results(rlib *r) {
	int i;
	for(i=0;i<r->queries_count;i++) {
		INPUT(r, i)->free_result(INPUT(r, i), r->results[i].result);
	}
}

void rlib_free_results_and_queries(rlib *r) {
	int i;
	for(i=0;i<r->queries_count;i++) {
		INPUT(r, i)->free_result(INPUT(r, i), r->results[i].result);
		g_free(r->queries[i].sql);
		g_free(r->queries[i].name);
	}
}


gint rlib_free_follower(rlib *r ) {
	gint i;
	for(i=0; i<r->resultset_followers_count; i++) {
		rlib_pcode_free(r->followers[i].leader_code);
		rlib_pcode_free(r->followers[i].follower_code);
	}

	return TRUE;
}

gint rlib_free(rlib *r) {
	int i;
	rlib_charencoder_free(r->output_encoder);
	g_free(r->output_encoder_name);

	rlib_free_tree(r);
	xmlCleanupParser();
	rlib_free_results_and_queries(r);
	for(i=0;i<r->inputs_count;i++) {
		r->inputs[i].input->input_close(r->inputs[i].input);
		r->inputs[i].input->free(r->inputs[i].input);	
		if(r->inputs[i].handle != NULL)
			g_module_close(r->inputs[i].handle);
		g_free(r->inputs[i].name);
	}

	if(r->did_execute) {
		OUTPUT(r)->free(r);
		ENVIRONMENT(r)->free(r);
	}
	g_hash_table_destroy(r->output_parameters);
	g_hash_table_destroy(r->input_metadata);
	g_hash_table_destroy(r->parameters);
	rlib_free_follower(r);
				
	g_free(r);
	return 0;
}
