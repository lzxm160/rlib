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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#include "rlib.h"
#include "pcode.h"


// START rlib_datetime 'object'
int rlib_datetime_valid_date(struct rlib_datetime *dt) {
	return g_date_valid(&dt->date);
}


int rlib_datetime_valid_time(struct rlib_datetime *dt) {
	return (dt->ltime > 0)? TRUE : FALSE;
}


void rlib_datetime_clear_time(struct rlib_datetime *t) {
	t->ltime = 0;
}


void rlib_datetime_clear_date(struct rlib_datetime *t) {
	g_date_clear(&t->date, 1);
}


void rlib_datetime_clear(struct rlib_datetime *t1) {
	rlib_datetime_clear_time(t1);
	rlib_datetime_clear_date(t1);
}


void rlib_datetime_makesamedate(struct rlib_datetime *target, struct rlib_datetime *chgto) {
	target->date = chgto->date;
}


void rlib_datetime_makesametime(struct rlib_datetime *target, struct rlib_datetime *chgto) {
	target->ltime = chgto->ltime;
}


gint rlib_datetime_compare(struct rlib_datetime *t1, struct rlib_datetime *t2) {
	gint result = 0;
	if (rlib_datetime_valid_date(t1) && rlib_datetime_valid_date(t2)) {
		result = g_date_compare(&t1->date, &t2->date);
	}	
	if ((result == 0) && rlib_datetime_valid_time(t1) && rlib_datetime_valid_time(t2)) {
		result = t1->ltime - t2->ltime;
	}
	return result;
}


void rlib_datetime_set_date(struct rlib_datetime *dt, int y, int m, int d) {
	GDate *t;
	t = g_date_new_dmy(d, m, y);
	dt->date = *t;
	g_date_free(t);
}


void rlib_datetime_set_time(struct rlib_datetime *dt, int h, int m, int s) {
	dt->ltime = 256 * 256 * 256 + h * 256 * 256 + m * 256 + s;
}


static void rlib_datetime_format_date(struct rlib_datetime *dt, char *buf, int max, const char *fmt) {
	if (rlib_datetime_valid_date(dt)) {
		g_date_strftime(buf, max, fmt, &dt->date);
	} else {
		strcpy(buf, "!ERR_DT_D");
		rlogit("Invalid date in format date");
	}
}


static void rlib_datetime_format_time(struct rlib_datetime *dt, char *buf, int max, const char *fmt) {
	time_t now = time(NULL);
	struct tm *tmp = localtime(&now);
	if (rlib_datetime_valid_time(dt)) {
		tmp->tm_hour = ((dt->ltime & 0x00FF0000) >> 16);
		tmp->tm_min = ((dt->ltime & 0x0000FF00) >> 8);
		tmp->tm_sec = (dt->ltime & 0x000000FF);
		strftime(buf, max, fmt, tmp);
	} else {
		strcpy(buf, "!ERR_DT_T");
		rlogit("Invalid time in format time");
	}
}


//separate format string into 2 pcs. one with date, other with time.
static gchar datechars[] = "aAbBcCdDeFgGhJmuUVwWxyY";
static gchar timechars[] = "HIklMpPrRsSTXzZ";
static void split_tdformat(gchar **datefmt, gchar **timefmt, gint *order, const gchar *fmtstr) {
	gint havedate = FALSE, havetime = FALSE;
	gchar *splitpoint = NULL;
	gchar *s, *t;
	gchar *pctptr;
	gint mode = 0;

	*timefmt = *datefmt = NULL;
	*order = 0;
	t = (gchar *) fmtstr;
	while (!splitpoint && (t = g_utf8_strchr(t, bytelength(t), '%'))) {
		pctptr = t;
		t = g_utf8_next_char(t);
		switch (g_utf8_get_char(t)) {
		case '%':
			t = g_utf8_next_char(t);
			break;
		case 'E': //These are prefixes that moderate the next char
		case 'O':
			t = g_utf8_next_char(t);
			//supposed to fall thru - break intentionally missing
		default:
			if ((s = g_utf8_strchr(datechars, bytelength(datechars), g_utf8_get_char(t)))) {
				if (mode && (mode != 1)) splitpoint = pctptr;
				if (!mode) mode = 1; //date first
				havedate = TRUE;
			} else if ((s = g_utf8_strchr(timechars, bytelength(timechars), g_utf8_get_char(t)))) {
				if (mode && (mode != 2)) splitpoint = pctptr;
				if (!mode) mode = 2; // time first
				havetime = TRUE;
			}
			t = g_utf8_next_char(t);
			break;
		}
	}
	switch (mode) {
	case 1: // date first
		if (splitpoint) {
			*timefmt = g_strdup(splitpoint);
			*datefmt = g_strndup(fmtstr, splitpoint - fmtstr);
		} else {
			*datefmt = g_strdup(fmtstr);
		}
		break;
	case 2: // time first
		if (splitpoint) {
			*timefmt = g_strndup(fmtstr, splitpoint - fmtstr);
			*datefmt = g_strdup(splitpoint);
		} else {
			*timefmt = g_strdup(fmtstr);
		}
		break;
	}
	*order = mode;
}


