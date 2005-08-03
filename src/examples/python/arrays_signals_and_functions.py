#!/usr/bin/python
# -*- coding: ISO-8859-15 -*-

"""Example python RLIB interface which uses an array as a datasource
and implements signals as well as
user defined functions"""
import gc
import rlib
 
def mike_roth(a, b):
    return "pancakes are yummier then %s and %s" % (a, b)
 
def bobdoan(a):
    if isinstance(a, str):
        return a.title()
    return a
 
def signalfunc(value):
    print "rlib signal:", value

testarray = (("first_name", "last_name", "color", "group", "breakfast"),
             ("Bob", "Doan", "blue", "1", "Green Eggs And Spam I Am I Am"),
             ("Eric", "Eburuschkin", "Código", "1", "Green Eggs And Spam I Am I Am"),
             ("Mike", "Roth", "yellow", "2", "Green Eggs And Spam I Am I Am"),
             ("Bob", "Kratz", "pink", "2", "Green Eggs And Spam I Am I Am"),
             ("Steve", "Tilden", "purple", "2", "Dude"))
print "Collection point 1",gc.collect()
gc.set_debug(gc.DEBUG_LEAK)
myr = rlib.Rlib()
myr.add_datasource_array("local_array")
myr.add_query_as("local_array", "testarray", "data")
myr.add_report("array.xml")
myr.signal_connect_string("report_start", signalfunc, "Starting report")
myr.signal_connect(rlib.SIGNAL_REPORT_DONE, signalfunc, ("Ending report",))
myr.add_function("bobdoan", bobdoan, 1)
myr.add_function("mikeroth", mike_roth, 2)
myr.set_output_format_from_text("pdf");
myr.execute()
open('arrays_signals_and_functions.pdf','wb').write(myr.get_output())
del myr
print "Collection point 2",gc.collect()
