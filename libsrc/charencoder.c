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
 * This module implements a characterencoder that converts a character
 * either to or from UTF8 which is the internal language of RLIB.
 * it is implemented as a simple 'class'.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <langinfo.h>
#include <errno.h>

#include "config.h"
#include "rlib.h"
#include "rlib_input.h"

#if DISABLE_UTF8
#else
static const gchar *encode(gchar *encodebuf, size_t len, iconv_t cd, const gchar *txt) {
	size_t slen;
	gchar *dest = encodebuf;
	gint result = 0;
	const gchar *ret = txt;
	const gchar *ttxt = txt;
	int fatal = FALSE;

	encodebuf[0] = '\0';
	if ((txt != NULL) && (cd != (iconv_t) -1) && (*txt != '\0')) {
		slen = r_bytecount(txt);
		while (slen > 0 && !fatal) {
#if ICONV_CONST_CHAR_PP
			result = iconv(cd, (const gchar **) &txt, &slen, &dest, &len);
#else
			result = iconv(cd, (gchar **)&txt, &slen, &dest, &len);
#endif
			if (result < 0) {
				gchar *hex;				
				switch (errno) {
				case EILSEQ:
					if (slen > 0) { 
						++txt;
						--slen;
					}
//Needs more work here to figure out how much to skip ....... Chet.
#if 0
					if (slen > 0) {
						++txt;
						--slen;
					}
#endif
					if (len > 0) { *dest++ = '?'; --len; }
					hex = str2hex(ttxt);
					r_warning("iconv: Invalid sequence changed to ?");
					r_debug("input was [%s]", hex);
					g_free(hex);
					break;
				case EINVAL:
					r_warning("iconv: Partial multibyte sequence at end of input");
					fatal = TRUE;
					break;
				case E2BIG:
					r_error("iconv: Output buffer size reached %d", MAXSTRLEN);
					fatal = TRUE;
					break;
				}
			}	
		}
		*dest = '\0';
		ret = encodebuf;
	}
	return ret;
}
#endif

/*static iconv_t open_input_encoder(const gchar *encoding) {
	if (!g_strcasecmp(encoding, "UTF-8") || !g_strcasecmp(encoding, "UTF8")) {
		return (iconv_t) -1;
	}
	return iconv_open("UTF-8", encoding);
}
*/

static const char *fix_encoding_name(const char *encoding) {
	if ((encoding == NULL)
			|| !g_strcasecmp(encoding, "UTF-8") 
			|| !g_strcasecmp(encoding, "UTF8")
			|| !g_strcasecmp(encoding, "UTF 8")) {
		encoding = "";
	}
	return encoding;
}


rlib_char_encoder *rlib_char_encoder_new(const gchar *encoding, gboolean output) {
	rlib_char_encoder *rce = g_new0(rlib_char_encoder, 1);
	rce->encoder = (iconv_t) -1;
	rce->output = output;
	encoding = fix_encoding_name(encoding);
	g_strlcpy(rce->name, encoding, sizeof(rce->name));
	if (!*encoding) {
		rce->isUTF8 = TRUE;
		g_strlcpy(rce->name, "UTF-8", sizeof(rce->name));
	}
	return rce;
}


void rlib_char_encoder_destroy(rlib_char_encoder **rcep) {
	rlib_char_encoder *rce = *rcep;
	if (rce) {
		if (rce->encoder != (iconv_t) -1) iconv_close(rce->encoder);
		rce->encoder = (iconv_t) -1;
		if (rce->allocatedbuffer) g_free(rce->allocatedbuffer);
		g_free(rce);
	}
	*rcep = NULL;
}


void rlib_char_encoder_set_buffer(rlib_char_encoder *rce, gchar *buf, gint max) {
	rce->buffer = buf;
	rce->bufsize = max;
}


const gchar *rlib_char_encoder_encode(rlib_char_encoder *rce, const gchar *text) {
#if DISABLE_UTF8
	return text;
#else
	const gchar *result = text;
	if (rce) {
		if (!rce->isUTF8) {
			if (!rce->buffer) {
				rce->allocatedbuffer = rce->buffer = g_malloc(MAXSTRLEN);
				rce->buffer[0] = '\0';
				rce->bufsize = MAXSTRLEN;
			}
			if (rce->encoder == (iconv_t) -1) {
				if (!rce->error) { // No previous error on open attempt
					rce->encoder = (rce->output)? iconv_open(rce->name, "UTF-8") : iconv_open("UTF-8", rce->name);
					if (rce->encoder == (iconv_t) -1) {
						rce->error = TRUE;
						r_error("Could not create iconv for %s", rce->name);
					}
				}
			}
			result = encode(rce->buffer, rce->bufsize, rce->encoder, text);
//		if (rce->output) r_debug("encode_text OUTPUT is [%s], input[%s]", result, text);
		}
	} else {
		r_error("Encoder was NULL");
	}
	if (result == NULL) {
		r_error("rlib_char_encoder_encode got NULL result");
		result = "";
	}
	return result;
#endif	
}


const gchar *rlib_char_encoder_get_name(rlib_char_encoder *rce) {
	return rce->name;
}


gboolean rlib_char_encoder_isUTF8(rlib_char_encoder *rce) {
	return rce->isUTF8;
}


//End of rlib_encoder_object



