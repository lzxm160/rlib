/*
 *  Copyright (C) 2003 SICOM Systems, INC.
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
 */

#include <string.h>

#include "config.h"
#include "rlib.h"
#include "rlib_input.h"

static gint rlib_navigate_followers(rlib *r, gint my_leader, gint way) {
	gint i, rtn = TRUE, follower = -1;
	for(i=0;i<r->resultset_followers_count;i++) {
		if(r->followers[i].leader == my_leader) {
			follower = r->followers[i].follower;
			if(way == RLIB_NAVIGATE_NEXT) {
				if(rlib_navigate_next(r, follower) != TRUE)
					rtn = FALSE;
			} else if(way == RLIB_NAVIGATE_PREVIOUS) {
				if(rlib_navigate_previous(r, follower) != TRUE)
					rtn = FALSE;
			} else if(way == RLIB_NAVIGATE_FIRST) {
				if(rlib_navigate_first(r, follower) != TRUE)
					rtn = FALSE;
			} else if(way == RLIB_NAVIGATE_LAST) {
				if(rlib_navigate_last(r, follower) != TRUE)
					rtn = FALSE;
			}
		}
	}
	return rtn;
}

gint rlib_navigate_next(rlib *r, gint resultset_num) {
	gint rtn;
	rtn = INPUT(r, resultset_num)->next(INPUT(r, resultset_num), r->results[resultset_num].result);
	if(rtn == TRUE)
		return rlib_navigate_followers(r, resultset_num, RLIB_NAVIGATE_NEXT);
	else
		return FALSE;
}

gint rlib_navigate_previous(rlib *r, gint resultset_num) {
	gint rtn;
	rtn = INPUT(r, resultset_num)->previous(INPUT(r, resultset_num), r->results[resultset_num].result);
	if(rtn == TRUE)
		return rlib_navigate_followers(r, resultset_num, RLIB_NAVIGATE_PREVIOUS);
	else
		return FALSE;
}

gint rlib_navigate_first(rlib *r, gint resultset_num) {
	gint rtn;
	rtn = INPUT(r, resultset_num)->first(INPUT(r, resultset_num), r->results[resultset_num].result);
	if(rtn == TRUE)
		return rlib_navigate_followers(r, resultset_num, RLIB_NAVIGATE_FIRST);
	else
		return FALSE;
}

gint rlib_navigate_last(rlib *r, gint resultset_num) {
	gint rtn;
	rtn = INPUT(r, resultset_num)->last(INPUT(r, resultset_num), r->results[resultset_num].result);
	if(rtn == TRUE)
		return rlib_navigate_followers(r, resultset_num, RLIB_NAVIGATE_LAST);
	else
		return FALSE;
}
