<?php 
	$data[0][0] = "first_name";
	$data[0][1] = "last_name";
	$data[0][2] = "color";
	$data[0][3] = "group";

	$data[1][0] = "Bob";
	$data[1][1] = "Doan";
	$data[1][2] = "blue";
	$data[1][3] = "1";

	$data[2][0] = "Eric";
	$data[2][1] = "E&buruschkin";
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

	$data[6][0] = "Joe";
	$data[6][1] = "Joe";
	$data[6][2] = "purple";
	$data[6][3] = "2";

	$data[7][0] = "Jim";
	$data[7][1] = "Jim";
	$data[7][2] = "purple";
	$data[7][3] = "2";

	$data[8][0] = "Greg";
	$data[8][1] = "Greg";
	$data[8][2] = "purple";
	$data[8][3] = "2";

	$data[9][0] = "Bill";
	$data[9][1] = "Bill";
	$data[9][2] = "purple";
	$data[9][3] = "2";

	$data[10][0] = "Todd";
	$data[10][1] = "Todd";
	$data[10][2] = "purple";
	$data[10][3] = "2";

	$data[11][0] = "Jennifer";
	$data[11][1] = "Jennifer";
	$data[11][2] = "purple";
	$data[11][3] = "2";

	$data[12][0] = "Lauren";
	$data[12][1] = "Lauren";
	$data[12][2] = "purple";
	$data[12][3] = "2";

	$data[13][0] = "Logan";
	$data[13][1] = "Logan";
	$data[13][2] = "purple";
	$data[13][3] = "2";

	$data[14][0] = "Eggs";
	$data[14][1] = "Eggs";
	$data[14][2] = "purple";
	$data[14][3] = "2";

	$data[15][0] = "Bacon";
	$data[15][1] = "Bacon";
	$data[15][2] = "purple";
	$data[15][3] = "2";

	$rlib =	rlib_init();
	rlib_add_datasource_array($rlib, "local_array");
	rlib_add_query_as($rlib, "local_array", "data", "data");
	rlib_add_report($rlib, "pages_across.xml");
//	rlib_add_report($rlib, "x.xml");
	rlib_set_output_format_from_text($rlib, "html");
	rlib_execute($rlib);
	header(rlib_get_content_type($rlib));
	rlib_spool($rlib);
	rlib_free($rlib);
?>
