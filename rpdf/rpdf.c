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
 * $Id$s
 *
 * This module implements the C language API (Application Programming Interface)
 * for the RLIB library functions.
 */

/*
	JPEG Stuff from: The Independent JPEG Group's JPEG software
	TODO: Include README And stuff from jpeg 
*/
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include <glib.h>

#include "rpdf.h"

#define M_SOF0  0xC0		/* Start Of Frame N */
#define M_SOF1  0xC1		/* N indicates which compression process */
#define M_SOF2  0xC2		/* Only SOF0-SOF2 are now in common use */
#define M_SOF3  0xC3
#define M_SOF5  0xC5		/* NB: codes C4 and CC are NOT SOF markers */
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8		/* Start Of Image (beginning of datastream) */
#define M_EOI   0xD9		/* End Of Image (end of datastream) */
#define M_SOS   0xDA		/* Start Of Scan (begins compressed data) */
#define M_APP0	 0xE0		/* Application-specific marker, type N */
#define M_APP12 0xEC		/* (we don't bother to list all 16 APPn's) */
#define M_COM   0xFE		/* COMment */

#define DEGREE_2_RAD(x) (x*M_PI/180.0)
#define CALLBACK void (*callback)(gchar *data, gint len, gchar *user_data)

static gchar * obj_printf(gchar *obj, gchar *fmt, ...) {
	va_list vl;
	gchar *result = NULL;

	va_start(vl, fmt);
	result = g_strdup_vprintf(fmt, vl);
	va_end(vl);
	if (result != NULL) {
		if(obj == NULL) {
			return result;
		} else {
			gchar *new_obj = g_strconcat(obj, result, NULL);
			g_free(obj);
			g_free(result);
			return new_obj;
		}
	}
	return NULL;
}

static void rpdf_object_append(struct rpdf *pdf, gboolean put_wrapper, gchar *contents, gchar *stream, gint stream_length) {
	struct rpdf_object *object = g_new0(struct rpdf_object, 1);
	object->put_wrapper = put_wrapper;
	object->contents = contents;
	object->stream = stream;
	object->stream_length = stream_length;
	pdf->objects = g_slist_append(pdf->objects, object);
}

static struct rpdf_stream * rpdf_stream_new(gint type, gpointer data) {
	struct rpdf_stream *stream = g_new0(struct rpdf_stream, 1);
	stream->type = type;
	stream->data = data;
	return stream;
}

static void rpdf_stream_append(struct rpdf *pdf, struct rpdf_stream *stream) {
	pdf->page_contents[pdf->page_count-1] = g_slist_append(pdf->page_contents[pdf->page_count-1], stream);
}

static gboolean rpdf_out_string(struct rpdf *rpdf, gchar *output) {
	gint size = strlen(output);
	rpdf->out_buffer = g_realloc(rpdf->out_buffer, rpdf->size+size);
	memcpy(rpdf->out_buffer + rpdf->size, output, size);
	rpdf->size += size;
	return TRUE;
}

static gboolean rpdf_out_binary(struct rpdf *rpdf, gchar *output, gint size) {
	rpdf->out_buffer = g_realloc(rpdf->out_buffer, rpdf->size+size);
	memcpy(rpdf->out_buffer + rpdf->size, output, size);
	rpdf->size += size;
	return TRUE;
}

static void rpdf_finalize_objects(gpointer data, gpointer user_data) {
	gchar buf[128];
	struct rpdf_object *object = data;
	struct rpdf *pdf = user_data;
	pdf->object_count++;
	
	pdf->xref = g_slist_append(pdf->xref, (gpointer)pdf->size);
	sprintf(buf, "%d 0 obj\n", pdf->object_count);
	rpdf_out_string(pdf, buf);
	if(object->put_wrapper)
		rpdf_out_string(pdf, "<<\n");
	rpdf_out_string(pdf, object->contents);
	if(object->put_wrapper)
		rpdf_out_string(pdf, ">>\n");
	if(object->stream != NULL) {
		rpdf_out_string(pdf, "stream\n");
		rpdf_out_binary(pdf, object->stream, object->stream_length);
		rpdf_out_string(pdf, "\nendstream\n");
		g_free(object->stream);
	}		
	rpdf_out_string(pdf, "endobj\n");
}

static void rpdf_finalize_xref(gpointer data, gpointer user_data) {
	gint spot = (gint)data;
	struct rpdf *pdf = user_data;
	gchar buf[128];
	sprintf(buf, "%010d 00000 n \n", spot);
	rpdf_out_string(pdf, buf);
}

static void rpdf_stream_font_destroyer(gpointer data, gpointer user_data) {
	g_free(data);
}

static void rpdf_object_destroyer(gpointer data, gpointer user_data) {
	struct rpdf_object *object = data;
	g_free(object->contents);
	g_free(object);
}

