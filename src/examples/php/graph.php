<? dl ("librlib.so");
	$data[0][0] = "name";
	$data[0][1] = "ty_sales";
	$data[0][2] = "ly_sales";
	$data[0][3] = "projection";
	$data[0][4] = "sold";
	$data[0][5] = "negative";

	$data[1][0] = "BK 1";
	$data[1][1] = "3000";
	$data[1][2] = "3300";
	$data[1][3] = "3300";
	$data[1][4] = "-.5";
	$data[1][5] = "-2";

	$data[2][0] = "BK 2";
	$data[2][1] = "2400";
	$data[2][2] = "2100";
	$data[2][3] = "3301";
	$data[2][4] = "1";
	$data[2][5] = "-4";

	$data[3][0] = "BK 3";
	$data[3][1] = "4000";
	$data[3][2] = "4100";
	$data[3][3] = "3302";
	$data[3][4] = "2";
	$data[3][5] = "-6";

	$data[4][0] = "BK 4";
	$data[4][1] = "4700";
	$data[4][2] = "4100";
	$data[4][3] = "3303";
	$data[4][4] = "3";
	$data[4][5] = "-8";

	$data[5][0] = "BK 5";
	$data[5][1] = "2500";
	$data[5][2] = "5500";
	$data[5][3] = "3304";
	$data[5][4] = "12.5";
	$data[5][5] = "-10";

	$rlib =	rlib_init();
	rlib_add_datasource_array($rlib, "local_array");
	rlib_add_query_as($rlib, "local_array", "data", "data");
	rlib_add_report($rlib, "graph.xml");
	rlib_set_output_format_from_text($rlib, "pdf");
	rlib_execute($rlib);
	header(rlib_get_content_type($rlib));
	rlib_spool($rlib);
	rlib_free($rlib);
?>
