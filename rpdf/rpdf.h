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
} rpdf_paper [] = {
	{RPDF_PAPER_LETTER,612, 792, "LETTER"},
	{RPDF_PAPER_LEGAL, 612, 1008, "LEGAL"},
	{RPDF_PAPER_A4, 595, 842, "A4"},
	{RPDF_PAPER_B5, 499, 708, "B5"},
	{RPDF_PAPER_C5, 459, 649, "C5"},
	{RPDF_PAPER_DL, 312, 624, "DL"},
	{RPDF_PAPER_EXECUTIVE, 522, 756, "EXECUTIVE"},
	{RPDF_PAPER_COMM10, 297, 684, "COMM10"},
	{RPDF_PAPER_MONARCH, 279, 540, "MONARCH"},
	{RPDF_PAPER_FILM35MM, 528, 792, "FILM35MM"},
	{-1},
};

gint rpdf_courier_width[RPDF_MAX_CHARS] = {600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600,600600,600,600,600,600,600};
gint rpdf_helvetica_width[RPDF_MAX_CHARS] = {278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,355,556,556,889,667,191,333,333,389,584,	278,333,278,278,556,556,556,556,556,556,556,556,556,556,278,278,584,584,584,556,1015,667,667,722,722,667,611,778,722,278,500,667,556,833,722,778,667,778,722,667,611,722,667,944,667,667,611,278,278,278,469,556,333,556,556,500,556,556,278,556,556,222,222,500,222,833,556,556,556,556,333,500,278,556,500,722,500,500,500,334,260,334,584,350,556,350,222,556,333,1000,556,556,333,1000,667,333,1000,350,611,350,350,222,222,333,333,350,556,1000,333,1000,500,333,944,350,500,667,278,333,556,556,556,556,260,556,333,737,370,556,584,333,737,333,400,584,333,333,333,556,537,278,333,333,365,556,834,834,834,611,667,667,667,667,667,667,1000,722,667,667,667,667,278,278,278,278,722,722,778,778,778,778,778,584,778,722,722,722,722,667,667,611,556,556,556,556,556,556,889,500,556,556,556,556,278,278,278,278,556,556,556,556,556,556,556,584,611,556,556,556,556,500,556,500};
gint rpdf_helvetica_bold_width[RPDF_MAX_CHARS] = {278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,333,474,556,556,889,722,238,333,333,389,584,278,333,278,278,556,556,556,556,556,556,556,556,556,556,333,333,584,584,584,611,975,722,722,722,722,667,611,778,722,278,556,722,611,833,722,778,667,778,722,667,611,722,667,944,667,667,611,333,278,333,584,556,333,556,611,556,611,556,333,611,611,278,278,556,278,889,611,611,611,611,389,556,333,611,556,778,556,556,500,389,280,389,584,350,556,350,278,556,500,1000,556,556,333,1000,667,333,1000,350,611,350,350,278,278,500,500,350,556,1000,333,1000,556,333,944,350,500,667,278,333,556,556,556,556,280,556,333,737,370,556,584,333,737,333,400,584,333,333,333,611,556,278,333,333,365,556,834,834,834,611,722,722,722,722,722,722,1000,722,667,667,667,667,278,278,278,278,722,722,778,778,778,778,778,584,778,722,722,722,722,667,667,611,556,556,556,556,556,556,889,556,556,556,556,556,278,278,278,278,611,611,611,611,611,611,611,584,611,611,611,611,611,556,611,556};
gint rpdf_helvetica_italics_width[RPDF_MAX_CHARS] = {278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,355,556,556,889,667,191,333,333,389,584,278,333,278,278,556,556,556,556,556,556,556,556,556,556,278,278,584,584,584,556,1015,667,667,722,722,667,611,778,722,278,500,667,556,833,722,778,667,778,722,667,611,722,667,944,667,667,611,278,278,278,469,556,333,556,556,500,556,556,278,556,556,222,222,500,222,833,556,556,556,556,333,500,278,556,500,722,500,500,500,334,260,334,584,350,556,350,222,556,333,1000,556,556,333,1000,667,333,1000,350,611,350,350,222,222,333,333,350,556,1000,333,1000,500,333,944,350,500,667,278,333,556,556,556,556,260,556,333,737,370,556,584,333,737,333,400,584,333,333,333,556,537,278,333,333,365,556,834,834,834,611,667,667,667,667,667,667,1000,722,667,667,667,667,278,278,278,278,722,722,778,778,778,778,778,584,778,722,722,722,722,667,667,611,556,556,556,556,556,556,889,500,556,556,556,556,278,278,278,278,556,556,556,556,556,556,556,584,611,556,556,556,556,500,556,500};
gint rpdf_helvetica_bold_italics_width[RPDF_MAX_CHARS] = {278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,333,474,556,556,889,722,238,333,333,389,584,278,333,278,278,556,556,556,556,556,556,556,556,556,556,333,333,584,584,584,611,975,722,722,722,722,667,611,778,722,278,556,722,611,833,722,778,667,778,722,667,611,722,667,944,667,667,611,333,278,333,584,556,333,556,611,556,611,556,333,611,611,278,278,556,278,889,611,611,611,611,389,556,333,611,556,778,556,556,500,389,280,389,584,350,556,350,278,556,500,1000,556,556,333,1000,667,333,1000,350,611,350,350,278,278,500,500,350,556,1000,333,1000,556,333,944,350,500,667,278,333,556,556,556,556,280,556,333,737,370,556,584,333,737,333,400,584,333,333,333,611,556,278,333,333,365,556,834,834,834,611,722,722,722,722,722,722,1000,722,667,667,667,667,278,278,278,278,722,722,778,778,778,778,778,584,778,722,722,722,722,667,667,611,556,556,556,556,556,556,889,556,556,556,556,556,278,278,278,278,611,611,611,611,611,611,611,584,611,611,611,611,611,556,611,556};
gint rpdf_times_width[RPDF_MAX_CHARS] = {250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,333,408,500,500,833,778,180,333,333,500,564,250,333,250,278,500,500,500,500,500,500,500,500,500,500,278,278,564,564,564,444,921,722,667,667,722,611,556,722,722,333,389,722,611,889,722,722,556,722,667,556,611,722,722,944,722,722,611,333,278,333,469,500,333,444,500,444,500,444,333,500,500,278,278,500,278,778,500,500,500,500,333,389,278,500,500,722,500,500,444,480,200,480,541,350,500,350,333,500,444,1000,500,500,333,1000,556,333,889,350,611,350,350,333,333,444,444,350,500,1000,333,980,389,333,722,350,444,722,250,333,500,500,500,500,200,500,333,760,276,500,564,333,760,333,400,564,300,300,333,500,453,250,333,300,310,500,750,750,750,444,722,722,722,722,722,722,889,667,611,611,611,611,333,333,333,333,722,722,722,722,722,722,722,564,722,722,722,722,722,722,556,500,444,444,444,444,444,444,667,444,444,444,444,444,278,278,278,278,500,500,500,500,500,500,500,564,500,500,500,500,500,500,500,500};
gint rpdf_times_bold_width[RPDF_MAX_CHARS] = {250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,333,555,500,500,1000,833,278,333,333,500,570,250,333,250,278,500,500,500,500,500,500,500,500,500,500,333,333,570,570,570,500,930,722,667,722,722,667,611,778,778,389,500,778,667,944,722,778,611,778,722,556,667,722,722,1000,722,722,667,333,278,333,581,500,333,500,556,444,556,444,333,500,556,278,333,556,278,833,556,500,556,556,444,389,333,556,500,722,500,500,444,394,220,394,520,350,500,350,333,500,500,1000,500,500,333,1000,556,333,1000,350,667,350,350,333,333,500,500,350,500,1000,333,1000,389,333,722,350,444,722,250,333,500,500,500,500,220,500,333,747,300,500,570,333,747,333,400,570,300,300,333,556,540,250,333,300,330,500,750,750,750,500,722,722,722,722,722,722,1000,722,667,667,667,667,389,389,389,389,722,722,778,778,778,778,778,570,778,722,722,722,722,722,611,556,500,500,500,500,500,500,722,444,444,444,444,444,278,278,278,278,500,556,500,500,500,500,500,570,500,556,556,556,556,500,556,500};
gint rpdf_times_italics_width[RPDF_MAX_CHARS] = {250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,333,420,500,500,833,778,214,333,333,500,675,250,333,250,278,500,500,500,500,500,500,500,500,500,500,333,333,675,675,675,500,920,611,611,667,722,611,611,722,722,333,444,667,556,833,667,722,611,722,611,500,556,722,611,833,611,556,556,389,278,389,422,500,333,500,500,444,500,444,278,500,500,278,278,444,278,722,500,500,500,500,389,389,278,500,444,667,444,444,389,400,275,400,541,350,500,350,333,500,556,889,500,500,333,1000,500,333,944,350,556,350,350,333,333,556,556,350,500,889,333,980,389,333,667,350,389,556,250,389,500,500,500,500,275,500,333,760,276,500,675,333,760,333,400,675,300,300,333,500,523,250,333,300,310,500,750,750,750,500,611,611,611,611,611,611,889,667,611,611,611,611,333,333,333,333,722,667,722,722,722,722,722,675,722,722,722,722,722,556,611,500,500,500,500,500,500,500,667,444,444,444,444,444,278,278,278,278,500,500,500,500,500,500,500,675,500,500,500,500,500,444,500,444};
gint rpdf_times_bold_italics_width[RPDF_MAX_CHARS] = {250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,389,555,500,500,833,778,278,333,333,500,570,250,333,250,278,500,500,500,500,500,500,500,500,500,500,333,333,570,570,570,500,832,667,667,667,722,667,667,722,778,389,500,667,611,889,722,722,611,722,667,556,611,722,667,889,667,611,611,333,278,333,570,500,333,500,500,444,500,444,333,500,556,278,278,500,278,778,556,500,500,500,389,389,278,556,444,667,500,444,389,348,220,348,570,350,500,350,333,500,500,1000,500,500,333,1000,556,333,944,350,611,350,350,333,333,500,500,350,500,1000,333,1000,389,333,722,350,389,611,250,389,500,500,500,500,220,500,333,747,266,500,606,333,747,333,400,570,300,300,333,576,500,250,333,300,300,500,750,750,750,500,667,667,667,667,667,667,944,667,667,667,667,667,389,389,389,389,722,722,722,722,722,722,722,570,722,722,722,722,722,611,611,500,500,500,500,500,500,500,722,444,444,444,444,444,278,278,278,278,500,556,500,500,500,500,500,570,500,556,556,556,556,444,500,444};
gint rpdf_symbol_width[RPDF_MAX_CHARS] = {250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,333,713,500,549,833,778,439,333,333,500,549,250,549,250,278,500,500,500,500,500,500,500,500,500,500,278,278,549,549,549,444,549,722,667,722,612,611,763,603,722,333,631,722,686,889,722,722,768,741,556,592,611,690,439,768,645,795,611,333,863,333,658,500,500,631,549,549,494,439,521,411,603,329,603,549,549,576,521,549,549,521,549,603,439,576,713,686,493,686,494,480,200,480,549,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,750,620,247,549,167,713,500,753,753,753,753,1042,987,603,987,603,400,549,411,549,549,713,494,460,549,549,549,549,1000,603,1000,658,823,686,795,987,768,768,823,768,768,713,713,713,713,713,713,713,768,713,790,790,890,823,549,250,713,603,603,1042,987,603,987,603,494,329,790,790,786,713,384,384,384,384,384,384,494,494,494,494,0,329,274,686,686,686,384,384,384,384,384,384,494,494,494,0};
gint rpdf_dingbats_width[RPDF_MAX_CHARS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,278,974,961,974,980,719,789,790,791,690,960,939,549,855,911,933,911,945,974,755,846,762,761,571,677,763,760,759,754,494,552,537,577,692,786,788,788,790,793,794,816,823,789,841,823,833,816,831,923,744,723,749,790,792,695,776,768,792,759,707,708,682,701,826,815,789,789,707,687,696,689,786,787,713,791,785,791,873,761,762,762,759,759,892,892,788,784,438,138,277,415,392,392,668,668,0,390,390,317,317,276,276,509,509,410,410,234,234,334,334,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,732,544,544,910,667,760,760,776,595,694,626,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,788,894,838,1016,458,748,924,748,918,927,928,928,834,873,828,924,924,917,930,931,463,883,836,836,867,867,696,696,874,0,874,760,946,771,865,771,888,967,888,831,873,927,970,918,0};

struct _rpdf_fonts {
	gchar name[64];
	gchar subtype[64];
	gint *widths;
} rpdf_fonts[] =  {
	{"Courier", "Type1", rpdf_courier_width},
	{"Courier-Bold", "Type1", rpdf_courier_width},
	{"Courier-Oblique", "Type1", rpdf_courier_width},
	{"Courier-BoldOblique", "Type1", rpdf_courier_width},
	{"Helvetica", "Type1", rpdf_helvetica_width},
	{"Helvetica-Bold", "Type1", rpdf_helvetica_bold_width},
	{"Helvetica-Oblique", "Type1", rpdf_helvetica_italics_width},
	{"Helvetica-BoldOblique", "Type1", rpdf_helvetica_bold_italics_width},
	{"Times-Roman", "Type1", rpdf_times_width},
	{"Times-Bold", "Type1", rpdf_times_bold_width},
	{"Times-Italic", "Type1", rpdf_times_italics_width},
	{"Times-BoldItalic", "Type1", rpdf_times_bold_italics_width},
	{"Symbol", "Type1", rpdf_symbol_width},
	{"ZapfDingbats", "Type1", rpdf_dingbats_width},
	{""}
};

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