void rlib_datetime_format(struct rlib_datetime *dt, gchar *buf, gint max, const gchar *fmt) {
	gchar *datefmt, *timefmt;
	gint order;
	gchar datebuf[128];
	gchar timebuf[128];
	gint havedate = FALSE, havetime = FALSE;
	
	split_tdformat(&datefmt, &timefmt, &order, fmt);
	*datebuf = *timebuf = '\0';
	if (datefmt && rlib_datetime_valid_date(dt)) {
		rlib_datetime_format_date(dt, datebuf, 127, datefmt);	
		havedate = TRUE;
	} 
	if (timefmt && rlib_datetime_valid_time(dt)) {
		rlib_datetime_format_time(dt, timebuf, 127, timefmt);
		havetime = TRUE;
	}
	if (timefmt && !havetime) {
		r_warning("Attempt to format time with NULL time value");
	}
	if (datefmt && !havedate) {
		r_warning("Attempt to format date with NULL date value");
	}
	switch (order) {
	case 1:
		g_strlcpy(buf, datebuf, max);
		g_strlcat(buf, timebuf, max - bytelength(datebuf));
		break;
	case 2:
		g_strlcpy(buf, timebuf, max);
		g_strlcat(buf, datebuf, max - bytelength(timebuf));
		break;
	default:
		g_strlcpy(buf, "!ERR_DT_NO", max);
		r_error("Datetime format has no date or no format");
		break; // format has no date or time codes ???
	}
	if (datefmt) g_free(datefmt);
	if (timefmt) g_free(timefmt);
}


gint rlib_datetime_daysdiff(struct rlib_datetime *dt, struct rlib_datetime *dt2) {
	return g_date_days_between(&dt->date, &dt2->date);
}


gint rlib_datetime_secsdiff(struct rlib_datetime *dt, struct rlib_datetime *dt2) {
	return dt->ltime - dt2->ltime;
}


// END of rlib_datetime


gchar * rlib_value_get_type_as_str(struct rlib_value *v) {
	if(v == NULL)
		return "(null)";
	if(RLIB_VALUE_IS_NUMBER(v))
		return "number";
	if(RLIB_VALUE_IS_STRING(v))
		return "string";
	if(RLIB_VALUE_IS_DATE(v))
		return "date";
	if(RLIB_VALUE_IS_IIF(v))
		return "iif";
	if(RLIB_VALUE_IS_ERROR(v))
		return "ERROR";
	return "UNKNOWN";
}

static void rlib_pcode_operator_fatal_execption(gchar *operator, gint pcount, struct rlib_value *v1, struct rlib_value *v2, 
struct rlib_value *v3) {
	rlogit("RLIB EXPERIENCED A FATAL MATH ERROR WHILE TRYING TO PREFORM THE FOLLOWING OPERATION: %s\n", operator);
	rlogit("\tDATA TYPES ARE [%s]", rlib_value_get_type_as_str(v1));
	if(pcount > 1)
		rlogit(" [%s]", rlib_value_get_type_as_str(v2));
	if(pcount > 2)
		rlogit(" [%s]", rlib_value_get_type_as_str(v3));
	rlogit("\n");
}

