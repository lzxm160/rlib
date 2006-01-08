# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _rlib

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



rlib_init = _rlib.rlib_init

rlib_add_datasource_mysql = _rlib.rlib_add_datasource_mysql

rlib_add_datasource_postgre = _rlib.rlib_add_datasource_postgre

rlib_add_datasource_odbc = _rlib.rlib_add_datasource_odbc

rlib_add_datasource_xml = _rlib.rlib_add_datasource_xml

rlib_add_datasource_csv = _rlib.rlib_add_datasource_csv

rlib_add_query_as = _rlib.rlib_add_query_as

rlib_add_report = _rlib.rlib_add_report

rlib_add_report_from_buffer = _rlib.rlib_add_report_from_buffer

rlib_execute = _rlib.rlib_execute

rlib_get_content_type_as_text = _rlib.rlib_get_content_type_as_text

rlib_spool = _rlib.rlib_spool

rlib_set_output_format = _rlib.rlib_set_output_format

rlib_add_resultset_follower_n_to_1 = _rlib.rlib_add_resultset_follower_n_to_1

rlib_add_resultset_follower = _rlib.rlib_add_resultset_follower

rlib_set_output_format_from_text = _rlib.rlib_set_output_format_from_text

rlib_get_output = _rlib.rlib_get_output

rlib_get_output_length = _rlib.rlib_get_output_length

rlib_signal_connect = _rlib.rlib_signal_connect

rlib_signal_connect_string = _rlib.rlib_signal_connect_string

rlib_query_refresh = _rlib.rlib_query_refresh

rlib_add_parameter = _rlib.rlib_add_parameter

rlib_set_locale = _rlib.rlib_set_locale

rlib_set_output_parameter = _rlib.rlib_set_output_parameter

rlib_set_output_encoding = _rlib.rlib_set_output_encoding

rlib_set_datasource_encoding = _rlib.rlib_set_datasource_encoding

rlib_free = _rlib.rlib_free

rlib_version = _rlib.rlib_version

rlib_graph_add_bg_region = _rlib.rlib_graph_add_bg_region

rlib_graph_clear_bg_region = _rlib.rlib_graph_clear_bg_region

rlib_graph_set_x_minor_tick = _rlib.rlib_graph_set_x_minor_tick

rlib_graph_set_x_minor_tick_by_location = _rlib.rlib_graph_set_x_minor_tick_by_location

