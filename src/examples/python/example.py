#!/usr/bin/python
# -*- coding: ISO-8859-15 -*-

""" Example RLIB program written in python"""

import rlib

# Create a RLIB report instance
myreport = rlib.open()
myreport.add_datasource_mysql("local_mysql", "localhost", "rlib", "rlib", "rlib")
myreport.add_query_as("local_mysql", "select * FROM products", "products")
myreport.add_report("products.xml")
myreport.set_output_format_from_text("pdf")
# Run the report
myreport.execute()
# Send the report to standard out - alternatively use the get_output method to return
# the report data to your program.
myreport.spool()