gint rlib_pcode_operator_add(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(v1 != NULL && v2 != NULL) {
		if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
			gint64 result = RLIB_VALUE_GET_AS_NUMBER(v1) + RLIB_VALUE_GET_AS_NUMBER(v2);
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
			return TRUE;
		}
		if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
			gchar *newstr = g_malloc(bytelength(RLIB_VALUE_GET_AS_STRING(v1))+bytelength(RLIB_VALUE_GET_AS_STRING(v2))+1);
			memcpy(newstr, RLIB_VALUE_GET_AS_STRING(v2), bytelength(RLIB_VALUE_GET_AS_STRING(v2))+1);
			strcat(newstr, RLIB_VALUE_GET_AS_STRING(v1));
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, newstr));
			g_free(newstr);
			return TRUE;
		}
		if((RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_NUMBER(v2)) || (RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_DATE(v2))) {
			struct rlib_value *number, *date;
			struct rlib_datetime newday;
			if(RLIB_VALUE_IS_DATE(v1)) {
				date = v1;
				number = v2;
			} else {
				date = v2;
				number = v1;
			}
			newday = RLIB_VALUE_GET_AS_DATE(date);
			g_date_add_days(&newday.date, RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(number)));
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_date(&rval_rtn, &newday));
			return TRUE;
		}
	}
	rlib_pcode_operator_fatal_execption("ADD", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));
	return FALSE;
}


gint rlib_pcode_operator_subtract(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(v1 != NULL && v2 != NULL) {
		if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
			gint64 result = RLIB_VALUE_GET_AS_NUMBER(v2) - RLIB_VALUE_GET_AS_NUMBER(v1);
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
			return TRUE;
		} else if (RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
			struct rlib_datetime *dt1, *dt2;
			gint64 result;
			dt1 = &RLIB_VALUE_GET_AS_DATE(v1);
			dt2 = &RLIB_VALUE_GET_AS_DATE(v2);
			if (rlib_datetime_valid_date(dt1) && rlib_datetime_valid_date(dt2)) {
				result= rlib_datetime_daysdiff(dt2, dt1);
				rlib_value_free(v1);
				rlib_value_free(v2);
				rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(result)));
				return TRUE;
			} else if (rlib_datetime_valid_time(dt1) && rlib_datetime_valid_time(dt2)) {
				result = rlib_datetime_secsdiff(dt2, dt1);
				rlib_value_free(v1);
				rlib_value_free(v2);
				rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(result)));
				return TRUE;
			}
		} else if (RLIB_VALUE_IS_DATE(v2) && RLIB_VALUE_IS_NUMBER(v1)) {
			struct rlib_value *number, *date;
			struct rlib_datetime newday;
			date = v2;
			number = v1;
			newday = RLIB_VALUE_GET_AS_DATE(date);
			g_date_subtract_days(&newday.date, RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(number)));
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_date(&rval_rtn, &newday));
			return TRUE;
		}
	}
	rlib_pcode_operator_fatal_execption("SUBTRACT", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_multiply(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(v1 != NULL && v2 != NULL) {
		if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
			gint64 result = RLIB_FXP_MUL(RLIB_VALUE_GET_AS_NUMBER(v1), RLIB_VALUE_GET_AS_NUMBER(v2));
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
			return TRUE;
		}
	}
	rlib_pcode_operator_fatal_execption("MULTIPLY", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_divide(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(v1 != NULL && v2 != NULL) {
		if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
			gint64 result = RLIB_FXP_DIV(RLIB_VALUE_GET_AS_NUMBER(v2), RLIB_VALUE_GET_AS_NUMBER(v1));
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
			return TRUE;
		}
	}
	rlib_pcode_operator_fatal_execption("DIVIDE", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;	
}

gint rlib_pcode_operator_mod(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(v1 != NULL && v2 != NULL) {
		if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
			gint64 result = RLIB_VALUE_GET_AS_NUMBER(v2) % RLIB_VALUE_GET_AS_NUMBER(v1);
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
			return TRUE;
		}
	}
	rlib_pcode_operator_fatal_execption("MOD", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;		
}

gint rlib_pcode_operator_pow(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(v1 != NULL && v2 != NULL) {
		if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
			gint64 result = pow(RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v2)), RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v1)));
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result*RLIB_DECIMAL_PRECISION));
			return TRUE;
		}
	}
	rlib_pcode_operator_fatal_execption("POW", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;		
}


