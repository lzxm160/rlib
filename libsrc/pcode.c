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

/*
	This will convert a infix string i.e.: 1 * 3 (3+2) + amount + v.whatever
	and convert it to a math stack... sorta like using postfix
	
	See the verbs table for all of the operators that are supported
	
	Operands are 	numbers, 
						field names such as amount or someresultsetname.field
						rlib varables reffereced as v.name

*/

struct rlib_pcode_operator rlib_pcode_verbs[] = {
      {"+",		 	1, 2,	TRUE,	OP_ADD,		FALSE,	rlib_pcode_operator_add},
      {"-",		 	1, 2,	TRUE,	OP_SUB,		FALSE,	rlib_pcode_operator_subtract},
      {"*",		 	1, 4, TRUE,	OP_MUL,		FALSE,	rlib_pcode_operator_multiply},
      {"/",		 	1, 4,	TRUE,	OP_DIV,		FALSE,	rlib_pcode_operator_divide},
      {"%",  	 	1, 4,	TRUE,	OP_MOD,		FALSE,	rlib_pcode_operator_mod},
      {"^",		 	1, 4,	TRUE,	OP_POW,		FALSE,	rlib_pcode_operator_pow},
      {"<=",	 	2, 6,	TRUE,	OP_LTE,		FALSE,	rlib_pcode_operator_lte},
      {"<",		 	1, 6,	TRUE,	OP_LT,		FALSE,	rlib_pcode_operator_lt},
      {">=",	 	2, 6,	TRUE,	OP_GTE,		FALSE,	rlib_pcode_operator_gte},
      {">",		 	1, 6,	TRUE,	OP_GT,		FALSE,	rlib_pcode_operator_gt},
      {"==",	 	2, 6,	TRUE,	OP_EQL,		FALSE,	rlib_pcode_operator_eql},
      {"!=",	 	2, 6,	TRUE,	OP_NOTEQL,	FALSE,	rlib_pcode_operator_noteql},
      {"&&",	 	2, 5,	TRUE,	OP_AND,		FALSE,	rlib_pcode_operator_and},
      {"||",	 	2, 5,	TRUE,	OP_OR,		FALSE,	rlib_pcode_operator_or},
      {"(",		 	1, 0,	FALSE,OP_LPERN,	FALSE,	NULL},
      {")",		 	1, 99,FALSE,OP_RPERN,	FALSE,	NULL},
      {",",		 	1, 99,FALSE,OP_COMMA,	FALSE,	NULL},
      {"abs(",	 	4, 0,	TRUE,	OP_ABS,		TRUE,		rlib_pcode_operator_abs},
      {"ceil(", 	5, 0,	TRUE,	OP_CEIL, 	TRUE, 	rlib_pcode_operator_ceil},
      {"floor(",	6, 0,	TRUE,	OP_FLOOR,	TRUE, 	rlib_pcode_operator_floor},
      {"round(",	6, 0,	TRUE,	OP_ROUND,	TRUE, 	rlib_pcode_operator_round},
      {"sin(",	 	4, 0,	TRUE,	OP_SIN,		TRUE, 	rlib_pcode_operator_sin},
      {"cos(",	 	4, 0,	TRUE,	OP_COS,		TRUE, 	rlib_pcode_operator_cos},
      {"ln(", 	 	3, 0,	TRUE,	OP_LN,		TRUE,		rlib_pcode_operator_ln},
      {"exp(",	 	4, 0,	TRUE,	OP_EXP,		TRUE,		rlib_pcode_operator_exp},
      {"atan(", 	5, 0,	TRUE,	OP_ATAN, 	TRUE, 	rlib_pcode_operator_atan},
      {"sqrt(", 	5, 0,	TRUE,	OP_SQRT, 	TRUE,		rlib_pcode_operator_sqrt},
      {"fxpval(", 7, 0,	TRUE,	OP_FXPVAL, 	TRUE, 	rlib_pcode_operator_fxpval},
      {"val(", 	4, 0,	TRUE,	OP_VAL,  	TRUE,		rlib_pcode_operator_val},
      {"str(", 	4, 0,	TRUE,	OP_STR,  	TRUE, 	rlib_pcode_operator_str},
      {"stod(", 	5, 0,	TRUE,	OP_STOD,  	TRUE,		rlib_pcode_operator_stod},
		{"iif(",  	4, 0, TRUE,	OP_IIF,		TRUE, 	rlib_pcode_operator_iif},
      {"dtos(", 	5, 0,	TRUE,	OP_DTOS,  	TRUE, 	rlib_pcode_operator_dtos},
      {"year(", 	5, 0,	TRUE,	OP_YEAR,  	TRUE, 	rlib_pcode_operator_year},
      {"month(", 	6, 0,	TRUE,	OP_MONTH,  	TRUE, 	rlib_pcode_operator_month},
      {"day(", 	4, 0,	TRUE,	OP_DAY,  	TRUE, 	rlib_pcode_operator_day},
      {"upper(", 	6, 0,	TRUE,	OP_UPPER,  	TRUE, 	rlib_pcode_operator_upper},
      {"lower(", 	6, 0,	TRUE,	OP_LOWER,  	TRUE, 	rlib_pcode_operator_lower},
      {"proper(", 7, 0,	TRUE,	OP_PROPER, 	TRUE,		rlib_pcode_operator_proper},
      {"stodt(", 	6, 0,	TRUE,	OP_STODS,  	TRUE, 	rlib_pcode_operator_stods},
      {"isnull(",	7, 0,	TRUE,	OP_ISNULL, 	TRUE, 	rlib_pcode_operator_isnull},
      {"dim(", 	4, 0,	TRUE,	OP_DIM,  	TRUE, 	rlib_pcode_operator_dim},
      {"wiy(", 	4, 0,	TRUE,	OP_WIY,  	TRUE, 	rlib_pcode_operator_wiy},
      {"wiyo(", 	5, 0,	TRUE,	OP_WIYO,  	TRUE, 	rlib_pcode_operator_wiyo},
      {"date(", 	5, 0,	TRUE,	OP_DATE,  	TRUE, 	rlib_pcode_operator_date},
      { NULL, 	 	0, 0, FALSE,-1,			TRUE,		NULL}
};

