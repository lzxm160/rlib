all: libr.so chopblock 

#-I/usr/include/gnome-xml -I/home/bdoan/mgrng/slib \

CCFLAGS=-Wall -Wsign-compare -Werror -g -c
CCINC=-I/usr/include/libxml2/libxml -I /usr/include/libxml2 \
        -I/usr/include/php -I/usr/include/php/main -I/usr/include/php/Zend -I/usr/include/php/TSRM -I/usr/include/php/regex \
        -I$(HOME)/mgrng/slib/ -I/usr/include/mysql
LINC=-L/usr/lib/mysql -L$(HOME)/mgrng/slib -lcpdf -lxml2 -lm -lmysqlclient

sql.o: sql.c rlib.h
	gcc $(CCFLAGS) $(CCINC) sql.c

reportgen.o: reportgen.c rlib.h pcode.h
	gcc $(CCFLAGS) $(CCINC) reportgen.c

parsexml.o: parsexml.c rlib.h
	gcc $(CCFLAGS) $(CCINC) parsexml.c

php.o: php.c rlib.h
	gcc $(CCFLAGS) $(CCINC) php.c

pcode.o: pcode.c rlib.h pcode.h
	gcc $(CCFLAGS) $(CCINC) pcode.c

pcode_op_functions.o: pcode_op_functions.c rlib.h pcode.h
	gcc $(CCFLAGS) $(CCINC) pcode_op_functions.c

formatstring.o: formatstring.c rlib.h pcode.h
	gcc $(CCFLAGS) $(CCINC) formatstring.c

init.o: init.c rlib.h
	gcc $(CCFLAGS) $(CCINC) init.c

breaks.o: breaks.c rlib.h pcode.h
	gcc $(CCFLAGS) $(CCINC) breaks.c

fxp.o: fxp.c
	gcc $(CCFLAGS) $(CCINC) fxp.c

util.o: util.c
	gcc $(CCFLAGS) $(CCINC) util.c

resolution.o: resolution.c rlib.h pcode.h
	gcc $(CCFLAGS) $(CCINC) resolution.c

pdf.o: pdf.c rlib.h
	gcc $(CCFLAGS) $(CCINC) pdf.c

html.o: html.c rlib.h
	gcc $(CCFLAGS) $(CCINC) html.c

txt.o: txt.c rlib.h
	gcc $(CCFLAGS) $(CCINC) txt.c

csv.o: csv.c rlib.h
	gcc $(CCFLAGS) $(CCINC) csv.c

libr.so: parsexml.o php.o  reportgen.o sql.o init.o resolution.o util.o pcode.o \
		pcode_op_functions.o formatstring.o fxp.o breaks.o pdf.o html.o \
		txt.o csv.o
	gcc -g -fpic -shared -o libr.so -Wall $(LINC) \
		php.o parsexml.o reportgen.o sql.o init.o resolution.o util.o pcode.o \
		pcode_op_functions.o formatstring.o fxp.o breaks.o pdf.o html.o txt.o \
		csv.o

chopblock: chopblock.c
	gcc -o chopblock chopblock.c

clean:
	rm -f libr.so
	rm -f chopblock
	rm -f *.o