gint rlib_pcode_operator_lte(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(v1 != NULL && v2 != NULL) {
		if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
			if(RLIB_VALUE_GET_AS_NUMBER(v2) <= RLIB_VALUE_GET_AS_NUMBER(v1))	{
				rlib_value_free(v1);
				rlib_value_free(v2);
				rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
			} else {
				rlib_value_free(v1);
				rlib_value_free(v2);
				rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
			}
			return TRUE;
		}
		if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
			if(g_utf8_collate(RLIB_VALUE_GET_AS_STRING(v2), RLIB_VALUE_GET_AS_STRING(v1)) <= 0) {
				rlib_value_free(v1);
				rlib_value_free(v2);
				rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
			} else {
				rlib_value_free(v1);
				rlib_value_free(v2);
				rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
			}
			return TRUE;
		}
	}
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		struct rlib_datetime *t1, *t2;
		long long val;
		t1 = &RLIB_VALUE_GET_AS_DATE(v1);
		t2 = &RLIB_VALUE_GET_AS_DATE(v2);
		val = (rlib_datetime_compare(t2, t1) <= 0)? RLIB_DECIMAL_PRECISION : 0;
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, val));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("<=", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;		
}


gint rlib_pcode_operator_lt(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(v1 != NULL && v2 != NULL) {
		if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
			if(RLIB_VALUE_GET_AS_NUMBER(v2) < RLIB_VALUE_GET_AS_NUMBER(v1))	{
				rlib_value_free(v1);
				rlib_value_free(v2);
				rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
			} else {
				rlib_value_free(v1);
				rlib_value_free(v2);
				rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
			}
			return TRUE;
		}
		if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
			if(g_utf8_collate(RLIB_VALUE_GET_AS_STRING(v2), RLIB_VALUE_GET_AS_STRING(v1)) < 0) {
				rlib_value_free(v1);
				rlib_value_free(v2);
				rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
			} else {
				rlib_value_free(v1);
				rlib_value_free(v2);
				rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
			}
			return TRUE;
		}	
	}
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		struct rlib_datetime *t1, *t2;
		long long val;
		t1 = &RLIB_VALUE_GET_AS_DATE(v1);
		t2 = &RLIB_VALUE_GET_AS_DATE(v2);
		val = (rlib_datetime_compare(t2, t1) < 0)? RLIB_DECIMAL_PRECISION : 0;
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, val));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("<", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;		
}

gint rlib_pcode_operator_gte(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) >= RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
		} else {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		if(g_utf8_collate(RLIB_VALUE_GET_AS_STRING(v2), RLIB_VALUE_GET_AS_STRING(v1)) >= 0) {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
		} else {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		struct rlib_datetime *t1, *t2;
		long long val;
		t1 = &RLIB_VALUE_GET_AS_DATE(v1);
		t2 = &RLIB_VALUE_GET_AS_DATE(v2);
		val = (rlib_datetime_compare(t2, t1) >= 0)? RLIB_DECIMAL_PRECISION : 0;
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, val));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption(">=", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;		
}

gint rlib_pcode_operator_gt(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) > RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
		} else {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		if(g_utf8_collate(RLIB_VALUE_GET_AS_STRING(v2), RLIB_VALUE_GET_AS_STRING(v1)) > 0) {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
		} else {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		struct rlib_datetime *t1, *t2;
		long long val;
		t1 = &RLIB_VALUE_GET_AS_DATE(v1);
		t2 = &RLIB_VALUE_GET_AS_DATE(v2);
		val = (rlib_datetime_compare(t2, t1) > 0)? RLIB_DECIMAL_PRECISION : 0;
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, val));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption(">", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;		
}

gint rlib_pcode_operator_eql(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) == RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
		} else {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		gint64 push;
		if(RLIB_VALUE_GET_AS_STRING(v2) == NULL && RLIB_VALUE_GET_AS_STRING(v1) == NULL)
			push = RLIB_DECIMAL_PRECISION;
		if(RLIB_VALUE_GET_AS_STRING(v2) == NULL || RLIB_VALUE_GET_AS_STRING(v1) == NULL)
			push = 0;
		if(g_utf8_collate(RLIB_VALUE_GET_AS_STRING(v2), RLIB_VALUE_GET_AS_STRING(v1)) == 0) {
			push = RLIB_DECIMAL_PRECISION;
		} else {
			push = 0;
		}
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_new_number(&rval_rtn, push);
		rlib_value_stack_push(vs, &rval_rtn);
		return TRUE;
	}
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		struct rlib_datetime *t1, *t2;
		long long val;
		t1 = &RLIB_VALUE_GET_AS_DATE(v1);
		t2 = &RLIB_VALUE_GET_AS_DATE(v2);
		val = (rlib_datetime_compare(t2, t1) == 0)? RLIB_DECIMAL_PRECISION : 0;
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, val));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("==", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;		
}

