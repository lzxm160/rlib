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

#include "ralloc.h"
#include "rlib.h"
#include "pcode.h"

char * rlib_value_get_type_as_str(struct rlib_value *v) {
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

static void rlib_pcode_operator_fatal_execption(char *operator, int pcount, struct rlib_value *v1, struct rlib_value *v2, struct rlib_value *v3) {
	debugf("RLIB EXPERIENCED A FATAL MATH ERROR WHILE TRYING TO PREFORM THE FOLLOWING OPERATION: %s\n", operator);
	debugf("\tDATA TYPES ARE [%s]", rlib_value_get_type_as_str(v1));
	if(pcount > 1)
		debugf(" [%s]", rlib_value_get_type_as_str(v2));
	if(pcount > 2)
		debugf(" [%s]", rlib_value_get_type_as_str(v3));
	debugf("\n");
}

int rlib_pcode_operator_add(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		long long result = RLIB_VALUE_GET_AS_NUMBER(v1) + RLIB_VALUE_GET_AS_NUMBER(v2);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		char *newstr = rmalloc(strlen(RLIB_VALUE_GET_AS_STRING(v1))+strlen(RLIB_VALUE_GET_AS_STRING(v2))+1);
		memcpy(newstr, RLIB_VALUE_GET_AS_STRING(v2), strlen(RLIB_VALUE_GET_AS_STRING(v2))+1);
		strcat(newstr, RLIB_VALUE_GET_AS_STRING(v1));
		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, newstr));
		rfree(newstr);
		return TRUE;
	}
	if((RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_NUMBER(v2)) || (RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_DATE(v2))) {
		struct rlib_value *number, *date;
		int i;
		struct tm tm_newday;
		if(RLIB_VALUE_IS_DATE(v1)) {
			date = v1;
			number = v2;
		} else {
			date = v2;
			number = v1;
		}
		tm_newday = RLIB_VALUE_GET_AS_DATE(date);
		for(i=0;i<RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(number));i++) {
			bumpday(&tm_newday.tm_year, &tm_newday.tm_mon, &tm_newday.tm_mday);
		}
		rlib_value_stack_push(vs, rlib_value_new_date(&rval_rtn, &tm_newday));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("ADD", 2, v1, v2, NULL);
	return FALSE;
}


int rlib_pcode_operator_subtract(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		long long result = RLIB_VALUE_GET_AS_NUMBER(v2) - RLIB_VALUE_GET_AS_NUMBER(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	if((RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_NUMBER(v2)) || (RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_DATE(v2))) {
		struct rlib_value *number, *date;
		int i;
		struct tm tm_newday;
		if(RLIB_VALUE_IS_DATE(v1)) {
			date = v1;
			number = v2;
		} else {
			date = v2;
			number = v1;
		}
		tm_newday = RLIB_VALUE_GET_AS_DATE(date);
		for(i=0;i<RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(number));i++) {
			bumpdaybackwords(&tm_newday.tm_year, &tm_newday.tm_mon, &tm_newday.tm_mday);
		}
		rlib_value_stack_push(vs, rlib_value_new_date(&rval_rtn, &tm_newday));
		return TRUE;
	}
	
	rlib_pcode_operator_fatal_execption("SUBTRACT", 2, v1, v2, NULL);
	return FALSE;
}

int rlib_pcode_operator_multiply(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		long long result = RLIB_FXP_MUL(RLIB_VALUE_GET_AS_NUMBER(v1), RLIB_VALUE_GET_AS_NUMBER(v2));
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("MULTIPLY", 2, v1, v2, NULL);
	return FALSE;
}

int rlib_pcode_operator_divide(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		long long result = RLIB_FXP_DIV(RLIB_VALUE_GET_AS_NUMBER(v2), RLIB_VALUE_GET_AS_NUMBER(v1));
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("DIVIDE", 2, v1, v2, NULL);
	return FALSE;	
}

int rlib_pcode_operator_mod(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		long long result = RLIB_VALUE_GET_AS_NUMBER(v2) % RLIB_VALUE_GET_AS_NUMBER(v1);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("MOD", 2, v1, v2, NULL);
	return FALSE;		
}

