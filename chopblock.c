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


/*
	A silly willy program to shop send HEADER/DEBUG stuff from RLIB to stderr and PDF stuff to stdout
	that way you can php report.php | chopblock | acroread
	and test reports easy
*/
#include <stdio.h>

#define HEADER "Content-Length:"

int main() {
	char header[50] = HEADER;
	char buf[1024];
	char *woot=NULL;
	char dude;
	int size=0;
	int i=0;
	FILE *dest=stderr;
	int hp=0;
	
	while((size=read(0, &buf, 1023)) > 0) {
		for(i=0;i<size;i++) {
			fwrite((char *)&buf[i], 1, 1, dest); //FIX THIS
			if(header[hp] == buf[i])
				hp++;
			else
				hp=0;
			if(hp == strlen(header)) {
				while(read(0, &dude, 1)) {
					if(dude == 10)
						break;
				}
				dest=stdout;
			}
		}
	}
}
