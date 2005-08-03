#!/usr/bin/python
# -*- coding: ISO-8859-15 -*-
 
import rlib

data = [["name", "ty_sales", "ly_sales", "projection", "sold", "negative"],
        ["BK 1", "3000", "3300", "3300", "-1.5", "-2"],
        ["BK 2", "2400", "2100", "3301", "1", "-4"],
        ["BK 3", "4000", "4100", "3302", "2", "-6"],
        ["BK 4", "4700", "4100", "3303", "3", "-8"],
        ["BK 5", "2500", "5500", "3304", "8", "-10"]]

line_data = [["dow", "rest1", "rest2", "rest3", "rest4", "rest5", "rest6", "os1", "os2", "os3", "os4", "os5", "os6"],
             ["Sunday", "100", "120", "130", "110", "120", "130", "5", "10", "-2", "-4", "4", "-1"],
             ["Monday", "120", "110", "120", "140", "150", "160", "-2", "4", "3", "-2", "2", "-2"],
             ["Tuesday", "110", "130", "110", "120", "130", "120", "12", "12", "-5", "2", "-4", "5"],
             ["Wednesday", "30", "50", "40", "90", "30", "40", "-5", "-2", "10", "12", "13", "-4"],
             ["Thusday", "60", "100", "90", "80", "70", "85", "5", "4", "3", "2", "1", "0"],
             ["Friday", "100", "120", "130", "110", "120", "130", "-2", "-3", "-4", "-5", "-6", "-7"],
             ["Saturday", "120", "110", "120", "140", "150", "160", "-2", "2", "-2", "2", "-2", "2"]]

pie_data = [["emotion", "value"],
            ["Happy", "25"],
            ["Sad", "25"],
            ["Grumpy", "25"],
            ["Sleepy", "25"]]

pie_data2 = [["food", "value"],
             ["Pizza", "20"],
             ["Chicken", "30"],
             ["Meat", "20"],
             ["Potatos", "10"],
             ["Vegies", "10"],
             ["Fruit", "15"],
             ["Fish", "5"],
             ["Pasta", "20"],
             ["Ice Cream", "10"],
             ["Fresca", "50"],
             ["Water", "20"],
             ["Soda", "20"],
             ["Milk", "5"],
             ["Oj", "5"],
             ["Cake", "20"],
             ["Pie", "10"],
             ["Pudding", "5"]]

sales_data = [["dow", "net", "check"],
              ["Monday", "8000", "5.00"],
              ["Tuesday", "6000", "5.50"],
              ["Wednesday", "6500", "4.50"],
              ["Thursday", "5000", "7.00"]]

myreport = rlib.open()
myreport.add_datasource_array("local_array")
myreport.add_query_as("local_array", "data", "data")
myreport.add_query_as("local_array", "line_data", "line_data")
myreport.add_query_as("local_array", "pie_data", "pie_data")
myreport.add_query_as("local_array", "pie_data2", "pie_data2")
myreport.add_query_as("local_array", "sales_data", "sales_data")
myreport.set_output_parameter("html_image_directory", "/tmp")
myreport.set_output_parameter("trim_links", "1")
myreport.add_report("graph.xml")
myreport.set_output_format_from_text("pdf")
myreport.execute()
open('graph.pdf','wb').write(myreport.get_output())



