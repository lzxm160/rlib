#!/usr/bin/python
# -*- coding: ISO-8859-15 -*-

"""Example python RLIB interface which uses arrays as a datasource.
"""

import rlib
 
testarray = (("first_name", "last_name", "color", "group", "breakfast"),
             ("Bob", "Doan", "blue", "1", "Green Eggs And Spam I Am I Am"),
             ("Eric", "Eburuschkin", "green", "1", "Green Eggs And Spam I Am I Am"),
             ("Mike", "Roth", "yellow", "2", "Green Eggs And Spam I Am I Am"),
             ("Bob", "Kratz", "pink", "2", "Green Eggs And Spam I Am I Am"),
             ("Steve", "Tilden", "purple", "2", "Dude"))

moredata = [["initials"],
            ["WRD"],
            ["ERB"]]


myr = rlib.Rlib()
myr.add_datasource_array("local_array")
myr.add_query_as("local_array", "testarray", "data")
myr.add_query_as("local_array", "moredata", "more_data")
myr.add_resultset_follower("data", "more_data")
myr.add_report("follower.xml")
myr.set_output_format_from_text("pdf")
myr.execute()
open('follower.pdf','wb').write(myr.get_output())