struct rlib_pcode_operator * rlib_find_operator(char *ptr) {
	struct rlib_pcode_operator *op;
	int len = strlen(ptr);
	op = rlib_pcode_verbs;
	while(op && op->tag != NULL) {
		if(len >= op->taglen) {		
			if(!strncmp(ptr, op->tag, op->taglen)) {
				return op;
			}
		}
		op++;
	}
	return NULL;
}

void rlib_pcode_init(struct rlib_pcode *p) {
	p->count = 0;
}

int rlib_pcode_add(struct rlib_pcode *p, struct rlib_pcode_instruction *i) {
	p->instructions[p->count++] = *i;	
	return 0;
}

struct rlib_pcode_instruction * rlib_new_pcode_instruction(struct rlib_pcode_instruction *rpi, int instruction, void *value) {
	rpi->instruction = instruction;
	rpi->value = value;
	return rpi;
}

long long rlib_str_to_long_long(char *str) {
	long long foo=0;
	char *other_side=NULL;
	int len=0;
	long long left = 0 , right = 0;
	
	if(str == NULL)
		return 0;
	
	left = atoll(str);
	other_side = index(str, '.');
	if(other_side != NULL) {
		other_side++;
		right = atoll(other_side);
		len = strlen(other_side);
	}
	foo = (left * RLIB_DECIMAL_PERCISION) + (right*(RLIB_DECIMAL_PERCISION/tentothe(len)));
	return foo;
}