static void rpdf_make_page_stream(gpointer data, gpointer user_data) {
	struct rpdf_stream *stream = data;
	gchar *result = NULL, extra[512];
	struct rpdf *pdf = user_data;
	gchar *old_page_data;
	extra[0] = 0;
	
	if(stream->type == RPDF_TYPE_FONT || stream->type == RPDF_TYPE_TEXT) {
		if(pdf->text_on == FALSE) {
			pdf->text_on = TRUE;
			sprintf(extra, "BT\n");
		}
	}
	if(stream->type == RPDF_TYPE_RECT || stream->type == RPDF_TYPE_FILL || stream->type == RPDF_TYPE_IMAGE || stream->type == RPDF_TYPE_LINE
		|| stream->type == RPDF_TYPE_MOVE || stream->type == RPDF_TYPE_CLOSEPATH || stream->type == RPDF_TYPE_STROKE || stream->type == RPDF_TYPE_WIDTH 
		||	stream->type == RPDF_TYPE_ARC) {
		if(pdf->text_on == TRUE) {
			pdf->text_on = FALSE;
			sprintf(extra, "ET\n");
		}
	}

	if(stream->type == RPDF_TYPE_FONT) {
		struct rpdf_stream_font *stream_font = stream->data;
		gchar *font_object;
		result = g_strdup_printf("%s/F%s %.03lf Tf\n", extra, stream_font->font_object->name, stream_font->size);
		font_object = g_hash_table_lookup(pdf->page_fonts, stream_font->font_object->name);
		if(font_object == NULL) {
			g_hash_table_insert(pdf->page_fonts, stream_font->font_object->name, stream_font);
		}
	} else if(stream->type == RPDF_TYPE_TEXT) {
		struct rpdf_stream_text *stream_text = stream->data;
		gdouble angle = M_PI * stream_text->angle / 180.0;
		gdouble text_sin= sin(angle);
		gdouble text_cos = cos(angle);

		result = g_strdup_printf("%s%.04f %.04f %.04f %.04f %.04f %.04f Tm\n(%s) Tj\n", extra, text_cos, text_sin, -text_sin, text_cos, stream_text->x*RPDF_DPI, stream_text->y*RPDF_DPI, stream_text->text); 
		g_free(stream_text->text);
		g_free(stream_text);
	} else if(stream->type == RPDF_TYPE_RECT) {
		struct rpdf_stream_rect *stream_rect = stream->data;
		result = g_strdup_printf("%s%.03f %.03f %.03f %.03f re\n", extra, stream_rect->x*RPDF_DPI, stream_rect->y*RPDF_DPI, stream_rect->width*RPDF_DPI, stream_rect->height*RPDF_DPI);
		g_free(stream_rect);
	} else if(stream->type == RPDF_TYPE_FILL) {
		result = g_strdup_printf("%sf\n", extra);
	} else if(stream->type == RPDF_TYPE_COLOR) {
		struct rpdf_stream_color *stream_color = stream->data;
		result = g_strdup_printf("%.04f %.04f %.04f rg\n%.04f %.04f %.04f RG\n", stream_color->r, stream_color->g, stream_color->b, stream_color->r, stream_color->g, stream_color->b);
		g_free(stream_color);
	} else if(stream->type == RPDF_TYPE_IMAGE) {
		struct rpdf_images *image = stream->data;
		result = g_strdup_printf("%sq\n1.0000 0.0000 0.0000 1.0000 %.04f %.04f cm\n%.04f 0.0000 0.0000 %.04f 0.0000 0.0000 cm\n/IMrpdf%d Do\nQ\n", 
			extra, image->x*RPDF_DPI, image->y*RPDF_DPI, image->width, image->height, image->number);
	} else if(stream->type == RPDF_TYPE_MOVE) {
		struct rpdf_stream_rect *stream_point = stream->data;
		result = g_strdup_printf("%s%.03f %.03f m\n", extra, stream_point->x*RPDF_DPI, stream_point->y*RPDF_DPI);
		g_free(stream_point);
	} else if(stream->type == RPDF_TYPE_WIDTH) {
		gdouble *width = stream->data;
		result = g_strdup_printf("%s%.04f w\n", extra, *width);
		g_free(width);
	} else if(stream->type == RPDF_TYPE_LINE) {
		struct rpdf_stream_rect *stream_point = stream->data;
		result = g_strdup_printf("%s%.03f %.03f l\n", extra, stream_point->x*RPDF_DPI, stream_point->y*RPDF_DPI);
		g_free(stream_point);
	} else if(stream->type == RPDF_TYPE_CLOSEPATH) {
		result = g_strdup_printf("%sh\n", extra);
	} else if(stream->type == RPDF_TYPE_STROKE) {
		result = g_strdup_printf("%sS\n", extra);
	} else if(stream->type == RPDF_TYPE_ARC) {
		char buf[1024];
		struct rpdf_stream_arc *arc = stream->data;
		gdouble save_x, save_y;
		gdouble x = arc->x;
		gdouble y = arc->y;
		gdouble start_angle = arc->start_angle;
		gdouble end_angle = arc->end_angle;
		gdouble radius = arc->radius;
		gint nsegs = 8,i;
		gdouble total_angle,dt,dtm,t1,a0,b0,c0,d0,a1,b1,c1,d1;

		x *= RPDF_DPI;
		y *= RPDF_DPI;
		radius *= RPDF_DPI;
		start_angle = DEGREE_2_RAD(start_angle);
		end_angle = DEGREE_2_RAD(end_angle);
		total_angle = end_angle - start_angle;
		dt = total_angle / (gfloat)nsegs;
		dtm = dt/3.0;
		t1 = start_angle;
		a0 = x + (radius * cos(t1));
		b0 = y + (radius * sin(t1));
		c0 = -radius * sin(t1);
		d0 = radius * cos(t1);
		save_x = a0;
		save_y = b0;
		if(total_angle < DEGREE_2_RAD(360.0)) {  //Pie Slices
			sprintf(buf, "%s%.03f %.03f m\n", extra, x, y);
			sprintf(buf, "%s%.03f %.03f l\n", buf, a0, b0);
		} else {
			sprintf(buf, "%s%.03f %.03f m\n", extra, a0,b0);
		
		}		
		for (i = 1; i <= nsegs; i++) {
			t1 = ((gfloat)i * dt) + start_angle;
			a1 = x + (radius * cos(t1));
			b1 = y + (radius * sin(t1));
			c1 = -radius * sin(t1);
			d1 = radius * cos(t1);
			sprintf(buf, "%s%.02f %.02f %.02f %.02f %.02f %.02f c\n", buf, (a0 + (c0 * dtm)),((b0 + (d0 * dtm))),(a1 - (c1 * dtm)),((b1 - (d1 * dtm))),a1,(b1));
			a0 = a1;
			b0 = b1;
			c0 = c1;
			d0 = d1;
		}
		if(total_angle < DEGREE_2_RAD(360.0)) { //pizza :)
			sprintf(buf, "%s%.03f %.03f l\n", buf, x, y);
		}
		result = g_strdup_printf("%s", buf);
		g_free(arc);
	} else if(stream->type == RPDF_TYPE_TEXT_CB) {
		struct rpdf_stream_text_callback *stream_text_callback = stream->data;
		gchar *callback_data;
		gdouble angle = M_PI * stream_text_callback->angle / 180.0;
		gdouble text_sin= sin(angle);
		gdouble text_cos = cos(angle);
		callback_data = g_malloc(stream_text_callback->len+1);
	   stream_text_callback->callback(callback_data, stream_text_callback->len+1, stream_text_callback->user_data);
		result = g_strdup_printf("%s%.04f %.04f %.04f %.04f %.04f %.04f Tm\n(%s) Tj\n", extra, text_cos, text_sin, -text_sin, text_cos, stream_text_callback->x*RPDF_DPI, stream_text_callback->y*RPDF_DPI, callback_data); 
		g_free(callback_data);
		g_free(stream_text_callback);
	}

	g_free(stream);
	if(pdf->page_data == NULL) {
		pdf->page_data = result;
	} else {
		old_page_data = pdf->page_data;
		pdf->page_data = g_strconcat(pdf->page_data, result, NULL);
		g_free(old_page_data);
		g_free(result);
	}
}

