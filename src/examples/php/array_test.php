<? dl ("librlib.so");
	$data[0][0] = "first_name";
	$data[0][1] = "last_name";
	$data[0][2] = "color";
	$data[0][3] = "group";

	$data[1][0] = "Bob";
	$data[1][1] = "Doan";
	$data[1][2] = "blue";
	$data[1][3] = "1";

	$data[2][0] = "Eric";
	$data[2][1] = "Eburuschkin";
	$data[2][2] = "green";
	$data[2][3] = "1";

	$data[3][0] = "Mike";
	$data[3][1] = "Roth";
	$data[3][2] = "yellow";
	$data[3][3] = "2";

	$data[4][0] = "Bob";
	$data[4][1] = "Kratz";
	$data[4][2] = "pink";
	$data[4][3] = "2";

	$data[5][0] = "Steve";
	$data[5][1] = "Tilden";
	$data[5][2] = "purple";
	$data[5][3] = "2";

	$rlib =	rlib_init();
	rlib_add_datasource_array($rlib, "local_array");
	rlib_add_query_as($rlib, "local_array", "data", "data");
	rlib_add_report($rlib, "array_test.xml");
	rlib_set_output_format_from_text($rlib, "html");
	rlib_execute($rlib);
	header(rlib_get_content_type($rlib));
	rlib_spool($rlib);
	rlib_free($rlib);
?>