int rvalcmp(struct rlib_value *v1, struct rlib_value *v2) {
	if(RLIB_VALUE_IS_NUMBER(v1) && RLIB_VALUE_IS_NUMBER(v2)) {
		if(RLIB_VALUE_GET_AS_NUMBER(v1) == RLIB_VALUE_GET_AS_NUMBER(v2))
			return 0;
		else
			return -1;
	}
	if(RLIB_VALUE_IS_STRING(v1) && RLIB_VALUE_IS_STRING(v2)) {
		return strcmp(RLIB_VALUE_GET_AS_STRING(v1), RLIB_VALUE_GET_AS_STRING(v2));
	}
	if(RLIB_VALUE_IS_DATE(v1) && RLIB_VALUE_IS_DATE(v2)) {
		return memcmp(&RLIB_VALUE_GET_AS_DATE(v1), &RLIB_VALUE_GET_AS_DATE(v2), sizeof(struct tm));
	}
	return -1;
}

struct rlib_pcode_operand * rlib_new_operand(rlib *r, char *str) {
	int resultset;
	void *field=NULL;
	char *memresult;
	struct rlib_pcode_operand *o;
	struct report_variable *rv;
	long rvar;
	o = rmalloc(sizeof(struct rlib_pcode_operand));
	if(str[0] == '\'') {
		char *newstr = rmalloc(strlen(str)-1);
		memcpy(newstr, str+1, strlen(str)-1);
		newstr[strlen(str)-2] = '\0';
		o->type = OPERAND_STRING;
		o->value = newstr;
	} else if(str[0] == '{') {
		struct tm *tm_date = rmalloc(sizeof(struct tm));
		str++;
		o->type = OPERAND_DATE;
		o->value = stod(tm_date, str);				
	} else if((rv = rlib_resolve_variable(r, str))) {
		o->type = OPERAND_VARIABLE;
		o->value = rv;
	} else if((memresult = rlib_resolve_memory_variable(r, str))) {
		o->type = OPERAND_MEMORY_VARIABLE;
		o->value = memresult;
	} else if((rvar = rlib_resolve_rlib_variable(r, str))) {
		o->type = OPERAND_RLIB_VARIABLE;
		o->value = (void *)rvar;
	} 	else if(rlib_resolve_resultset_field(r, str, &field, &resultset)) {
		struct rlib_resultset_field *rf = rmalloc(sizeof(struct rlib_resultset_field));
		rf->resultset = resultset;
		rf->field = field;
		o->type = OPERAND_FIELD;
		o->value = rf;		
	} else {
		long long *newnum = rmalloc(sizeof(long long));
		o->type = OPERAND_NUMBER;
		*newnum = rlib_str_to_long_long(str);
		o->value = newnum;
	}

	return o;
}

void rlib_pcode_dump(struct rlib_pcode *p, int offset) {
	int i,j;
	for(i=0;i<p->count;i++) {
		for(j=0;j<offset*5;j++)
			rlogit(" ");
		rlogit("DUMP: ");
		if(p->instructions[i].instruction == PCODE_PUSH) {
			struct rlib_pcode_operand *o = p->instructions[i].value;
			rlogit("PUSH: ");
			if(o->type == OPERAND_NUMBER)
				rlogit("%lld", *((long long *)o->value));
			else if(o->type == OPERAND_STRING) 
				rlogit("%s", (char *)o->value);
			else if(o->type == OPERAND_FIELD) {
				struct rlib_resultset_field *rf = o->value;
				rlogit("Result Set = [%d]; Field = [%d]", rf->resultset, rf->field);
			} else if(o->type == OPERAND_MEMORY_VARIABLE) {
				rlogit("Result Memory Variable = [%s]", (char *)o->value);
			} else if(o->type == OPERAND_VARIABLE) {
				struct report_variable *rv = o->value;
				rlogit("Result Variable = [%s]", rv->name);
			} else if(o->type == OPERAND_RLIB_VARIABLE) {
				rlogit("RLIB Variable\n");
			} else if(o->type == OPERAND_IIF) {
				struct rlib_pcode_if *rpi = o->value;
				rlogit("*IFF EXPRESSION EVAULATION:\n");
				rlib_pcode_dump(rpi->evaulation, offset+1);
				rlogit("*IFF EXPRESSION TRUE:\n");
				rlib_pcode_dump(rpi->true, offset+1);
				rlogit("*IFF EXPRESSION FALSE:\n");
				rlib_pcode_dump(rpi->false, offset+1);
				rlogit("*IFF DONE\n");
				
			}

		} else if(p->instructions[i].instruction == PCODE_EXECUTE) {
			struct rlib_pcode_operator *o = p->instructions[i].value;
			
		rlogit("EXECUTE: %s", o->tag);
		}
		rlogit("\n");
	}
}