static void rpdf_make_page_images(gpointer data, gpointer user_data) {
	struct rpdf *pdf = user_data;
	struct rpdf_images *image = data;
	pdf->working_obj = obj_printf(pdf->working_obj, "/IMrpdf%d %d 0 R\n", image->number, 5+(pdf->page_count*2)+pdf->font_count+pdf->annot_count);
}

static void rpdf_make_page_image_obj(gpointer data, gpointer user_data) {
	struct rpdf *pdf = user_data;
	struct rpdf_images *image = data;
	struct rpdf_image_jpeg *jpeg = image->metadata;
	gchar *obj = NULL;
	
	obj = obj_printf(NULL, "/Type /XObject\n");
	obj = obj_printf(obj, "/Subtype /Image\n");	
	obj = obj_printf(obj, "/Name /IMrpdf%d\n", image->number);
	obj = obj_printf(obj, "/Width %d\n", jpeg->width);
	obj = obj_printf(obj, "/Height %d\n", jpeg->width);
	obj = obj_printf(obj, "/Filter /DCTDecode\n");
	obj = obj_printf(obj, "/BitsPerComponent %d\n", jpeg->precision);
	obj = obj_printf(obj, "/ColorSpace /");

	if(jpeg->components == 3)
		obj = obj_printf(obj, "DeviceRGB");
	else if(jpeg->components == 4)
		obj = obj_printf(obj, "DeviceCMYK");
	else 
		obj = obj_printf(obj, "DeviceGray");

	obj = obj_printf(obj, "\n");
	
	obj = obj_printf(obj, "/Length %ld\n", image->length);



	rpdf_object_append(pdf, TRUE, obj, image->data, image->length);
	g_free(image->metadata);
	g_free(image);
}

static void rpdf_make_page_annot_obj(gpointer data, gpointer user_data) {
	struct rpdf *pdf = user_data;
	struct rpdf_annots *annot = data;
	gchar *obj = NULL;
	obj = obj_printf(NULL, "/Type /Annot\n");
	obj = obj_printf(obj, "/Subtype /Link\n");
	obj = obj_printf(obj, "/Rect [%d %d %d %d]\n", (gint)(annot->start_x*RPDF_DPI), (gint)(annot->start_y*RPDF_DPI), (gint)(annot->end_x*RPDF_DPI), (gint)(annot->end_y*RPDF_DPI));
	obj = obj_printf(obj, "/A << /S /URI\n");
	obj = obj_printf(obj, "/URI (%s)\n", annot->url);
	obj = obj_printf(obj, ">>\n");
	obj = obj_printf(obj, "/Border [0 0 0]\n");
	obj = obj_printf(obj, "/C [0.0000 0.0000 1.0000]\n");
	obj = obj_printf(obj, "/F 0\n");
	rpdf_object_append(pdf, TRUE, obj, NULL, 0);
	g_free(annot->url);
	g_free(annot);
}