gint rlib_pcode_operator_noteql(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) != RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
		} else {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		if(g_utf8_collate(RLIB_VALUE_GET_AS_STRING(v2), RLIB_VALUE_GET_AS_STRING(v1)) != 0) {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
		} else {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		struct rlib_datetime *t1, *t2;
		long long val;
		t1 = &RLIB_VALUE_GET_AS_DATE(v1);
		t2 = &RLIB_VALUE_GET_AS_DATE(v2);
		val = (rlib_datetime_compare(t2, t1) != 0)? RLIB_DECIMAL_PRECISION : 0;
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, val));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("!=", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;		
}

gint rlib_pcode_operator_and(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) && RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
		} else {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		rlib_value_free(v1);
		rlib_value_free(v2);
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		if(RLIB_VALUE_GET_AS_STRING(v2) && RLIB_VALUE_GET_AS_STRING(v1)) {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
		} else {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
	}
	rlib_pcode_operator_fatal_execption("==", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;		
}

gint rlib_pcode_operator_or(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) || RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
		} else {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		if(RLIB_VALUE_GET_AS_STRING(v2) || RLIB_VALUE_GET_AS_STRING(v1)) {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
		} else {
			rlib_value_free(v1);
			rlib_value_free(v2);
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PRECISION));
	}
	rlib_pcode_operator_fatal_execption("==", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;		
}


