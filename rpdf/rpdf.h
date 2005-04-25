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
 */

#include <glib.h>

#define RPDF_DPI 72.0
#define RPDF_MAX_CHARS 256

#define RPDF_IMAGE_JPEG 1
#define RPDF_IMAGE_PNG  2
#define RPDF_IMAGE_GIF  3

#define RPDF_TYPE_FONT       1
#define RPDF_TYPE_TEXT       2
#define RPDF_TYPE_LINE       3
#define RPDF_TYPE_RECT       4
#define RPDF_TYPE_FILL       5
#define RPDF_TYPE_COLOR      6
#define RPDF_TYPE_IMAGE      7
#define RPDF_TYPE_MOVE       8
#define RPDF_TYPE_CLOSEPATH  9
#define RPDF_TYPE_STROKE    10
#define RPDF_TYPE_WIDTH     11
#define RPDF_TYPE_ARC       12
#define RPDF_TYPE_TEXT_CB   13

#define RPDF_PORTRAIT  0
#define RPDF_LANDSCAPE 1

#define RPDF_PAPER_LETTER    1
#define RPDF_PAPER_LEGAL     2
#define RPDF_PAPER_A4        3
#define RPDF_PAPER_B5        4
#define RPDF_PAPER_C5        5
#define RPDF_PAPER_DL        6
#define RPDF_PAPER_EXECUTIVE 7
#define RPDF_PAPER_COMM10    8
#define RPDF_PAPER_MONARCH   9
#define RPDF_PAPER_FILM35MM  10

#define DEGREE_2_RAD(x) (x*M_PI/180.0)
#define CALLBACK void (*callback)(gchar *data, gint len, gpointer user_data)


struct _rpdf_paper {
	gint type;
	gint x;
	gint y;
	gchar name[64];
};
extern struct _rpdf_paper rpdf_paper [];

extern gint rpdf_courier_width[RPDF_MAX_CHARS];
extern gint rpdf_helvetica_width[RPDF_MAX_CHARS];
extern gint rpdf_helvetica_bold_width[RPDF_MAX_CHARS];
extern gint rpdf_helvetica_italics_width[RPDF_MAX_CHARS];
extern gint rpdf_helvetica_bold_italics_width[RPDF_MAX_CHARS];
extern gint rpdf_times_width[RPDF_MAX_CHARS];
extern gint rpdf_times_bold_width[RPDF_MAX_CHARS];
extern gint rpdf_times_italics_width[RPDF_MAX_CHARS];
extern gint rpdf_times_bold_italics_width[RPDF_MAX_CHARS];
extern gint rpdf_symbol_width[RPDF_MAX_CHARS];
extern gint rpdf_dingbats_width[RPDF_MAX_CHARS];

struct _rpdf_fonts {
	gchar name[64];
	gchar subtype[64];
	gint *widths;
};
extern struct _rpdf_fonts rpdf_fonts[];

struct rpdf_page_info {
	struct _rpdf_paper *paper;
	gint orientation;
	
	gboolean has_translation;
	gdouble translate_x;
	gdouble translate_y;
	
	gboolean has_rotation;
	gdouble rotation_angle;
	
	struct _rpdf_fonts *font;
	struct rpdf_font_object *font_object;
	gdouble font_size;
	GSList *annots;
	GSList *images;
};

struct rpdf_annots {
	gint number;
	gdouble start_x;
	gdouble start_y;
	gdouble end_x;
	gdouble end_y;
	gchar *url;
};

struct rpdf_image_jpeg {
	gint process;
	gint width;
	gint height;
	gint components;
	gint precision;
};

struct rpdf_image_png {
	gint width;
	gint height;
	gint bpc;
	gint ct;
};

struct rpdf_images {
	gint number;
	gint image_type;
	gint length;
	gdouble x;
	gdouble y;
	gdouble width;
	gdouble height;
	gchar *data;
	gpointer metadata;
};

struct rpdf_object {
	gboolean put_wrapper;
	gchar *contents;
	gchar *stream;
	gint stream_length;
};

struct rpdf_font_object {
	gint number;
	gchar name[64];
	gchar encoding[64];
	struct _rpdf_fonts *font;
};

