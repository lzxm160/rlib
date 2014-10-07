<?php 
	$rlib =	rlib_init();
	rlib_add_datasource_csv($rlib, "local_csv");
	rlib_add_query_as($rlib, "local_csv", "data.csv", "data");
	rlib_add_report($rlib, "csv.xml");
	rlib_set_output_format_from_text($rlib, "json");
	rlib_set_output_parameter($rlib, "debugging", "yes");
	rlib_execute($rlib);
	header(rlib_get_content_type($rlib));
	rlib_spool($rlib);
	rlib_free($rlib);	
?>
