class Example {
	static {
		System.loadLibrary("rlibjava");
	}
	
	public static void main(String[] args) {
		SWIGTYPE_p_rlib rlib;
		String hostname = "localhost";
		String username = "rlib";
		String password = "rlib";
		String database = "rlib";	

		rlib = librlib.rlib_init();
		librlib.rlib_add_datasource_mysql(rlib, "local_mysql", hostname, username, password, database);
		librlib.rlib_add_query_as(rlib, "local_mysql", "select * FROM products", "products");
		librlib.rlib_add_report(rlib, "products.xml");
		librlib.rlib_set_output_format_from_text(rlib, "pdf");
		librlib.rlib_execute(rlib);
		librlib.rlib_spool(rlib);
		librlib.rlib_free(rlib);				
	}
}