int rlib_pcode_operator_pow(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		long long result = pow(RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v2)), RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v1)));
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result*RLIB_DECIMAL_PERCISION));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("POW", 2, v1, v2, NULL);
	return FALSE;		
}

int rlib_pcode_operator_lte(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) <= RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		if(strcmp(RLIB_VALUE_GET_AS_STRING(v2), RLIB_VALUE_GET_AS_STRING(v1)) <= 0) {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("<=", 2, v1, v2, NULL);
	return FALSE;		
}

int rlib_pcode_operator_lt(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) < RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		if(strcmp(RLIB_VALUE_GET_AS_STRING(v2), RLIB_VALUE_GET_AS_STRING(v1)) < 0) {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}	
	rlib_pcode_operator_fatal_execption("<", 2, v1, v2, NULL);
	return FALSE;		
}

int rlib_pcode_operator_gte(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) >= RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		if(strcmp(RLIB_VALUE_GET_AS_STRING(v2), RLIB_VALUE_GET_AS_STRING(v1)) >= 0) {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption(">=", 2, v1, v2, NULL);
	return FALSE;		
}

int rlib_pcode_operator_gt(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) > RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		if(strcmp(RLIB_VALUE_GET_AS_STRING(v2), RLIB_VALUE_GET_AS_STRING(v1)) > 0) {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption(">", 2, v1, v2, NULL);
	return FALSE;		
}

int rlib_pcode_operator_eql(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) == RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		if(RLIB_VALUE_GET_AS_STRING(v2) == NULL && RLIB_VALUE_GET_AS_STRING(v1) == NULL)
			return TRUE;
		if(RLIB_VALUE_GET_AS_STRING(v2) == NULL || RLIB_VALUE_GET_AS_STRING(v1) == NULL)
			return FALSE;
		if(strcmp(RLIB_VALUE_GET_AS_STRING(v2), RLIB_VALUE_GET_AS_STRING(v1)) == 0) {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		if(memcmp(&RLIB_VALUE_GET_AS_DATE(v2), &RLIB_VALUE_GET_AS_DATE(v1), sizeof(struct tm)) == 0) {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}	
	}
	rlib_pcode_operator_fatal_execption("==", 2, v1, v2, NULL);
	return FALSE;		
}

int rlib_pcode_operator_noteql(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) != RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		if(strcmp(RLIB_VALUE_GET_AS_STRING(v2), RLIB_VALUE_GET_AS_STRING(v1)) != 0) {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		if(memcmp(&RLIB_VALUE_GET_AS_DATE(v2), &RLIB_VALUE_GET_AS_DATE(v1), sizeof(struct tm)) != 0) {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}	
	}
	rlib_pcode_operator_fatal_execption("!=", 2, v1, v2, NULL);
	return FALSE;		
}

int rlib_pcode_operator_and(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) && RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		if(RLIB_VALUE_GET_AS_STRING(v2) && RLIB_VALUE_GET_AS_STRING(v1)) {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
	}
	rlib_pcode_operator_fatal_execption("==", 2, v1, v2, NULL);
	return FALSE;		
}

int rlib_pcode_operator_or(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v2) || RLIB_VALUE_GET_AS_NUMBER(v1))	{
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		if(RLIB_VALUE_GET_AS_STRING(v2) || RLIB_VALUE_GET_AS_STRING(v1)) {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
		} else {
			rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, 0));
		}
		return TRUE;
	}
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, RLIB_DECIMAL_PERCISION));
	}
	rlib_pcode_operator_fatal_execption("==", 2, v1, v2, NULL);
	return FALSE;		
}