struct operator_stack {
	int count;
	struct rlib_pcode_operator *op[200];
};

int operator_stack_push(struct operator_stack *os, struct rlib_pcode_operator *op) {
	if(op->tag[0] != ')' && op->tag[0] != ',')
		os->op[os->count++] = op;
	return 0;
}

struct rlib_pcode_operator * operator_stack_pop(struct operator_stack *os) {
	if(os->count > 0) {
		return os->op[--os->count];
	} else
		return NULL;
}

void operator_stack_init(struct operator_stack *os) {
	os->count = 0;
}

int operator_stack_is_all_less(struct operator_stack *os, struct rlib_pcode_operator *op) {
	int i;
	if(op->tag[0] == ')' || op->tag[0] == ',')
		return FALSE;

	if(op->tag[0] == '(' || op->is_function == TRUE)
		return TRUE;
	
	if(os->count == 0)
		return TRUE;

	for(i=os->count-1;i>=0;i--) {
		if(os->op[i]->tag[0] == '(' || os->op[i]->is_function == TRUE)
			break;
		if(os->op[i]->precedence >= op->precedence ) 
			return FALSE;
	}
	return TRUE;
}

void popopstack(struct rlib_pcode *p, struct operator_stack *os, struct rlib_pcode_operator *op) {
	struct rlib_pcode_operator *o;
	struct rlib_pcode_instruction rpi;
	if(op != NULL && (op->tag[0] == ')' || op->tag[0] == ',')) {
		while((o=operator_stack_pop(os))) {
			if(o->is_op == TRUE) {
				if(op->tag[0] != ',') {
					rlib_pcode_add(p, rlib_new_pcode_instruction(&rpi, PCODE_EXECUTE, o));
				} else {
					if(o->is_function == FALSE)
						rlib_pcode_add(p, rlib_new_pcode_instruction(&rpi, PCODE_EXECUTE, o));
				}
			}
			if(o->tag[0] == '(' || o->is_function == TRUE) {
				if(op->tag[0] == ',') //NEED TO PUT THE ( or FUNCTION back on the stack cause it takes more then 1 paramater
					operator_stack_push(os, o);
				break;
				
			}
		}		
	} else {
		while((o=operator_stack_pop(os))) {
			if(o->is_op == TRUE)
				rlib_pcode_add(p, rlib_new_pcode_instruction(&rpi, PCODE_EXECUTE, o));
			if(o->tag[0] == '(' || o->is_function == TRUE) 
				break;
		}	
	}
}

void forcepopstack(struct rlib_pcode *p, struct operator_stack *os) {
	struct rlib_pcode_operator *o;
	struct rlib_pcode_instruction rpi;
	while((o=operator_stack_pop(os))) {
		if(o->is_op == TRUE)
			rlib_pcode_add(p, rlib_new_pcode_instruction(&rpi, PCODE_EXECUTE, o));
	}
}


void smart_add_pcode(struct rlib_pcode *p, struct operator_stack *os, struct rlib_pcode_operator *op) {
	if(operator_stack_is_all_less(os, op)) {
		operator_stack_push(os, op);
	} else {		
		popopstack(p, os, op);
		operator_stack_push(os, op);
	}
}


struct rlib_pcode * rlib_infix_to_pcode(rlib *r, char *infix) {
	char *moving_ptr = infix;
	char *op_pointer = infix;
	struct rlib_pcode_operator *op;
	char operand[255];
	struct rlib_pcode *pcodes;
	struct operator_stack os;
	int found_op_last = FALSE;
	int last_op_was_function = FALSE;
	int move_pointers = TRUE;
	int instr=0;
	int indate=0;
	struct rlib_pcode_instruction rpi;

	if(infix == NULL || infix[0] == '\0' || !strcmp(infix, ""))
		return NULL;