static void rpdf_make_page_annot_list(gpointer data, gpointer user_data) {
	struct rpdf *pdf = user_data;
	struct rpdf_annots *annot = data;
	pdf->working_obj = obj_printf(pdf->working_obj, "%d 0 R ", 5+(pdf->page_count*2)+pdf->font_count+annot->number);
}

static void rpdf_make_page_fonts_stream(gpointer key, gpointer value, gpointer user_data) {
	struct rpdf *pdf = user_data;
	struct rpdf_stream_font *stream_font = value;
	pdf->working_obj = obj_printf(pdf->working_obj, "/F%s %d 0 R\n", stream_font->font_object->name, 5+(pdf->page_count*2)+stream_font->font_object->number);
}

static void rpdf_make_fonts_stream(gpointer key, gpointer value, gpointer user_data) {
	struct rpdf *pdf = user_data;
	struct rpdf_font_object *font = value;
	gchar *obj;
	obj = obj_printf(NULL, "/Type /Font\n");
	obj = obj_printf(obj, "/Subtype /%s\n", font->font->subtype);
	obj = obj_printf(obj, "/Name /F%s\n", font->name);
	obj = obj_printf(obj, "/BaseFont /%s\n", font->font->name);
	obj = obj_printf(obj, "/Encoding /%s\n", font->encoding);
	rpdf_object_append(pdf, TRUE,  obj, NULL, 0);
}

static void rpdf_number_fonts(gpointer key, gpointer value, gpointer user_data) {
	struct rpdf *pdf = user_data;
	struct rpdf_font_object *font = value;
	font->number = pdf->font_obj_number++;
}

static void rpdf_finalize_page_stream(struct rpdf *pdf) {
	if(pdf->text_on == TRUE) {
		gchar *old_page_data = pdf->page_data;
		pdf->page_data = g_strconcat(pdf->page_data, "ET\n", NULL);		
		g_free(old_page_data);	
	}
}

static void rpdf_string_destroyer (gpointer data) {
	g_free(data);
}

struct rpdf *rpdf_new() {
	struct rpdf *pdf = g_new0(struct rpdf, 1);
	pdf->header = g_strdup_printf("%%PDF-1.3\n");
	pdf->fonts = g_hash_table_new_full (g_str_hash, g_str_equal, rpdf_string_destroyer, NULL);	
	return pdf;
}