gint rlib_pcode_operator_abs(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		gint64 result = abs(RLIB_VALUE_GET_AS_NUMBER(v1));
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("abs", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_ceil(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		gint64 dec = RLIB_VALUE_GET_AS_NUMBER(v1) % RLIB_DECIMAL_PRECISION;
		gint64 result = RLIB_VALUE_GET_AS_NUMBER(v1) - dec + RLIB_DECIMAL_PRECISION;
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("ceil", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_floor(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		gint64 dec = RLIB_VALUE_GET_AS_NUMBER(v1) % RLIB_DECIMAL_PRECISION;
		gint64 result = RLIB_VALUE_GET_AS_NUMBER(v1) - dec - RLIB_DECIMAL_PRECISION;
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("floor", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_round(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		gint64 dec = RLIB_VALUE_GET_AS_NUMBER(v1) % RLIB_DECIMAL_PRECISION;
		gint64 result = RLIB_VALUE_GET_AS_NUMBER(v1);
		if(dec > 0) {
			result -= dec;
			if(dec > (5 * RLIB_DECIMAL_PRECISION / 10))
				result = result * RLIB_DECIMAL_PRECISION;
		}
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("round", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_sin(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		gdouble num = (double)RLIB_VALUE_GET_AS_NUMBER(v1) / (double)RLIB_DECIMAL_PRECISION;
		gint64 result = sin(num)*RLIB_DECIMAL_PRECISION;
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("sin", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_cos(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		gdouble num = (double)RLIB_VALUE_GET_AS_NUMBER(v1) / (double)RLIB_DECIMAL_PRECISION;
		gint64 result = cos(num)*RLIB_DECIMAL_PRECISION;
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("cos", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_ln(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		gdouble num = (double)RLIB_VALUE_GET_AS_NUMBER(v1) / (double)RLIB_DECIMAL_PRECISION;
		gint64 result = log(num)*RLIB_DECIMAL_PRECISION;
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("ln", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_exp(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		gdouble num = (double)RLIB_VALUE_GET_AS_NUMBER(v1) / (double)RLIB_DECIMAL_PRECISION;
		gint64 result = exp(num)*RLIB_DECIMAL_PRECISION;
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("exp", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_atan(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		gdouble num = (double)RLIB_VALUE_GET_AS_NUMBER(v1) / (double)RLIB_DECIMAL_PRECISION;
		gint64 result = atan(num)*RLIB_DECIMAL_PRECISION;
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("atan", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_sqrt(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		gdouble num = (double)RLIB_VALUE_GET_AS_NUMBER(v1) / (double)RLIB_DECIMAL_PRECISION;
		gint64 result = sqrt(num)*RLIB_DECIMAL_PRECISION;
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("sqrt", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_fxpval(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_STRING(v2)) {
		gint64 result = atoll(RLIB_VALUE_GET_AS_STRING(v2));
		gint64 decplaces = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v1));
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result*(RLIB_DECIMAL_PRECISION/tentothe(decplaces))));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("fxpval", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_val(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		gint64 val = rlib_str_to_long_long(RLIB_VALUE_GET_AS_STRING(v1));
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, val));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("val", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

//TODO: REVISIT THIS
gint rlib_pcode_operator_str(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	gchar fmtstring[20];
	gchar dest[30];
	struct rlib_value *v1, *v2, *v3, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	v3 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2) && RLIB_VALUE_IS_NUMBER(v3)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v1) > 0)
			sprintf(fmtstring, "%%%lld.%lldd", RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v2)), RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v1)));
		else
			sprintf(fmtstring, "%%%lld", RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v2)));
		rlib_number_sprintf(dest, fmtstring, v3, 0);
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_free(v3);
		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, dest));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("str", 3, v1, v2, v3);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_free(v3);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}


static gint rlib_pcode_operator_stod_common(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value, int which) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		struct rlib_datetime dt;
		gchar *tstr = RLIB_VALUE_GET_AS_STRING(v1);
		int err = FALSE;

		rlib_datetime_clear(&dt);
		if (which) { //convert time
			gint hour = 12, minute = 0, second = 0; //safe time for date only.
			gchar ampm = 'a';

			if (sscanf(tstr, "%2d:%2d:%2d%c", &hour, &minute, &second, &ampm) != 4) {
				if (sscanf(tstr, "%2d:%2d:%2d", &hour, &minute, &second) != 3) {
					second = 0;
					if (sscanf(tstr, "%2d:%2d%c", &hour, &minute, &ampm) != 3) {
						second = 0;
						ampm = 0;
						if (sscanf(tstr, "%2d:%2d", &hour, &minute) != 2) {
							if (sscanf(tstr, "%2d%2d%2d", &hour, &minute, &second) != 3) {
								second = 0;
								if (sscanf(tstr, "%2d%2d", &hour, &minute) != 2) {
									rlogit("Invalid Date format: stod(%s)", tstr);
									err = TRUE;
								}
							}
						}
					}
				}
			}
			if (toupper(ampm) == 'P') hour += 12;
			hour %= 24;
			if (!err) {
				rlib_datetime_set_time(&dt, hour, minute, second);
			}
		} else { //convert date
			gint year, month, day;
			if (sscanf(tstr, "%4d-%2d-%2d", &year, &month, &day) != 3) {
				if (sscanf(tstr, "%4d%2d%2d", &year, &month, &day) != 3) {
					rlogit("Invalid Date format: stod(%s)", tstr);
					err = TRUE;
				}
			}
			if (!err) {
				rlib_datetime_set_date(&dt, year, month, day);
			}			
		}
		if (!err) {
			rlib_value_free(v1);
			rlib_value_stack_push(vs, rlib_value_new_date(&rval_rtn, &dt));
			return TRUE;
		}
	}
	rlib_pcode_operator_fatal_execption("stod", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_stod(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	return rlib_pcode_operator_stod_common(r, vs, this_field_value, 0);
}

gint rlib_pcode_operator_tstod(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	return rlib_pcode_operator_stod_common(r, vs, this_field_value, 1);
}

gboolean rlib_pcode_operator_iif(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	gboolean thisresult = FALSE;
	struct rlib_value *v1;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_IIF(v1)) {
		struct rlib_pcode_if *rif = RLIB_VALUE_GET_AS_IIF(v1);
		struct rlib_value *result;
		execute_pcode(r, rif->evaulation, vs, this_field_value);
		result = rlib_value_stack_pop(vs);
		if(RLIB_VALUE_IS_NUMBER(result)) {
			if(RLIB_VALUE_GET_AS_NUMBER(result) == 0) {
				rlib_value_free(result);
				rlib_value_free(v1);
				thisresult = execute_pcode(r, rif->false, vs, this_field_value);
			} else {
				rlib_value_free(result);
				rlib_value_free(v1);
				thisresult = execute_pcode(r, rif->true, vs, this_field_value);
			}
		} else if(RLIB_VALUE_IS_STRING(result)) {
			if(RLIB_VALUE_GET_AS_STRING(result) == NULL) {
				rlib_value_free(result);
				rlib_value_free(v1);
				thisresult = execute_pcode(r, rif->false, vs, this_field_value);
			} else {
				rlib_value_free(result);
				rlib_value_free(v1);
				thisresult = execute_pcode(r, rif->true, vs, this_field_value);
			}
		} else {
			rlib_value_free(result);
			rlib_value_free(v1);
			rlogit("CAN'T COMPARE IIF VALUE [%d]\n", RLIB_VALUE_GET_TYPE(result));
		}
	}
	if (!thisresult) rlib_pcode_operator_fatal_execption("iif", 1, v1, NULL, NULL);
	return thisresult;
}


static gboolean rlib_pcode_operator_dtos_common(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value, char *format) {
	struct rlib_value *v1, rval_rtn;
	gboolean result = FALSE;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1)) {
		gchar buf[60];
		struct rlib_datetime *tmp = &RLIB_VALUE_GET_AS_DATE(v1);
		rlib_datetime_format(tmp, buf, sizeof(buf) - 1, format);
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, buf));
		result = TRUE;
	} else {
		rlib_pcode_operator_fatal_execption("dtos", 1, v1, NULL, NULL);
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));
	}
	return result;
}


