#!/usr/bin/python
# -*- coding: ISO-8859-15 -*-

import rlib

myreport = rlib.Rlib()
print rlib.version
myreport.add_datasource_xml("local_xml")
myreport.add_query_as("local_xml", "data.xml", "data")
myreport.add_report("array.xml")
myreport.set_output_format_from_text("pdf")
myreport.execute()
print myreport.get_content_type_as_text()
open('xml.pdf','wb').write(myreport.get_output())