gboolean rpdf_finalize(struct rpdf *pdf) {
	gint i, save_size;
	gchar *obj = NULL;
	gchar buf[128];
	rpdf_out_string(pdf, pdf->header);
	
	g_hash_table_foreach(pdf->fonts, rpdf_number_fonts, pdf);
	
	obj = obj_printf(NULL, "/Type /Catalog\n");
	obj = obj_printf(obj, "/Pages 3 0 R\n"); 
	obj = obj_printf(obj, "/Outlines 2 0 R\n"); 
	rpdf_object_append(pdf, TRUE, obj, NULL, 0);
	
	obj = obj_printf(NULL, "/Type /Outlines\n");
	obj = obj_printf(obj, "/Count 0\n"); 
	rpdf_object_append(pdf, TRUE, obj, NULL, 0);

	obj = obj_printf(NULL, "/Type /Pages\n");
	obj = obj_printf(obj, "/Count %d\n", pdf->page_count); 
	obj = obj_printf(obj, "/Kids ["); 
	for(i=0;i<pdf->page_count;i++) 
		obj = obj_printf(obj, " %d 0 R ", 3+2*(i+1)); 
	obj = obj_printf(obj, "]\n"); 
	rpdf_object_append(pdf, TRUE, obj, NULL, 0);

	buf[0] = 0;
	if(pdf->has_images)
		sprintf(buf, " /ImageC");
	obj = obj_printf(NULL, "[/PDF /Text%s ]\n",buf);
	rpdf_object_append(pdf, FALSE, obj, NULL, 0);

	for(i=0;i<pdf->page_count;i++) {
		struct rpdf_page_info *page_info = pdf->page_info[i];
		GSList *list = pdf->page_contents[i];
		gint slen = 0;
		pdf->text_on = FALSE;
		pdf->page_data = NULL;
		pdf->page_fonts = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);

		if(list != NULL)
			g_slist_foreach(list, rpdf_make_page_stream, pdf);
		rpdf_finalize_page_stream(pdf);

		obj = obj_printf(NULL, "/Type /Page\n");
		obj = obj_printf(obj, "/Parent 3 0 R\n");
		obj = obj_printf(obj, "/Resources <<\n");
		if(g_hash_table_size(pdf->page_fonts) > 0) {
			obj = obj_printf(obj, "/Font <<\n");
			pdf->working_obj = obj;
			g_hash_table_foreach(pdf->page_fonts, rpdf_make_page_fonts_stream, pdf);
			obj = pdf->working_obj;
			obj = obj_printf(obj, ">>\n");
		}

		if(page_info->images != NULL) {
			obj = obj_printf(obj, "/XObject <<\n");
			pdf->working_obj = obj;
			g_slist_foreach(page_info->images, rpdf_make_page_images, pdf);
			obj = pdf->working_obj;
			obj = obj_printf(obj, ">>\n");
		}
		
		obj = obj_printf(obj, "/ProcSet 4 0 R >>\n");
		obj = obj_printf(obj, "/MediaBox [0 0 %d %d]\n", page_info->paper->x, page_info->paper->y);
		obj = obj_printf(obj, "/CropBox [0 0 %d %d]\n", page_info->paper->x, page_info->paper->y);
		obj = obj_printf(obj, "/Rotate %d\n", page_info->orientation == RPDF_PORTRAIT ? 0 : 270);
		obj = obj_printf(obj, "/Contents %d 0 R\n", 6+(i*2));
		
		if(page_info->annots != NULL) {
			obj = obj_printf(obj, "/Annots [ ");
			pdf->working_obj = obj;
			g_slist_foreach(page_info->annots, rpdf_make_page_annot_list, pdf);
			obj = pdf->working_obj;
			obj = obj_printf(obj, "]\n");
		}
		
		rpdf_object_append(pdf, TRUE, obj, NULL, 0);
		
		if(pdf->page_data != NULL) {
			gchar buf1[512], buf2[512];
			gchar *save_page_data;
			if(page_info->has_rotation == TRUE) {
				gdouble angle = M_PI * page_info->rotation_angle / 180.0;
				gdouble text_sin= sin(angle);
				gdouble text_cos = cos(angle);
				sprintf(buf2, "%.04f %.04f %.04f %.04f 0.0 0.0 cm\n", text_cos, text_sin, -text_sin, text_cos);
				save_page_data = pdf->page_data;
				pdf->page_data = g_strconcat(buf2, pdf->page_data, NULL);
				g_free(save_page_data);
			}
			if(page_info->has_translation == TRUE) {
				sprintf(buf1, "1.0000 0.0000 0.0000 1.0000 %.04f %.04f cm\n", page_info->translate_x*RPDF_DPI, page_info->translate_y*RPDF_DPI);
				save_page_data = pdf->page_data;
				pdf->page_data = g_strconcat(buf1, pdf->page_data, NULL);
				g_free(save_page_data);
			}

			slen = strlen(pdf->page_data) + 1;
		}
		obj = obj_printf(NULL, "<</Length %d>>\n", slen);
		obj = obj_printf(obj, "stream\n");
		obj = obj_printf(obj, "\n");
		if(slen > 0) {
			gchar *save_obj = obj;
			obj = g_strconcat(obj, pdf->page_data, "\n", NULL);
			g_free(save_obj);
		}
		obj = obj_printf(obj, "endstream\n");
		rpdf_object_append(pdf, FALSE,  obj, NULL, 0);
		g_hash_table_destroy(pdf->page_fonts);
		g_free(pdf->page_data);
		g_slist_free(list);
	}

	g_hash_table_foreach(pdf->fonts, rpdf_make_fonts_stream, pdf);

	for(i=0;i<pdf->page_count;i++) {
		struct rpdf_page_info *page_info = pdf->page_info[i];
		if(page_info->annots != NULL) {
			g_slist_foreach(page_info->annots, rpdf_make_page_annot_obj, pdf);
			g_slist_free(page_info->annots);
		}
	}
	
	for(i=0;i<pdf->page_count;i++) {
		struct rpdf_page_info *page_info = pdf->page_info[i];
		if(page_info->images != NULL) {
			g_slist_foreach(page_info->images, rpdf_make_page_image_obj, pdf);
			g_slist_free(page_info->images);
		}
	}
	
	if(pdf->creator != NULL) 
		obj = obj_printf(NULL, "/Creator %s\n", pdf->creator); 
	else
		obj = obj_printf(NULL, "/Creator (RPDF By SICOM Systems)\n"); 

	obj = obj_printf(obj, "/CreationDate (D:20050220143402)\n"); 
	obj = obj_printf(obj, "/Producer (RPDF 1.0)\n");
	
	if(pdf->author != NULL)
		obj = obj_printf(obj, "/Author %s\n", pdf->author); 
	else		
		obj = obj_printf(obj, "/Author (RPDF)\n"); 
	
	if(pdf->title != NULL)
		obj = obj_printf(obj, "/Title %s\n", pdf->title); 
	else 
		obj = obj_printf(obj, "/Title (No Title)\n"); 

	if(pdf->subject != NULL)
		obj = obj_printf(obj, "/Subject %s\n", pdf->subject); 
	else
		obj = obj_printf(obj, "/Subject (None)\n"); 
				
	if(pdf->keywords != NULL)
		obj = obj_printf(obj, "/Keywords %s\n", pdf->keywords); 
	else
		obj = obj_printf(obj, "/Keywords (RPDF)\n"); 
		

	rpdf_object_append(pdf, TRUE, obj, NULL, 0);

	g_slist_foreach(pdf->objects, rpdf_finalize_objects, pdf);
	
	save_size = pdf->size;
	rpdf_out_string(pdf, "xref\n");
	sprintf(buf, "0 %d\n", pdf->object_count+1);
	rpdf_out_string(pdf, buf);
	rpdf_out_string(pdf, "0000000000 65535 f \n");
	g_slist_foreach(pdf->xref, rpdf_finalize_xref, pdf);
	
	rpdf_out_string(pdf, "trailer\n");
	rpdf_out_string(pdf, "<<\n");
	sprintf(buf, "/Size %d\n", 5+(pdf->page_count*2)+1+pdf->font_count+pdf->annot_count+pdf->image_count);
	rpdf_out_string(pdf, buf);
	rpdf_out_string(pdf, "/Root 1 0 R\n");
	sprintf(buf, "/Info %d 0 R\n", 5+(pdf->page_count*2)+pdf->font_count+pdf->annot_count+pdf->image_count);
	rpdf_out_string(pdf, buf);
	rpdf_out_string(pdf, ">>\n");
	sprintf(buf, "startxref\n%d\n", save_size);
	rpdf_out_string(pdf, buf);
	rpdf_out_string(pdf, "%%EOF\n\n");

	return TRUE;
}