gboolean rlib_pcode_operator_dateof(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value rval_rtn, *v1;
	
	v1 = rlib_value_stack_pop(vs);
	if (RLIB_VALUE_IS_DATE(v1)) {
		struct rlib_datetime dt = RLIB_VALUE_GET_AS_DATE(v1);
		rlib_datetime_clear_time(&dt);
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_date(&rval_rtn, &dt));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("dateof", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}


gboolean rlib_pcode_operator_timeof(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value rval_rtn, *v1;
	
	v1 = rlib_value_stack_pop(vs);
	if (RLIB_VALUE_IS_DATE(v1)) {
		struct rlib_datetime tm_date = RLIB_VALUE_GET_AS_DATE(v1);
		tm_date.date = *g_date_new();
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_date(&rval_rtn, &tm_date));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("dateof", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}


/*
* function setdateof(date1, date2) - makes date of date1 = date of date2.
* 	time of date1 is unchanged.
*/
gint rlib_pcode_operator_chgdateof(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		struct rlib_datetime *pd1 = &RLIB_VALUE_GET_AS_DATE(v1);
		struct rlib_datetime d2 = RLIB_VALUE_GET_AS_DATE(v2);
		rlib_datetime_makesamedate(&d2, pd1);
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_date(&rval_rtn, &d2));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("chgdateof", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}


gint rlib_pcode_operator_chgtimeof(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		struct rlib_datetime *pd1 = &RLIB_VALUE_GET_AS_DATE(v1);
		struct rlib_datetime d2 = RLIB_VALUE_GET_AS_DATE(v2);
		rlib_datetime_makesametime(&d2, pd1);
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_date(&rval_rtn, &d2));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("chgtimeof", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}


gboolean rlib_pcode_operator_dtos(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	return rlib_pcode_operator_dtos_common(r, vs, this_field_value, "%Y-%m-%d");
}


gboolean rlib_pcode_operator_dtosf(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value rval_rtn, *v1 = rlib_value_stack_pop(vs);
	gboolean result = FALSE;
	if (RLIB_VALUE_IS_STRING(v1)) {
		gchar *fmt = g_strdup(RLIB_VALUE_GET_AS_STRING(v1));
		rlib_value_free(v1);
		result = rlib_pcode_operator_dtos_common(r, vs, this_field_value, fmt);
		g_free(fmt);
	} else {
		rlib_pcode_operator_fatal_execption("dtosf", 1, v1, NULL, NULL);
		rlib_value_free(v1);
		v1 = rlib_value_stack_pop(vs); //clear the stack
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	}
	return result;
}


gint rlib_pcode_operator_year(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1)) {
		gint64 tmp = g_date_get_year(&RLIB_VALUE_GET_AS_DATE(v1).date);
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(tmp)));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("year", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}


gint rlib_pcode_operator_month(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1)) {
		int mon = g_date_get_month(&RLIB_VALUE_GET_AS_DATE(v1).date);
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(mon)));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("month", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_day(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1)) {
		int day = g_date_get_day(&RLIB_VALUE_GET_AS_DATE(v1).date);
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(day)));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("day", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_upper(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		gchar *tmp = g_strdup(RLIB_VALUE_GET_AS_STRING(v1));
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, g_utf8_strup(tmp, -1)));
		g_free(tmp);
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("upper", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_lower(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		gchar *tmp = g_strdup(RLIB_VALUE_GET_AS_STRING(v1));
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, g_utf8_strdown(tmp, -1)));
		g_free(tmp);
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("lower", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}


