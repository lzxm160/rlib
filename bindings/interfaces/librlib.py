# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _librlib

def _swig_setattr(self,class_type,name,value):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    self.__dict__[name] = value

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types



rlib_init = _librlib.rlib_init

rlib_add_datasource_mysql = _librlib.rlib_add_datasource_mysql

rlib_add_datasource_postgre = _librlib.rlib_add_datasource_postgre

rlib_add_datasource_odbc = _librlib.rlib_add_datasource_odbc

rlib_add_query_as = _librlib.rlib_add_query_as

rlib_add_report = _librlib.rlib_add_report

rlib_add_report_from_buffer = _librlib.rlib_add_report_from_buffer

rlib_execute = _librlib.rlib_execute

rlib_get_content_type_as_text = _librlib.rlib_get_content_type_as_text

rlib_spool = _librlib.rlib_spool

rlib_set_output_format = _librlib.rlib_set_output_format

rlib_add_resultset_follower_n_to_1 = _librlib.rlib_add_resultset_follower_n_to_1

rlib_add_resultset_follower = _librlib.rlib_add_resultset_follower

rlib_set_output_format_from_text = _librlib.rlib_set_output_format_from_text

rlib_get_output = _librlib.rlib_get_output

rlib_get_output_length = _librlib.rlib_get_output_length

rlib_signal_connect = _librlib.rlib_signal_connect

rlib_signal_connect_string = _librlib.rlib_signal_connect_string

rlib_query_refresh = _librlib.rlib_query_refresh

rlib_add_parameter = _librlib.rlib_add_parameter

rlib_set_locale = _librlib.rlib_set_locale

rlib_set_output_parameter = _librlib.rlib_set_output_parameter

rlib_set_output_encoding = _librlib.rlib_set_output_encoding

rlib_set_database_encoding = _librlib.rlib_set_database_encoding

rlib_set_datasource_encoding = _librlib.rlib_set_datasource_encoding

rlib_set_parameter_encoding = _librlib.rlib_set_parameter_encoding

rlib_set_encodings = _librlib.rlib_set_encodings

rlib_free = _librlib.rlib_free

rlib_version = _librlib.rlib_version

