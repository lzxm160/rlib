<?php 

	function callback() {
		global $rlib;
		global $yields;

		$yields[3][0] = "MIKEROTH";
		$yields[3][1] = "CALL";
		$yields[3][2] = "BACK";

		rlib_query_refresh($rlib);

		return "WHEEEE";
	}  
  
  
	$yields[0][0] = "item";
	$yields[0][1] = "portions";
	$yields[0][2] = "eqv";

	$yields[1][0] = "Hamburger";
	$yields[1][1] = "1";
	$yields[1][2] = "100";

	$yields[2][0] = "Chicken";
	$yields[2][1] = "1";
	$yields[2][2] = "150";

	$yields[3][0] = "Pasta";
	$yields[3][1] = "1";
	$yields[3][2] = "190";

	$coupons[0][0] = "name";
	$coupons[0][1] = "actual_count";
	$coupons[0][2] = "actual_amount";
	$coupons[0][3] = "computed_count";
	$coupons[0][4] = "computed_amount";

	$coupons[1][0] = "DOLLAR OFF";
	$coupons[1][1] = "0";
	$coupons[1][2] = "0";
	$coupons[1][3] = "0";
	$coupons[1][4] = "0";

	$coupons[2][0] = "FREE";
	$coupons[2][1] = "0";
	$coupons[2][2] = "0";
	$coupons[2][3] = "3";
	$coupons[2][4] = "-10.23";

	$coupons[3][0] = "SENIOR 10%";
	$coupons[3][1] = "0";
	$coupons[3][2] = "0";
	$coupons[3][3] = "0";
	$coupons[3][4] = "0";

	$coupons[4][0] = "15%";
	$coupons[4][1] = "0";
	$coupons[4][2] = "0";
	$coupons[4][3] = "0";
	$coupons[4][4] = "0";
	
	$deposits[0][0] = "time";
	$deposits[0][1] = "bag_id";
	$deposits[0][2] = "manager_id";
	$deposits[0][3] = "amount";

	$deposits[1][0] = "12:00";
	$deposits[1][1] = "DEPOSIT 1";
	$deposits[1][2] = "101";
	$deposits[1][3] = "2000.00";

	$deposits[2][0] = "4:00";
	$deposits[2][1] = "DEPOSIT 2";
	$deposits[2][2] = "102";
	$deposits[2][3] = "1000.00";

	$petty_cash[0][0] = "time";
	$petty_cash[0][1] = "name";
	$petty_cash[0][2] = "amount";

	$petty_cash[1][0] = "1:00";
	$petty_cash[1][1] = "Sugar";
	$petty_cash[1][2] = "20.00";

	$petty_cash[2][0] = "2:00";
	$petty_cash[2][1] = "Pants";
	$petty_cash[2][2] = "40.00";

	$misc_income[0][0] = "time";
	$misc_income[0][1] = "name";
	$misc_income[0][2] = "amount";

	$misc_income[1][0] = "1:00";
	$misc_income[1][1] = "Birthday Party";
	$misc_income[1][2] = "20.00";

	$inv_transfer[0][0] = "qty";
	$inv_transfer[0][1] = "name";
	$inv_transfer[0][2] = "from";
	$inv_transfer[0][3] = "to";

	$inv_transfer[1][0] = "100";
	$inv_transfer[1][1] = "Buns";
	$inv_transfer[1][2] = "1121";
	$inv_transfer[1][3] = "4452";

	$inv_transfer[2][0] = "400";
	$inv_transfer[2][1] = "Pattys";
	$inv_transfer[2][2] = "1121";
	$inv_transfer[2][3] = "4499";


	$inventory[0][0] = "num";
	$inventory[0][1] = "name";
	$inventory[0][2] = "amount";	
	$inventory[0][3] = "unit";
	$inventory[0][4] = "open";
	$inventory[0][5] = "usage";
	$inventory[0][6] = "received";
	$inventory[0][7] = "transfer_in";
	$inventory[0][8] = "transfer_out";
	$inventory[0][9] = "waste";
	$inventory[0][10] = "pysical_count";

	$inventory[1][0] = "1";
	$inventory[1][1] = "BUN, REG/PREMIUM";
	$inventory[1][2] = ".50";	
	$inventory[1][3] = "DOZEN";
	$inventory[1][4] = "176";
	$inventory[1][5] = "0";
	$inventory[1][6] = "0";
	$inventory[1][7] = "0";
	$inventory[1][8] = "0";
	$inventory[1][9] = "0";
	$inventory[1][10] = "0";

	$inventory[2][0] = "2";
	$inventory[2][1] = "BUN, KAISER";
	$inventory[2][2] = ".70";	
	$inventory[2][3] = "DOZEN";
	$inventory[2][4] = "176";
	$inventory[2][5] = "0";
	$inventory[2][6] = "0";
	$inventory[2][7] = "0";
	$inventory[2][8] = "0";
	$inventory[2][9] = "0";
	$inventory[2][10] = "0";

	$sillypants = 5;

	$rlib = rlib_init();
	rlib_add_datasource_array($rlib, "local_array");
	rlib_add_query_as($rlib, "local_array", "yields", "yields");
	rlib_add_query_as($rlib, "local_array", "coupons", "coupons");
	rlib_add_query_as($rlib, "local_array", "deposits", "deposits");
	rlib_add_query_as($rlib, "local_array", "petty_cash", "petty_cash");
	rlib_add_query_as($rlib, "local_array", "misc_income", "misc_income");
	rlib_add_query_as($rlib, "local_array", "inv_transfer", "inv_transfer");
	rlib_add_query_as($rlib, "local_array", "inventory", "inventory");
	rlib_add_report($rlib, "fixed_part.xml");
	rlib_set_output_format_from_text($rlib, "xml");
	rlib_signal_connect($rlib, "part_iteration", "callback");
	rlib_execute($rlib);
	header(rlib_get_content_type($rlib));
	rlib_spool($rlib);
	rlib_free($rlib);
?>