gint rlib_pcode_operator_left(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v2) && RLIB_VALUE_IS_NUMBER(v1)) {
		gchar *tmp = g_strdup(RLIB_VALUE_GET_AS_STRING(v2));
		gint n = RLIB_VALUE_GET_AS_NUMBER(v1)/RLIB_DECIMAL_PRECISION;
		if (n >= 0) {
			if (charcount(tmp) > n) *g_utf8_offset_to_pointer(tmp, n) = '\0';
		}
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, tmp));
		g_free(tmp);
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("left", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_right(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v2) && RLIB_VALUE_IS_NUMBER(v1)) {
		gchar *tmp = g_strdup(RLIB_VALUE_GET_AS_STRING(v2));
		gint n = RLIB_VALUE_GET_AS_NUMBER(v1)/RLIB_DECIMAL_PRECISION;
		gint len = charcount(tmp);
		if (n >= 0) {
			if (n > len) n = len;
		}
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, g_utf8_offset_to_pointer(tmp, len - n)));
		g_free(tmp);
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("left", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_substring(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, *v3, rval_rtn;
	v1 = rlib_value_stack_pop(vs); // the new size
	v2 = rlib_value_stack_pop(vs); // the start idx
	v3 = rlib_value_stack_pop(vs); //the string
	if(RLIB_VALUE_IS_STRING(v3) && RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		gchar *tmp = g_strdup(RLIB_VALUE_GET_AS_STRING(v3));
		gint st = RLIB_VALUE_GET_AS_NUMBER(v2)/RLIB_DECIMAL_PRECISION;
		gint sz = RLIB_VALUE_GET_AS_NUMBER(v1)/RLIB_DECIMAL_PRECISION;
		gint len = charcount(tmp);
		gint maxlen;
		if (st < 0) st = 0;
		if (st > len) st = len;
		maxlen = len - st;
		if (sz < 0) sz = maxlen;
		if (sz > maxlen) sz = maxlen;
		*g_utf8_offset_to_pointer(tmp, st + sz) = '\0';
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_free(v3);
		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, g_utf8_offset_to_pointer(tmp, st)));
		g_free(tmp);
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("left", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_free(v3);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_proper(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		gchar *tmp = g_strdup(RLIB_VALUE_GET_AS_STRING(v1));
		rlib_value_free(v1);

//TODO: find or write a utf8 version  of strproper.

		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, strproper(tmp)));
		g_free(tmp);
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("proper", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_stods(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		gint year, month, day, hour, min, sec;
		struct rlib_datetime dt;
		sscanf(RLIB_VALUE_GET_AS_STRING(v1), "%4d%2d%2d%2d%2d%2d", &year, &month, &day, &hour, &min, &sec);
		rlib_datetime_clear(&dt);
		rlib_datetime_set_date(&dt, year, month, day);
		rlib_datetime_set_time(&dt, hour, min, sec);
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_date(&rval_rtn, &dt));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("stods", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_isnull(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		gint64 result = RLIB_VALUE_GET_AS_STRING(v1) == NULL ? 1 : 0;
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("isnull", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_dim(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1)) {
		struct rlib_datetime *request = &RLIB_VALUE_GET_AS_DATE(v1);
		gint dim = g_date_get_days_in_month(g_date_get_month(&request->date), g_date_get_year(&request->date));
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(dim)));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("dim", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_wiy(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1)) {
		struct rlib_datetime *request = &RLIB_VALUE_GET_AS_DATE(v1);
		gint dim = g_date_get_monday_week_of_year(&request->date);
		rlib_value_free(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(dim)));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("wiy", 1, v1, NULL, NULL);
	rlib_value_free(v1);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_wiyo(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v2 = rlib_value_stack_pop(vs);
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		struct rlib_datetime request = RLIB_VALUE_GET_AS_DATE(v1);
		gint dim;
		gint offset;
		offset = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v2));
		g_date_subtract_days(&request.date, offset);
		dim = g_date_get_monday_week_of_year(&request.date);
		rlib_value_free(v1);
		rlib_value_free(v2);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(dim)));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("wiyo", 2, v1, v2, NULL);
	rlib_value_free(v1);
	rlib_value_free(v2);
	rlib_value_stack_push(vs, rlib_value_new_error(&rval_rtn));		
	return FALSE;
}

gint rlib_pcode_operator_date(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_datetime dt;
	struct rlib_value rval_rtn;
	struct tm *tmp = localtime(&r->now);
	rlib_datetime_clear(&dt);
	rlib_datetime_set_date(&dt, tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday);
	rlib_datetime_set_time(&dt, tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
	rlib_value_stack_push(vs, rlib_value_new_date(&rval_rtn, &dt));
	return TRUE;
}
