## libsrc/Makefile.am for RLIB

## Copyright (c) 2003 SICOM Systems Inc (bdoan@sicompos.com)
## rlib is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.

## rlib is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

## Report bugs to rlib-devel@lists.sourceforge.net
##
## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libr.la
libr_la_SOURCES = parsexml.c reportgen.c api.c resolution.c util.c pcode.c \
	pcode_op_functions.c formatstring.c fxp.c breaks.c pdf.c html.c txt.c \
	datasource.c csv.c ralloc.c environment.c free.c navigation.c save.c rlib_input.h \
	pcode.h ralloc.h rlib.h containers.h containers.c

libr_la_LIBADD = $(RLIB_LIBS) -ldl 

INCLUDES = $(RLIB_CFLAGS)

include_HEADERS = rlib.h rlib_input.h

libr_la_LDFLAGS = -release $(RLIB_VERSION)


includedir=$(prefix)/include/rlib