int rlib_pcode_operator_abs(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		long long result = abs(RLIB_VALUE_GET_AS_NUMBER(v1));
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("abs", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_ceil(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		long long dec = RLIB_VALUE_GET_AS_NUMBER(v1) % RLIB_DECIMAL_PERCISION;
		long long result = RLIB_VALUE_GET_AS_NUMBER(v1) - dec + RLIB_DECIMAL_PERCISION;
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("ceil", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_floor(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		long long dec = RLIB_VALUE_GET_AS_NUMBER(v1) % RLIB_DECIMAL_PERCISION;
		long long result = RLIB_VALUE_GET_AS_NUMBER(v1) - dec - RLIB_DECIMAL_PERCISION;
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("floor", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_round(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		long long dec = RLIB_VALUE_GET_AS_NUMBER(v1) % RLIB_DECIMAL_PERCISION;
		long long result = RLIB_VALUE_GET_AS_NUMBER(v1);
		if(dec > 0) {
			result -= dec;
			if(dec > (5 * RLIB_DECIMAL_PERCISION / 10))
				result = result * RLIB_DECIMAL_PERCISION;
		}
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("round", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_sin(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		double num = (double)RLIB_VALUE_GET_AS_NUMBER(v1) / (double)RLIB_DECIMAL_PERCISION;
		long long result = sin(num)*RLIB_DECIMAL_PERCISION;
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("sin", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_cos(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		double num = (double)RLIB_VALUE_GET_AS_NUMBER(v1) / (double)RLIB_DECIMAL_PERCISION;
		long long result = cos(num)*RLIB_DECIMAL_PERCISION;
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("cos", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_ln(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		double num = (double)RLIB_VALUE_GET_AS_NUMBER(v1) / (double)RLIB_DECIMAL_PERCISION;
		long long result = log(num)*RLIB_DECIMAL_PERCISION;
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("ln", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_exp(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		double num = (double)RLIB_VALUE_GET_AS_NUMBER(v1) / (double)RLIB_DECIMAL_PERCISION;
		long long result = exp(num)*RLIB_DECIMAL_PERCISION;
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("exp", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_atan(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		double num = (double)RLIB_VALUE_GET_AS_NUMBER(v1) / (double)RLIB_DECIMAL_PERCISION;
		long long result = atan(num)*RLIB_DECIMAL_PERCISION;
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("atan", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_sqrt(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
 	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1)) {
		double num = (double)RLIB_VALUE_GET_AS_NUMBER(v1) / (double)RLIB_DECIMAL_PERCISION;
		long long result = sqrt(num)*RLIB_DECIMAL_PERCISION;
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("sqrt", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_fxpval(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_STRING(v2)) {
		long long result = atoll(RLIB_VALUE_GET_AS_STRING(v2));
		long long decplaces = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v1));
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result*(RLIB_DECIMAL_PERCISION/tentothe(decplaces))));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("fxpval", 2, v1, v2, NULL);
	return FALSE;
}

int rlib_pcode_operator_val(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, rlib_str_to_long_long(RLIB_VALUE_GET_AS_STRING(v1))));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("val", 1, v1, NULL, NULL);
	return FALSE;
}

//TODO: REVISIT THIS
int rlib_pcode_operator_str(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, *v3, rval_rtn;
	char fmtstring[20];
	char dest[30];
	v1 = rlib_value_stack_pop(vs);
	v2 = rlib_value_stack_pop(vs);
	v3 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2) && RLIB_VALUE_IS_NUMBER(v3)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v1) > 0)
			sprintf(fmtstring, "%%%lld.%lldd", RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v2)), RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v1)));
		else
			sprintf(fmtstring, "%%%lld", RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v2)));
		rlib_number_sprintf(dest, fmtstring, v3, 0);
		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, dest));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("str", 3, v1, v2, v3);
	return FALSE;
}

int rlib_pcode_operator_stod(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		int year, month, day;
		struct tm tm_date;
		sscanf(RLIB_VALUE_GET_AS_STRING(v1), "%4d-%2d-%2d", &year, &month, &day);
		bzero(&tm_date, sizeof(struct tm));
		tm_date.tm_year = year-1900;
		tm_date.tm_mon = month-1;
		tm_date.tm_mday = day;
		rlib_value_stack_push(vs, rlib_value_new_date(&rval_rtn, &tm_date));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("stod", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_iif(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_IIF(v1)) {
		struct rlib_pcode_if *rif = RLIB_VALUE_GET_AS_IIF(v1);
		struct rlib_value *result;
		execute_pcode(r, rif->evaulation, vs, this_field_value);
		result = rlib_value_stack_pop(vs);
		if(RLIB_VALUE_IS_NUMBER(result)) {
			if(RLIB_VALUE_GET_AS_NUMBER(result) == 0) {
				return execute_pcode(r, rif->false, vs, this_field_value);
			} else {
				return execute_pcode(r, rif->true, vs, this_field_value);
			}
		} else if(RLIB_VALUE_IS_STRING(result)) {
			if(RLIB_VALUE_GET_AS_STRING(result) == NULL)
				return execute_pcode(r, rif->false, vs, this_field_value);
			else
				return execute_pcode(r, rif->true, vs, this_field_value);
		}
	}
	rlib_pcode_operator_fatal_execption("iif", 2, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_dtos(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1)) {
		char buf[20];
		sprintf(buf, "%04d-%02d-%02d", RLIB_VALUE_GET_AS_DATE(v1).tm_year+1900, RLIB_VALUE_GET_AS_DATE(v1).tm_mon+1, RLIB_VALUE_GET_AS_DATE(v1).tm_mday);
		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, buf));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("dtos", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_year(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1)) {
		long long tmp = (RLIB_VALUE_GET_AS_DATE(v1).tm_year)+1900;
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(tmp)));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("year", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_month(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1)) {
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(RLIB_VALUE_GET_AS_DATE(v1).tm_mon+1)));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("month", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_day(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1)) {
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(RLIB_VALUE_GET_AS_DATE(v1).tm_mday)));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("day", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_upper(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		char *tmp = rstrdup(RLIB_VALUE_GET_AS_STRING(v1));
		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, strupr(tmp)));
		rfree(tmp);
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("upper", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_lower(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		char *tmp = rstrdup(RLIB_VALUE_GET_AS_STRING(v1));
		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, strlwr(tmp)));
		rfree(tmp);
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("lower", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_proper(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		char *tmp = rstrdup(RLIB_VALUE_GET_AS_STRING(v1));
		rlib_value_stack_push(vs, rlib_value_new_string(&rval_rtn, strproper(tmp)));
		rfree(tmp);
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("proper", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_stods(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		int year, month, day, hour, min, sec;
		struct tm tm_date;
		sscanf(RLIB_VALUE_GET_AS_STRING(v1), "%4d%2d%2d%2d%2d%2d", &year, &month, &day, &hour, &min, &sec);
		bzero(&tm_date, sizeof(struct tm));
		tm_date.tm_year = year-1900;
		tm_date.tm_mon = month-1;
		tm_date.tm_mday = day;
		tm_date.tm_hour = hour;
		tm_date.tm_min = min;
		tm_date.tm_sec = sec;
		rlib_value_stack_push(vs, rlib_value_new_date(&rval_rtn, &tm_date));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("stods", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_isnull(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_STRING(v1)) {
		long long result = RLIB_VALUE_GET_AS_STRING(v1) == NULL ? 1 : 0;
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, result));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("isnull", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_dim(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1)) {
		struct tm *request = &RLIB_VALUE_GET_AS_DATE(v1);
		long dim = daysinmonth((request->tm_year)+1900 , request->tm_mon);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(dim)));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("dim", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_wiy(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, rval_rtn;
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1)) {
		char buf[MAXSTRLEN];
		struct tm *request = &RLIB_VALUE_GET_AS_DATE(v1);
		long dim;
		strftime(buf, MAXSTRLEN, "%U", request);
		dim = atol(buf);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(dim)));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("wiy", 1, v1, NULL, NULL);
	return FALSE;
}

int rlib_pcode_operator_wiyo(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	struct rlib_value *v1, *v2, rval_rtn;
	v2 = rlib_value_stack_pop(vs);
	v1 = rlib_value_stack_pop(vs);
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		char buf[MAXSTRLEN];
		struct tm request, *tmp = &RLIB_VALUE_GET_AS_DATE(v1);
		long dim;
		long offset;
		long timetmp;
		request = *tmp;
		offset = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(v2));
		request.tm_mday -= offset;
		timetmp = mktime(&request);
		localtime_r(&timetmp, &request);
		strftime(buf, MAXSTRLEN, "%U", &request);
		dim = atol(buf);
		rlib_value_stack_push(vs, rlib_value_new_number(&rval_rtn, LONG_TO_FXP_NUMBER(dim)));
		return TRUE;
	}
	rlib_pcode_operator_fatal_execption("wiyo", 2, v1, v2, NULL);
	return FALSE;
}