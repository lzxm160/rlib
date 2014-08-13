<?php
	$data[0][0] = "first_name";
	$data[0][1] = "last_name";
	$data[0][2] = "color";
	$data[0][3] = "group";
	$data[0][4] = "breakfast";

	$data[1][0] = "";
	$data[1][1] = "Doan";
	$data[1][2] = "blue";
	$data[1][3] = "1";
	$data[1][4] = "Green Eggs And Spam I Am I Am";

	$rlib =	rlib_init();
	rlib_version();
	rlib_add_datasource_array($rlib, "local_array");
	rlib_add_query_as($rlib, "local_array", "data", "data");
	rlib_add_report($rlib, "array.xml");
	rlib_add_function($rlib, "bobdoan", "bobdoan", 1);
	rlib_add_function($rlib, "mikeroth", "mike_roth", 2);
	rlib_set_output_format_from_text($rlib, "json");
	rlib_set_output_parameter($rlib, "debugging", "yes");
	rlib_set_output_parameter($rlib, "only_quote_strings", "yes");
	rlib_set_locale($rlib, "en_US");
	rlib_execute($rlib);
	//header(rlib_get_content_type($rlib));
	rlib_spool($rlib);
	rlib_free($rlib);
	
	function mike_roth($a, $b) {
		return "pancakes are yummier then $a and $b";
	}

	function bobdoan($a) {
		return strtoupper($a);
	}
	
?>