void rpdf_set_title(struct rpdf *pdf, gchar *title) {
	pdf->title = g_strdup(title);
}	

void rpdf_set_subject(struct rpdf *pdf, gchar *subject) {
	pdf->subject = g_strdup(subject);
}	

void rpdf_set_author(struct rpdf *pdf, gchar *author) {
	pdf->author = g_strdup(author);
}	

void rpdf_set_keywords(struct rpdf *pdf, gchar *keywords) {
	pdf->keywords = g_strdup(keywords);
}	

void rpdf_set_creator(struct rpdf *pdf, gchar *creator) {
	pdf->creator = g_strdup(creator);
}	

void rpdf_translate(struct rpdf *pdf, gdouble x, gdouble y) {
	struct rpdf_page_info *page_info = pdf->page_info[pdf->current_page];
	page_info->has_translation = TRUE;
	page_info->translate_x = x;
	page_info->translate_y = y;
}

void rpdf_rotate(struct rpdf *pdf, gdouble angle) {
	struct rpdf_page_info *page_info = pdf->page_info[pdf->current_page];
	page_info->has_rotation = TRUE;
	page_info->rotation_angle = angle;
}

gboolean rpdf_new_page(struct rpdf *pdf, gint paper, gint orientation) {
	struct rpdf_page_info *page_info;
	pdf->page_count++;
	pdf->page_contents = g_realloc(pdf->page_contents, sizeof(gpointer) * pdf->page_count);
	pdf->page_contents[pdf->page_count-1] = NULL;

	pdf->page_info = g_realloc(pdf->page_info, sizeof(gpointer) * pdf->page_count);
	page_info = pdf->page_info[pdf->page_count-1] = g_new0(struct rpdf_page_info, 1);
	page_info->paper = &rpdf_paper[paper-1];
	page_info->orientation = orientation;

	pdf->current_page = pdf->page_count - 1;
	return TRUE;
}

gboolean rpdf_set_page(struct rpdf *pdf, gint page) {
	page -= 1;
	if(page < 0 || page > pdf->page_count)
		return FALSE;
	pdf->current_page = page;
	return TRUE;
}

gboolean rpdf_set_font(struct rpdf *pdf, gchar *font, gchar *encoding, gdouble size) {
	gint i=0;
	gint found = FALSE;
	struct rpdf_font_object *font_object;
	struct rpdf_stream_font *stream;
	struct rpdf_page_info *page_info = pdf->page_info[pdf->current_page];
	struct rpdf_stream *real_stream;
	gchar *both;
	
	while(rpdf_fonts[i].name[0] != 0) {
		if(strcmp(rpdf_fonts[i].name, font) == 0) {
			found = TRUE;
			break;
		}
		i++;
	}

	if(found == FALSE) 
		return FALSE;		
	
	both = g_strconcat(font, encoding, NULL);
	font_object = g_hash_table_lookup(pdf->fonts, both);
		
	if(font_object == NULL) {
		font_object = g_new0(struct rpdf_font_object, 1);
		font_object->font = &rpdf_fonts[i];
		sprintf(font_object->name, "rpdf%d", pdf->font_count++);
		strcpy(font_object->encoding, encoding);
		g_hash_table_insert(pdf->fonts, both, font_object);			
	} else {
		g_free(both);
	}

	page_info->font_size = size;
	page_info->font = font_object->font;
	page_info->font_object = font_object;

	stream = g_new0(struct rpdf_stream_font, 1);
	stream->font_object = font_object;
	stream->size = size;
	real_stream = rpdf_stream_new(RPDF_TYPE_FONT, stream);
	rpdf_stream_append(pdf, real_stream);
	pdf->stream_font_destroyer = g_slist_append(pdf->stream_font_destroyer, stream);
	
	return TRUE;
}

gboolean rpdf_set_font_size(struct rpdf *pdf, gdouble size) {
	struct rpdf_page_info *page_info = pdf->page_info[pdf->current_page];
	struct rpdf_stream_font *stream;
	struct rpdf_stream *real_stream;
	page_info->font_size = size;

	if(page_info->font == NULL)
		return FALSE;

	stream = g_new0(struct rpdf_stream_font, 1);
	stream->font_object = page_info->font_object;
	stream->size = size;
	real_stream = rpdf_stream_new(RPDF_TYPE_FONT, stream);
	rpdf_stream_append(pdf, real_stream);
	pdf->stream_font_destroyer = g_slist_append(pdf->stream_font_destroyer, stream);
	return TRUE;
}

