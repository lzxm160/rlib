## bindings/perl/Makefile.am for RLIB

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

lib_LTLIBRARIES = librlibperl.la
librlibperl_la_SOURCES = perl.c 
librlibperl_la_LIBADD = -L${top_builddir}/libsrc/ -lr $(RLIB_LIBS)
INCLUDES = -I${top_srcdir}/libsrc/ $(RLIB_CFLAGS) $(PERL_CFLAGS) -D_GNU_SOURCE -fno-strict-aliasing
librlibperl_la_LDFLAGS = -release $(RLIB_VERSION)



## libdir = $(PERL_EXTENSION_DIR)