	pcodes = rmalloc(sizeof(struct rlib_pcode));
	rlib_pcode_init(pcodes);
	operator_stack_init(&os);
//	strlwrexceptquoted(infix);
	rmwhitespacesexceptquoted(infix);

	while(*moving_ptr) {
		if(*moving_ptr == '\'') {
			if(instr) {
				instr = 0;
				moving_ptr++;
				if(!*moving_ptr)
					break;
			}	else
				instr = 1;
		}
		if(*moving_ptr == '{') {
			indate = 1;
		}
		if(*moving_ptr == '}') {
			indate = 0;
			moving_ptr++;
			if(!*moving_ptr)
				break;
		}
		
		if(!instr && !indate && (op=rlib_find_operator(moving_ptr))) {
			if(moving_ptr != op_pointer) {
				memcpy(operand, op_pointer, moving_ptr - op_pointer);
				operand[moving_ptr - op_pointer] = '\0';
				if(operand[0] != ')') {
					rlib_pcode_add(pcodes, rlib_new_pcode_instruction(&rpi, PCODE_PUSH, rlib_new_operand(r, operand)));
				}
				op_pointer += moving_ptr - op_pointer;
				found_op_last = FALSE;
				last_op_was_function = FALSE;
			}
			if((found_op_last == TRUE  && last_op_was_function == FALSE && op->tag[0] == '-')) {
				move_pointers = FALSE;
			} else {
				found_op_last = TRUE;
/*
				IIF (In Line If's) Are a bit more complicated.  We need to swallow up the whole string like "IIF(1<2,true part, false part)"
				And then idetify all the 3 inner parts, then pass in recursivly to our selfs and populate rlib_pcode_if and smaet_add_pcode that				
*/				
				if(op->opnum == OP_IIF) {
					int pcount=1;
					int ccount=0;
					char *save_ptr, *iif, *save_iif;
					char *evaulation, *true=NULL, *false=NULL;
					struct rlib_pcode_if *rpif;
					struct rlib_pcode_operand *o;
					moving_ptr +=  op->taglen;
					save_ptr = moving_ptr;
					while(*moving_ptr) {
						if(*moving_ptr == '(')
							pcount++;
						if(*moving_ptr == ')')
							pcount--;
						moving_ptr++;
						if(pcount == 0)
							break;
					}
					save_iif = iif = rmalloc(moving_ptr - save_ptr);
					memcpy(iif, save_ptr, moving_ptr-save_ptr);
					iif[moving_ptr-save_ptr-1] = '\0';
					evaulation = iif;
					pcount=0;
					while(*iif) {
						if(*iif == '(')
							pcount++;
						if(*iif == ')')
							pcount--;
						if(*iif == ',' && pcount==0) {
							*iif='\0';
							iif++;
							if(ccount==0)
								true = iif;
							else
								false = iif;
							ccount++;
						}
						iif++;
					}
	
					rpif = rmalloc(sizeof(struct rlib_pcode_if));
					rpif->evaulation = rlib_infix_to_pcode(r, evaulation);			
					rpif->true = rlib_infix_to_pcode(r, true);			
					rpif->false = rlib_infix_to_pcode(r, false);
					rpif->str_ptr = iif;
					smart_add_pcode(pcodes, &os, op);
					o = rmalloc(sizeof(struct rlib_pcode_operand));			
					o->type = OPERAND_IIF;
					o->value = rpif;
					rlib_pcode_add(pcodes, rlib_new_pcode_instruction(&rpi, PCODE_PUSH, o));
					if(1) {
						struct rlib_pcode_operator *ox;
						ox=operator_stack_pop(&os);
						rlib_pcode_add(pcodes, rlib_new_pcode_instruction(&rpi, PCODE_EXECUTE, ox));					
					}
					moving_ptr-=1;
					op_pointer = moving_ptr;
					move_pointers = FALSE;
					rfree(save_iif);
				} else {
					smart_add_pcode(pcodes, &os, op);
					last_op_was_function = op->is_function;
					if(op->tag[0] == ')')
						last_op_was_function = TRUE;
					move_pointers = TRUE;
				}
			}
			if(move_pointers)
				moving_ptr = op_pointer = op_pointer + op->taglen + (moving_ptr - op_pointer);
			else
				moving_ptr++;
		} else
			moving_ptr++;
	}
	if((moving_ptr != op_pointer)) {
		memcpy(operand, op_pointer, moving_ptr - op_pointer);
		operand[moving_ptr - op_pointer] = '\0';
		if(operand[0] != ')') {
			rlib_pcode_add(pcodes, rlib_new_pcode_instruction(&rpi, PCODE_PUSH, rlib_new_operand(r, operand)));
		}
		op_pointer += moving_ptr - op_pointer;
	}
	forcepopstack(pcodes, &os);
	return pcodes;	
}

