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
 
#include <Python.h>

#include "rlib.h"
#include "ralloc.h"
#include "rlib_python.h"

typedef struct {
	PyObject_HEAD
	rlib *r;
} _rlib_PyObject;

extern PyTypeObject _rlib_PyObject_Type;

static PyObject *rlib_bar(PyObject *self, PyObject *args);
static PyObject *rlib_python_init(PyObject *self, PyObject *args);

static PyMethodDef RlibMethods[] = {
   {"init",  rlib_python_init, METH_VARARGS},
   {"bar",  rlib_bar, METH_VARARGS},
   {NULL, NULL}
};
void initlibrpython()
{
   (void) Py_InitModule("librpython", RlibMethods);
}

static PyObject *rlib_bar(PyObject *self, PyObject *args)
{
   char *string;
   int   len;
   if (!PyArg_ParseTuple(args, "s", &string))
       return NULL;
   len = strlen(string);
   return Py_BuildValue("i", len);
}

static PyObject *rlib_python_init(PyObject *self, PyObject *args) {
/*	_rlib_PyObject *r=NULL;

	r = PyObject_New(_rlib_PyObject,& _rlib_PyObject_Type);
	r->r = rlib_init(NULL);
	
	return r;
*/
	return NULL;
}
