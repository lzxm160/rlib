#!/usr/bin/python
# -*- coding: ISO-8859-15 -*-

import rlib

def callback():
    global myreport, yields
    yields[3][0] = "MIKEROTH";
    yields[3][1] = "CALL";
    yields[3][2] = "BACK";
    myreport.query_refresh()
    return "WHEEEE";

yields = [["item","portions","eqv"],
          ["Hamburger", "1", "100"],
          ["Chicken", "1", "150"],
          ["Pasta", "1", "190"]]

coupons = [["name", "actual_count", "actual_amount", "computed_count", "computed_amount"],
           ["DOLLAR OFF", "0", "0", "0", "0"],
           ["FREE", "0", "0", "3", "-10.23"],
           ["SENIOR 10%", "0", "0", "0", "0"],
           ["15%", "0", "0", "0", "0"]]

deposits = [["time", "bag_id", "manager_id", "amount"],
            ["12:00", "DEPOSIT 1", "101", "2000.00"],
            ["4:00", "DEPOSIT 2", "102", "1000.00"]]

petty_cash = [["time", "name", "amount"],
              ["1:00", "Sugar", "20.00"],
              ["2:00", "Pants", "40.00"]]

misc_income = [["time", "name", "amount"],
               ["1:00", "Birthday Party", "20.00"]]

inv_transfer = [["qty", "name", "from", "to"],
                ["100", "Buns", "1121", "4452"],
                ["400", "Pattys", "1121", "4499"]]

inventory = [["num",  "name", "amount",  "unit", "open", "usage", "received", "transfer_in", "transfer_out", "waste", "pysical_count"],
             ["1", "BUN, REG/PREMIUM", ".50", "DOZEN", "176", "0", "0", "0", "0", "0", "0"],
             ["2", "BUN, KAISER",      ".70", "DOZEN", "176", "0", "0", "0", "0", "0", "0"]]

sillypants = 5

myreport = rlib.open();
myreport.add_datasource_array("local_array");
myreport.add_query_as("local_array", "yields", "yields");
myreport.add_query_as("local_array", "coupons", "coupons");
myreport.add_query_as("local_array", "deposits", "deposits");
myreport.add_query_as("local_array", "petty_cash", "petty_cash");
myreport.add_query_as("local_array", "misc_income", "misc_income");
myreport.add_query_as("local_array", "inv_transfer", "inv_transfer");
myreport.add_query_as("local_array", "inventory", "inventory");
myreport.add_report("fixed_part.xml");
myreport.set_output_format_from_text("html");
myreport.signal_connect_string("part_iteration", callback);
myreport.execute();
myreport.spool();