gboolean rpdf_text_callback(struct rpdf *pdf, gdouble x, gdouble y, gdouble angle, gint len, CALLBACK, gpointer user_data) {
	struct rpdf_stream_text_callback *stream;
	stream = g_new0(struct rpdf_stream_text_callback, 1);
	stream->x = x;
	stream->y = y;
	stream->angle = angle;
	stream->len = len;
	stream->callback = callback;
	stream->user_data = user_data;
	rpdf_stream_append(pdf, rpdf_stream_new(RPDF_TYPE_TEXT_CB, stream));
	return TRUE;
}

gboolean rpdf_text(struct rpdf *pdf, gdouble x, gdouble y, gdouble angle, gchar *text) {
	struct rpdf_stream_text *stream;
	gint slen;
	gint count = 0, spot=0, i;
	stream = g_new0(struct rpdf_stream_text, 1);
	stream->x = x;
	stream->y = y;
	stream->angle = angle;
	
	slen = strlen(text);
	for(i=0;i<slen;i++) {
		if(text[i] == '(' || text[i] == ')')
			count++;
	}
	if(count == 0)
		stream->text = g_strdup(text); 
	else {
		stream->text = g_malloc(slen + 1 + count);
		for(i=0;i<slen;i++) {
			if(text[i] == '(' || text[i] == ')') {
				stream->text[spot++] = '\\';
			}
			stream->text[spot++] = text[i];
		}
		stream->text[spot++] = 0;
	
	}
	rpdf_stream_append(pdf, rpdf_stream_new(RPDF_TYPE_TEXT, stream));
	return TRUE;
}

static guint stream_read_byte(gchar *stream, gint *spot, gint size) {
	guchar data = stream[*spot];
	if(*spot >= size)
		return -1;
	*spot = *spot + 1;
	return data;
}

static guint16 stream_read_two_bytes(gchar *stream, gint *spot, gint size) {
	guint16 data;
	guchar data1 = stream[*spot], data2;
	*spot = *spot + 1;
	data2 = stream[*spot];
	*spot = *spot + 1;

	data = ((guint16)data1 << 8) + data2;

	if(*spot >= size)
		return -1;

	return data;
}

static gint jpeg_get_marker(gchar *stream, gint *spot, gint size) {
	for(;stream_read_byte(stream, spot, size) != 0xFF;);
	return stream_read_byte(stream, spot, size);
}

static void jpeg_skip_section(gchar *stream, gint *spot, gint size) {
 	guint length;

	length = stream_read_two_bytes(stream, spot, size);

	if(length < 2)
		return;

	length -= 2;
	
	*spot = *spot + length;
}

static void jpeg_process_SOFn(gchar *stream, gint *spot, gint size, gint marker, struct rpdf_image_jpeg *info) {
	gint length;
	
	length = stream_read_two_bytes(stream, spot, size);
	info->process = marker;
	info->precision = stream_read_byte(stream, spot, size);
	info->height = stream_read_two_bytes(stream, spot, size);
	info->width = stream_read_two_bytes(stream, spot, size);
	info->components = stream_read_byte(stream, spot, size);
	if(length != (guint)(8 + info->components * 3)) // bad image...
		return;
	*spot = *spot + (3 * info->components);
}

gboolean rpdf_image(struct rpdf *pdf, gdouble x, gdouble y, gdouble width, gdouble height, gint image_type, gchar *file_name) {
	struct rpdf_page_info *page_info = pdf->page_info[pdf->current_page];
	gint fd;
	gint size;
	gint read_spot = 0;
	gboolean loop = TRUE;
	struct rpdf_images *image;
	if(file_name == NULL)
		return FALSE;
		
	fd = open(file_name, O_RDONLY, 0);
	
	if(fd <= 0)
		return FALSE;
	
	size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	
	image = g_new0(struct rpdf_images, 1);
	image->x = x;
	image->y = y;
	image->width = width;
	image->height = height;
	image->data = g_malloc(size);
	image->image_type = image_type;
	image->length = size;
	
	read(fd, image->data, size);
	close(fd);
	 
	
	if(image_type == RPDF_IMAGE_JPEG) {
		struct rpdf_image_jpeg *jpeg_info = g_new0(struct rpdf_image_jpeg, 1);
		image->metadata = jpeg_info;
		guint test1, test2;
		test1 = stream_read_byte(image->data, &read_spot, size);
		test2 = stream_read_byte(image->data, &read_spot, size);
		if(test1 != 0xFF && test2 != M_SOI) {
			g_free(image->data);
			g_free(image);
			return FALSE;
		}
		while( loop) {
			gint marker = jpeg_get_marker(image->data, &read_spot, size);
			switch (marker) {
				case M_SOF0:
				case M_SOF1:
				case M_SOF2:
				case M_SOF3:
				case M_SOF5:
				case M_SOF6:
				case M_SOF7:
				case M_SOF9:
				case M_SOF10:
				case M_SOF11:
				case M_SOF13:
				case M_SOF14:
				case M_SOF15:
			      jpeg_process_SOFn(image->data, &read_spot, size, marker, jpeg_info);
					if(image->width == 0 && image->height == 0) {
						image->width = jpeg_info->width;
						image->height = jpeg_info->height;
					}
			      break;
			    case M_SOS:	
					loop = FALSE;
			    default:
			      jpeg_skip_section(image->data, &read_spot, size);
			      break;
			}
		}	
	}
	image->number = pdf->image_count++;
	page_info->images = g_slist_append(page_info->images, image);
	rpdf_stream_append(pdf, rpdf_stream_new(RPDF_TYPE_IMAGE, image));

	pdf->has_images = TRUE;
	return TRUE;
}

