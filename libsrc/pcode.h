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

#define RLIB_VALUE_ERROR	-1
#define RLIB_VALUE_NONE	0
#define RLIB_VALUE_NUMBER	1
#define RLIB_VALUE_STRING	2
#define RLIB_VALUE_DATE 	3
#define RLIB_VALUE_IIF 	100


#define RLIB_VALUE_GET_TYPE(a) (a->type)
#define RLIB_VALUE_IS_NUMBER(a)	(RLIB_VALUE_GET_TYPE(a)==RLIB_VALUE_NUMBER)
#define RLIB_VALUE_IS_STRING(a)	(RLIB_VALUE_GET_TYPE(a)==RLIB_VALUE_STRING)
#define RLIB_VALUE_IS_DATE(a)	(RLIB_VALUE_GET_TYPE(a)==RLIB_VALUE_DATE)
#define RLIB_VALUE_IS_IIF(a)	(RLIB_VALUE_GET_TYPE(a)==RLIB_VALUE_IIF)
#define RLIB_VALUE_IS_ERROR(a)	(RLIB_VALUE_GET_TYPE(a)==RLIB_VALUE_ERROR)
#define RLIB_VALUE_IS_NONE(a)	(RLIB_VALUE_GET_TYPE(a)==RLIB_VALUE_NONE)
#define RLIB_VALUE_GET_AS_NUMBER(a) (a->number_value)
#define RLIB_VALUE_GET_AS_NUMBERNP(a) ((long long *)&a->number_value)
#define RLIB_VALUE_GET_AS_STRING(a) (a->string_value)
#define RLIB_VALUE_GET_AS_DATE(a) (a->date_value)
#define RLIB_VALUE_GET_AS_IIF(a) ((struct rlib_pcode_if *)a->iif_value)

#define RLIB_FXP_MUL(a, b) fxp_mul(a, b, RLIB_DECIMAL_PERCISION)
#define RLIB_FXP_DIV(num, denom) fxp_div(num, denom, RLIB_FXP_PERCISION)

#define RLIB_FXP_TO_NORMAL_LONG_LONG(a) (a/RLIB_DECIMAL_PERCISION)
#define LONG_TO_FXP_NUMBER(a) ((a)*RLIB_DECIMAL_PERCISION)

#define OP_ADD		1
#define OP_SUB 	2
#define OP_MUL 	3
#define OP_DIV 	4
#define OP_MOD 	5
#define OP_POW 	6
#define OP_LT		7
#define OP_LTE 	8
#define OP_GT		9
#define OP_GTE 	10
#define OP_EQL 	11
#define OP_NOTEQL	12
#define OP_AND		13
#define OP_OR		14
#define OP_LPERN	15
#define OP_RPERN	16
#define OP_COMMA	17
#define OP_ABS 	18
#define OP_CEIL	19
#define OP_FLOOR	20
#define OP_ROUND	21
#define OP_SIN 	22
#define OP_COS 	23
#define OP_LN		24
#define OP_EXP 	25
#define OP_ATAN	26 	
#define OP_SQRT	27
#define OP_FXPVAL	28
#define OP_VAL 	29
#define OP_STR 	30
#define OP_STOD 	31
#define OP_IIF 	32
#define OP_DTOS 	33
#define OP_YEAR 	34
#define OP_MONTH 	35
#define OP_DAY 	36
#define OP_UPPER 	37
#define OP_LOWER 	38
#define OP_PROPER	39
#define OP_STODS 	40
#define OP_ISNULL	41
#define OP_DIM 	42
#define OP_WIY 	43
#define OP_WIYO 	44

struct rlib_value_stack {
	int count;
	struct rlib_value values[100];
};

#define PCODE_PUSH	1
#define PCODE_EXECUTE	2

struct rlib_pcode_operator {
      char *tag;			// What I expect to find in the infix string
      int taglen;		
      int precedence; 
		int is_op;
		int opnum;
		int is_function;
		int	(*execute)(rlib *r, struct rlib_value_stack *, struct rlib_value *this_field_value);
};

struct rlib_pcode_instruction {
	char instruction;
	void *value;
};

struct rlib_pcode {
	int count;
	struct rlib_pcode_instruction instructions[200];
};

struct rlib_pcode_if {
	struct rlib_pcode *evaulation;
	struct rlib_pcode *true;
	struct rlib_pcode *false;		
};

#define OPERAND_NUMBER				1
#define OPERAND_STRING				2
#define OPERAND_DATE 				3
#define OPERAND_FIELD				4
#define OPERAND_VARIABLE			5
#define OPERAND_MEMORY_VARIABLE	6
#define OPERAND_RLIB_VARIABLE		7
#define OPERAND_IIF					8

#define RLIB_RLIB_VARIABLE_PAGENO		1
#define RLIB_RLIB_VARIABLE_VALUE 		2
#define RLIB_RLIB_VARIABLE_LINENO		3
#define RLIB_RLIB_VARIABLE_DETAILCNT	4


struct rlib_pcode_operand {
	char type;
	void *value;
};

#define RLIB_DECIMAL_PERCISION	10000000
#define RLIB_FXP_PERCISION 7

int execute_pcode(rlib *r, struct rlib_pcode *code, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
struct rlib_value * rlib_value_stack_pop(struct rlib_value_stack *vs);
int rlib_value_stack_push(struct rlib_value_stack *vs, struct rlib_value *value);
struct rlib_value * rlib_value_new_number(struct rlib_value *rval, long long value);
struct rlib_value * rlib_value_new_string(struct rlib_value *rval, char *value);
struct rlib_value * rlib_value_new_date(struct rlib_value *rval, struct tm *date);
void rlib_pcode_dump(struct rlib_pcode *p, int offset);

int rlib_pcode_operator_multiply(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_add(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_subtract(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_divide(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_mod(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_pow(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_lt(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_lte(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_gt(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_gte(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_eql(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_noteql(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_and(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_or(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_abs(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_ceil(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_floor(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_round(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_sin(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_cos(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_ln(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_exp(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_atan(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_sqrt(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_fxpval(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_val(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_str(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_stod(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_iif(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_dtos(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_year(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_month(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_day(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_upper(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_lower(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_proper(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_stods(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_isnull(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_dim(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_wiy(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);
int rlib_pcode_operator_wiyo(rlib *r, struct rlib_value_stack *vs, struct rlib_value *this_field_value);

