/*
 *  Copyright (C) 2003-2005 SICOM Systems, INC.
 *
 *  Authors: Chet Heilman <cheilman@sicompos.com>
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
 * header definitions for the rlib_output_encoder class.
 *
 */
#ifndef CHARENCODER_H
#define CHARENCODER_H

#include <iconv.h>
#include <glib.h>

#include "charencoder.h"

#define ENCODING_NAME_SIZE	64

struct rlib_char_encoder {
	iconv_t encoder;
	gchar name[ENCODING_NAME_SIZE];
	gboolean output; //0 = name->UTF8, 1 = UTF8->name
	gboolean isUTF8;
	gboolean error;
	gchar *allocatedbuffer;
	gchar *buffer;
	gint bufsize;
};
typedef struct rlib_char_encoder rlib_char_encoder;

rlib_char_encoder *rlib_char_encoder_new(const char *encoding, int output);
void rlib_char_encoder_destroy(rlib_char_encoder **rcep);
const gchar *rlib_char_encoder_encode(rlib_char_encoder *rce, const gchar *text);
void rlib_char_encoder_set_buffer(rlib_char_encoder *rce, gchar *buf, gint max);
inline const gchar *rlib_char_encoder_get_name(rlib_char_encoder *rce);
inline gboolean rlib_char_encoder_isUTF8(rlib_char_encoder *rce);

#endif