void rlib_value_stack_init(struct rlib_value_stack *vs) {
	vs->count = 0;
}

int rlib_value_stack_push(struct rlib_value_stack *vs, struct rlib_value *value) {
	if(vs->count == 99)
		return FALSE;
	vs->values[vs->count++] = *value;
	return TRUE;
}

struct rlib_value * rlib_value_stack_pop(struct rlib_value_stack *vs) {
	return &vs->values[--vs->count];
}
struct rlib_value * rlib_value_new(struct rlib_value *rval, int type, int free, void * value) {
	rval->type = type;
	rval->free = free;

	if(type == RLIB_VALUE_NUMBER)
		rval->number_value = *(long long *)value;
	if(type == RLIB_VALUE_STRING)
		rval->string_value = value;
	if(type == RLIB_VALUE_DATE)
		rval->date_value = *(struct tm *)value;
	if(type == RLIB_VALUE_IIF)
		rval->iif_value = value;

	return rval;
}

struct rlib_value * rlib_value_dup(struct rlib_value *orig) {
	struct rlib_value *new;
	new = rmalloc(sizeof(struct rlib_value));
	memcpy(new, orig, sizeof(struct rlib_value));
	if(orig->type == RLIB_VALUE_STRING)
		new->string_value = rstrdup(orig->string_value);
	return new;
}

struct rlib_value * rlib_value_dup_contents(struct rlib_value *rval) {
	if(rval->type == RLIB_VALUE_STRING)
		rval->string_value = rstrdup(rval->string_value);
	return rval;
}


int rlib_value_free(struct rlib_value *rval) {
	if(rval == NULL)
		return FALSE;
	if(rval->free == FALSE)
		return FALSE;
	if(rval->type == RLIB_VALUE_STRING) {
		rfree(rval->string_value);
		rval->free = FALSE;
		RLIB_VALUE_TYPE_NONE(rval);
		return TRUE;
	}
	return FALSE;
}

struct rlib_value * rlib_value_new_number(struct rlib_value *rval, long long value) {
	return rlib_value_new(rval, RLIB_VALUE_NUMBER, FALSE, &value);
}

struct rlib_value * rlib_value_new_string(struct rlib_value *rval, char *value) {
	return rlib_value_new(rval, RLIB_VALUE_STRING, TRUE, rstrdup(value));
}

struct rlib_value * rlib_value_new_date(struct rlib_value *rval, struct tm *date) {
	return rlib_value_new(rval, RLIB_VALUE_DATE, FALSE, date);
}

struct rlib_value * rlib_value_new_error(struct rlib_value *rval) {
	return rlib_value_new(rval, RLIB_VALUE_ERROR, FALSE, NULL);
}

