## bindings/php/Makefile.am for RLIB

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

noinst_PROGRAMS = test1 test2
INCLUDES = $(POSTGRE_CFLAGS) $(LIBXML_CFLAGS) -I${top_srcdir}/libsrc/  

test1_SOURCES= rlib.h test1.c
test1_LDADD = -L${top_srcdir}/libsrc/ $(MYSQL_LIBS) $(POSTGRE_LIBS) $(LIBXML_LIBS) -lcpdf -lr

test2_SOURCES= rlib.h test2.c
test2_LDADD = -L${top_srcdir}/libsrc/ $(MYSQL_LIBS) $(POSTGRE_LIBS) $(LIBXML_LIBS) -lcpdf -lr