gboolean rpdf_link(struct rpdf *pdf, gdouble start_x, gdouble start_y, gdouble end_x, gdouble end_y, gchar *url) {
	struct rpdf_page_info *page_info = pdf->page_info[pdf->current_page];
	struct rpdf_annots *annot = g_new0(struct rpdf_annots, 1);
	annot->number = pdf->annot_count++;
	annot->start_x = start_x;
	annot->start_y = start_y;
	annot->end_x = end_x;
	annot->end_y = end_y;
	annot->url = g_strdup(url);
	page_info->annots = g_slist_append(page_info->annots, annot);
	return TRUE;
}

gboolean rpdf_moveto(struct rpdf *pdf, gdouble x, gdouble y) {
	struct rpdf_stream_point *stream;
	stream = g_new0(struct rpdf_stream_point, 1);
	stream->x = x;
	stream->y = y;
	rpdf_stream_append(pdf, rpdf_stream_new(RPDF_TYPE_MOVE, stream));
	return TRUE;
}

gboolean rpdf_set_line_width(struct rpdf *pdf, gdouble width) {
	gdouble *new_width = g_malloc(sizeof(gdouble));
	*new_width = width;
	rpdf_stream_append(pdf, rpdf_stream_new(RPDF_TYPE_WIDTH, new_width));
	return TRUE;
}

gboolean rpdf_lineto(struct rpdf *pdf, gdouble x, gdouble y) {
	struct rpdf_stream_point *stream;
	stream = g_new0(struct rpdf_stream_point, 1);
	stream->x = x;
	stream->y = y;
	rpdf_stream_append(pdf, rpdf_stream_new(RPDF_TYPE_LINE, stream));
	return TRUE;
}

gboolean rpdf_closepath(struct rpdf *pdf) {
	rpdf_stream_append(pdf, rpdf_stream_new(RPDF_TYPE_CLOSEPATH, NULL));
	return TRUE;
}

gboolean rpdf_stroke(struct rpdf *pdf) {
	rpdf_stream_append(pdf, rpdf_stream_new(RPDF_TYPE_STROKE, NULL));
	return TRUE;
}

gboolean rpdf_rect(struct rpdf *pdf, gdouble x, gdouble y, gdouble width, gdouble height) {
	struct rpdf_stream_rect *stream;
	stream = g_new0(struct rpdf_stream_rect, 1);
	stream->x = x;
	stream->y = y;
	stream->width = width;
	stream->height = height;
	rpdf_stream_append(pdf, rpdf_stream_new(RPDF_TYPE_RECT, stream));
	return TRUE;
}

gboolean rpdf_fill(struct rpdf *pdf) {
	rpdf_stream_append(pdf, rpdf_stream_new(RPDF_TYPE_FILL, NULL));
	return TRUE;
}

gboolean rpdf_setrgbcolor(struct rpdf *pdf, gdouble r, gdouble g, gdouble b) {
	struct rpdf_stream_color *stream;
	stream = g_new0(struct rpdf_stream_color, 1);
	stream->r = r;
	stream->g = g;
	stream->b = b;
	rpdf_stream_append(pdf, rpdf_stream_new(RPDF_TYPE_COLOR, stream));
	return TRUE;
}

gdouble rpdf_text_width(struct rpdf *pdf, gchar *text) {
	gint slen,i;
	gdouble width = 0.0;
	struct rpdf_page_info *page_info = pdf->page_info[pdf->current_page];
	
	if(text == NULL)
		return 0.0;
	
	slen = strlen(text);
	for(i=0;i<slen;i++) {
		width += page_info->font->widths[(gint)text[i]];
	
	}
	return width*page_info->font_size/1000.0;
}

gint rpdf_arc(struct rpdf *pdf, gdouble x, gdouble y, gdouble radius, gdouble start_angle, gdouble end_angle) {
	struct rpdf_stream_arc *stream = g_new0(struct rpdf_stream_arc, 1);
	stream->x = x;
	stream->y = y;
	stream->radius = radius;
	stream->start_angle = start_angle;
	stream->end_angle = end_angle;
	rpdf_stream_append(pdf, rpdf_stream_new(RPDF_TYPE_ARC, stream));
	return TRUE;
}

gchar *rpdf_get_buffer(struct rpdf *pdf, gint *length) {
	*length = pdf->size;
	return pdf->out_buffer;
}

void rpdf_free(struct rpdf *pdf) {
	gint i;
	g_free(pdf->header);
	g_free(pdf->title);
	g_free(pdf->subject);
	g_free(pdf->author);
	g_free(pdf->keywords);
	g_free(pdf->creator);
	g_hash_table_destroy(pdf->fonts);
	g_free(pdf->page_contents);
	g_free(pdf->out_buffer);
	for(i=0;i<pdf->page_count;i++)
		g_free(pdf->page_info[i]);
	g_free(pdf->page_info);
	g_slist_foreach(pdf->stream_font_destroyer, rpdf_stream_font_destroyer, NULL);
	g_slist_free(pdf->stream_font_destroyer);	
	g_slist_foreach(pdf->objects, rpdf_object_destroyer, NULL);
	g_slist_free(pdf->objects);	
	g_free(pdf);
}