/*
	The RLIB SYMBOL TABLE is a bit commplicated because of all the datasources and internal variables
*/
struct rlib_value *rlib_operand_get_value(rlib *r, struct rlib_value *rval, struct rlib_pcode_operand *o, struct rlib_value *this_field_value) {
	if(o->type == OPERAND_NUMBER) {
		return rlib_value_new(rval, RLIB_VALUE_NUMBER, FALSE, o->value);
	} else if(o->type == OPERAND_STRING) {
		return rlib_value_new(rval, RLIB_VALUE_STRING, FALSE, o->value);
	} else if(o->type == OPERAND_DATE) {
		return rlib_value_new(rval, RLIB_VALUE_DATE, FALSE, o->value);
	} else if(o->type == OPERAND_FIELD) {
		return rlib_value_new_string(rval, rlib_resolve_field_value(r, o->value));
	} else if(o->type == OPERAND_MEMORY_VARIABLE) {
		return rlib_value_new(rval, RLIB_VALUE_STRING, FALSE, o->value);		
	} else if(o->type == OPERAND_RLIB_VARIABLE) {
		long type = ((long)o->value);
		if(type == RLIB_RLIB_VARIABLE_PAGENO) {
			long long pageno = (long long)r->current_page_number*RLIB_DECIMAL_PERCISION;
			return rlib_value_new_number(rval, pageno);
		} else if(type == RLIB_RLIB_VARIABLE_VALUE) {
			return this_field_value;
		} else if(type == RLIB_RLIB_VARIABLE_LINENO) {
			long long cln = (long long)r->current_line_number*RLIB_DECIMAL_PERCISION;
			return rlib_value_new_number(rval, cln);				
		} else if(type == RLIB_RLIB_VARIABLE_DETAILCNT) {
			long long dcnt = (long long)r->detail_line_count * RLIB_DECIMAL_PERCISION;
			return rlib_value_new_number(rval, dcnt);				
		}
	} else if(o->type == OPERAND_VARIABLE) {
		struct report_variable *rv = o->value;
		long long val = 0;
		struct rlib_value *count = &RLIB_VARIABLE_CA(rv)->count;
		struct rlib_value *amount = &RLIB_VARIABLE_CA(rv)->amount;
		if(rv->type == REPORT_VARIABLE_COUNT) {
			val = RLIB_VALUE_GET_AS_NUMBER(count);
		} else if(rv->type == REPORT_VARIABLE_EXPRESSION) {
			val = RLIB_VALUE_GET_AS_NUMBER(amount);
		} else if(rv->type == REPORT_VARIABLE_SUM) {
			val = RLIB_VALUE_GET_AS_NUMBER(amount);
		} else if(rv->type == REPORT_VARIABLE_AVERAGE) {
			val = rlib_fxp_div(RLIB_VALUE_GET_AS_NUMBER(amount), RLIB_VALUE_GET_AS_NUMBER(count), RLIB_FXP_PERCISION);
		} else if(rv->type == REPORT_VARIABLE_LOWEST) {
			val = RLIB_VALUE_GET_AS_NUMBER(amount);
		} else if(rv->type == REPORT_VARIABLE_HIGHEST) {
			val = RLIB_VALUE_GET_AS_NUMBER(amount);
		}
		return rlib_value_new(rval, RLIB_VALUE_NUMBER, TRUE, &val);
	} else if(o->type == OPERAND_IIF) {
		return rlib_value_new(rval, RLIB_VALUE_IIF, FALSE, o->value);
	}
	rlib_value_new(rval, RLIB_VALUE_ERROR, FALSE, NULL);
	return 0;
}

int execute_pcode(rlib *r, struct rlib_pcode *code, struct rlib_value_stack *vs, struct rlib_value *this_field_value) {
	int i;
	for(i=0;i<code->count;i++) {
		if(code->instructions[i].instruction == PCODE_PUSH) {
			struct rlib_pcode_operand *o = code->instructions[i].value;
			struct rlib_value rval;
			rlib_value_stack_push(vs, rlib_operand_get_value(r, &rval, o, this_field_value));
		} else if(code->instructions[i].instruction == PCODE_EXECUTE) {
			struct rlib_pcode_operator *o = code->instructions[i].value;
			if(o->execute != NULL) {
				o->execute(r, vs, this_field_value);
			}
		}
	}
	return TRUE;
}

struct rlib_value * rlib_execute_pcode(rlib *r, struct rlib_value *rval, struct rlib_pcode *code, struct rlib_value *this_field_value) {
	struct rlib_value_stack value_stack;

	if(code == NULL)
		return NULL;

	rlib_value_stack_init(&value_stack);
	execute_pcode(r, code, &value_stack, this_field_value);
	*rval = *rlib_value_stack_pop(&value_stack);
	return rval;		
}
