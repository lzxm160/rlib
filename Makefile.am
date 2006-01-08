## Makefile.am for RLIB

## Copyright (c) 2003-2006 SICOM Systems Inc (bdoan@sicompos.com)
## RLIB is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.

## RLIB is distributed in the hope that it will be useful,
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

EXTRA_DIST = AUTHORS CVS HACKING install-sh README README-JPEG rlib.pc.in  rlib.spec.in

if HAVE_DOC
DOC_DIR=doc
endif


SUBDIRS = rpdf libsrc inputs bindings src tests \
	$(DOC_DIR)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = rlib.pc