struct rpdf_stream_font {
	struct rpdf_font_object *font_object;
	gdouble size;
};

struct rpdf_stream_text {
	gdouble angle;
	gdouble x;
	gdouble y;
	gchar *text;
};

struct rpdf_stream_text_callback {
	gdouble angle;
	gdouble x;
	gdouble y;
	gint len;
	CALLBACK;
	gpointer user_data;
};

struct rpdf_stream_arc {
	gdouble x;
	gdouble y;
	gdouble radius;
	gdouble start_angle;
	gdouble end_angle;
};

struct rpdf_stream_point {
	gdouble x;
	gdouble y;
};

struct rpdf_stream_rect {
	gdouble x;
	gdouble y;
	gdouble width;
	gdouble height;
};

struct rpdf_stream_color {
	gdouble r;
	gdouble g;
	gdouble b;
};

struct rpdf_stream {
	gint type;
	gpointer data;
};

struct rpdf {
	gchar *header;
	gchar *title;
	gchar *subject;
	gchar *author;
	gchar *keywords;
	gchar *creator;
	
	gint current_page;
	gint page_count;
	gint font_count;
	gint annot_count;
	gint image_count;
	
	gint size;
	gchar *out_buffer;
	gboolean has_images;
	gint object_count;
	GSList *objects; 
	GSList *xref;
	GHashTable *fonts;
	
	GSList **page_contents;
	struct rpdf_page_info **page_info;
	
	gboolean text_on;
	gchar *page_data;
	GHashTable *page_fonts;
	gchar *working_obj;
	gint font_obj_number;
	GSList *stream_font_destroyer;
};

struct rpdf *rpdf_new();
gboolean rpdf_finalize(struct rpdf *pdf);
void rpdf_set_title(struct rpdf *pdf, gchar *title);
void rpdf_set_subject(struct rpdf *pdf, gchar *subject);
void rpdf_set_author(struct rpdf *pdf, gchar *author);
void rpdf_set_keywords(struct rpdf *pdf, gchar *keywords);
void rpdf_set_creator(struct rpdf *pdf, gchar *creator);
void rpdf_translate(struct rpdf *pdf, gdouble x, gdouble y);
void rpdf_rotate(struct rpdf *pdf, gdouble angle);
gboolean rpdf_new_page(struct rpdf *pdf, gint paper, gint orientation);
gboolean rpdf_set_page(struct rpdf *pdf, gint page);
gboolean rpdf_set_font(struct rpdf *pdf, gchar *font, gchar *encoding, gdouble size);
gboolean rpdf_set_font_size(struct rpdf *pdf, gdouble size);
gboolean rpdf_text_callback(struct rpdf *pdf, gdouble x, gdouble y, gdouble angle, gint len, CALLBACK, gpointer user_data);
gboolean rpdf_text(struct rpdf *pdf, gdouble x, gdouble y, gdouble angle, gchar *text);
gboolean rpdf_image(struct rpdf *pdf, gdouble x, gdouble y, gdouble width, gdouble height, gint image_type, gchar *file_name);
gboolean rpdf_link(struct rpdf *pdf, gdouble start_x, gdouble start_y, gdouble end_x, gdouble end_y, gchar *url);
gboolean rpdf_moveto(struct rpdf *pdf, gdouble x, gdouble y);
gboolean rpdf_set_line_width(struct rpdf *pdf, gdouble width);
gboolean rpdf_lineto(struct rpdf *pdf, gdouble x, gdouble y);
gboolean rpdf_closepath(struct rpdf *pdf);
gboolean rpdf_stroke(struct rpdf *pdf);
gboolean rpdf_rect(struct rpdf *pdf, gdouble x, gdouble y, gdouble width, gdouble height);
gboolean rpdf_fill(struct rpdf *pdf);
gboolean rpdf_setrgbcolor(struct rpdf *pdf, gdouble r, gdouble g, gdouble b);
gdouble rpdf_text_width(struct rpdf *pdf, gchar *text);
gint rpdf_arc(struct rpdf *pdf, gdouble x, gdouble y, gdouble radius, gdouble start_angle, gdouble end_angle);
gchar *rpdf_get_buffer(struct rpdf *pdf, gint *length);
void rpdf_free(struct rpdf *pdf);
