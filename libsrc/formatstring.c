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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "ralloc.h"
#include "rlib.h"
#include "pcode.h"

int rlb_string_sprintf(char *dest, char *fmtstr, struct rlib_value *rval) {
	char *value = RLIB_VALUE_GET_AS_STRING(rval);
	return sprintf(dest, fmtstr, value);
}

int rlib_number_sprintf(char *dest, char *fmtstr, const struct rlib_value *rval, int special_format) {
	int dec=0;
	int left_mul=1;
	int left_padzero=0;
	int left_pad=0;
	int right_mul=1;
	int right_padzero=1;
	int right_pad=0;
	int where=0;
	int commatize=0;
	char *c;

	for(c=fmtstr;*c && (*c != 'd');c++) {
		if(*c=='$')
			commatize=1;
		if(*c=='%')
			where=0;
		else if(*c=='.') {
			dec=1;
			where=1;
		} else if(isdigit(*c)) {		
			if(where==0) {
				if(*c=='0')
					left_padzero = 1;
				else {
					left_pad += (*c-'0')*left_mul;
					left_mul *= 10;
				}
			
			} else {
				if(*c=='0')
					right_padzero = 1;
				else {
					right_pad += (*c-'0')*right_mul;
					right_mul *= 10;
				}				
			}
		}	
	}
	if(rval != NULL) {
		char fleft[20];
		char fright[20];
		char left_holding[20];
		char right_holding[20];
		int ptr=0;
		long long left, right;
		left = RLIB_VALUE_GET_AS_NUMBER(rval) / RLIB_DECIMAL_PERCISION;
		if(special_format)
			left = llabs(left);
		fleft[ptr++]='%';
		if(left_padzero)
			fleft[ptr++]='0';
		if(left_pad)
			if(commatize)
				sprintf(fleft +ptr, "%d'lld", left_pad);
			else
				sprintf(fleft +ptr, "%dlld", left_pad);
		else {
			if(commatize)
				fleft[ptr++] = '\'';
			fleft[ptr++] = 'l';
			fleft[ptr++] = 'l';
			fleft[ptr++] = 'd';
			fleft[ptr++] = '\0';
		}
		sprintf(left_holding, fleft, left);
		strcpy(dest, left_holding);
		dest[strlen(left_holding)] = '\0';
		if(dec) {
			ptr=0;
			if(!special_format && RLIB_VALUE_GET_AS_NUMBER(rval) < 0 && left == 0) {
				char tmp[MAXSTRLEN];
				sprintf(tmp, "-%s", left_holding);
				strcpy(left_holding, tmp);
				strcpy(dest, left_holding);
				dest[strlen(left_holding)] = '\0';
			}
				
			right = llabs(RLIB_VALUE_GET_AS_NUMBER(rval)) % RLIB_DECIMAL_PERCISION;
			fright[ptr++]='%';
			if(right_padzero)
				fright[ptr++]='0';
			if(right_pad)
				sprintf(&fright[ptr], "%dlld", right_pad);
			else {
				fright[ptr++] = 'l';
				fright[ptr++] = 'l';
				fright[ptr++] = 'd';
				fright[ptr++] = '\0';
			}
			right /= tentothe(RLIB_FXP_PERCISION-right_pad);
			sprintf(right_holding, fright, right);
			dest[strlen(left_holding)] = '.';
			strcpy((dest+strlen(left_holding))+1, right_holding);
			dest[strlen(left_holding)+strlen(right_holding)+1]='\0';
		}
	
	}
	return strlen(dest);
}

int rlib_format_string(rlib *r, struct report_field *rf, struct rlib_value *rval, char *buf) {
	if(rf->format == NULL) {
		if(RLIB_VALUE_IS_NUMBER(rval)) {
			sprintf(buf, "%lld", RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(rval)));
		} else if(RLIB_VALUE_IS_STRING(rval)) {
			sprintf(buf, "%s", RLIB_VALUE_GET_AS_STRING(rval));
		} else if(RLIB_VALUE_IS_DATE(rval))  {
			strftime(buf, 100, "%m/%d/%Y", &RLIB_VALUE_GET_AS_DATE(rval));
		} else {
			sprintf(buf, "!ERR_F");
			return FALSE;
		}
	} else {
		struct rlib_value rval_fmtstr2, *rval_fmtstr=&rval_fmtstr2;
		char *formatstring;
		rval_fmtstr = rlib_execute_pcode(r, &rval_fmtstr2, rf->format_code, rval);
		if(!RLIB_VALUE_IS_STRING(rval_fmtstr)) {
			sprintf(buf, "!ERR_F_F");
			return FALSE;
		} else {
			formatstring = RLIB_VALUE_GET_AS_STRING(rval_fmtstr);
			if(RLIB_VALUE_IS_DATE(rval)) {
				strftime(buf, 100, formatstring, &RLIB_VALUE_GET_AS_DATE(rval));				
			} else {	
				int i=0,j=0,pos=0,fpos=0;
				char fmtstr[20];
				int special_format=0;
				char *idx;
				int len_formatstring;
				idx = index(formatstring, ':');
				if(idx != NULL && RLIB_VALUE_IS_NUMBER(rval)) {
					formatstring = rstrdup(formatstring);
					idx = index(formatstring, ':');
					special_format=1;
					if(RLIB_VALUE_GET_AS_NUMBER(rval) >= 0)
						idx[0] = '\0';
					else
						formatstring = idx+1;				
				}
					
				len_formatstring = strlen(formatstring);
				
				for(i=0;i<len_formatstring;i++) {
					if(formatstring[i] == '%' && ((i+1) < len_formatstring && formatstring[i+1] != '%')) {
						while(formatstring[i] != 's' && formatstring[i] != 'd' && i <=len_formatstring) {
							fmtstr[fpos++] = formatstring[i++];
						}
						fmtstr[fpos++] = formatstring[i];
						fmtstr[fpos] = '\0';
						if(fmtstr[fpos-1] == 'd') {
							if(RLIB_VALUE_IS_NUMBER(rval)) {
								char tmp[50];
								
								rlib_number_sprintf(tmp, fmtstr, rval, special_format);
								for(j=0;j<(int)strlen(tmp);j++)
									buf[pos++] = tmp[j];
							} else {
								sprintf(buf, "!ERR_F_D");
								return FALSE;
							}
						} else if(fmtstr[fpos-1] == 's') {
							if(RLIB_VALUE_IS_STRING(rval)) {
								char tmp[500];
								rlb_string_sprintf(tmp, fmtstr, rval);
								for(j=0;j<(int)strlen(tmp);j++)
									buf[pos++] = tmp[j];

							} else {
								sprintf(buf, "!ERR_F_S");
								return FALSE;
							}
						}
					} else {
						buf[pos++] = formatstring[i];
						if(formatstring[i] == '%')
							i++;
					}
				}
				buf[pos++] = '\0'; 
			}
			
		}
	}


	return TRUE;
}
