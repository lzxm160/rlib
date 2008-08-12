class Example {
	static {
		System.loadLibrary("rlibjava");
	}
	
	public static void main(String[] args) {
		SWIGTYPE_p_rlib rlib1;
		String hostname = "localhost";
		String username = "rlib";
		String password = "rlib";
		String database = "rlib";

		rlib1 = rlib.rlib_init();
		rlib.rlib_add_datasource_mysql(rlib1, "local_mysql", hostname, username, password, database);
		rlib.rlib_add_query_as(rlib1, "local_mysql", "select * FROM products", "products");
		rlib.rlib_add_report(rlib1, "products.xml");
		rlib.rlib_set_output_format_from_text(rlib1, "pdf");
		rlib.rlib_execute(rlib1);
		rlib.rlib_spool(rlib1);
		rlib.rlib_free(rlib1);
	}
}
