
import rlibcompat as rlib

myrlib = rlib.rlib_init();
rlib.rlib_add_datasource_mysql(myrlib, "local_mysql", "localhost", "rlib", "rlib", "rlib");
rlib.rlib_add_query_as(myrlib, "local_mysql", "select * FROM products", "products");
rlib.rlib_add_report(myrlib, "products.xml");
rlib.rlib_set_output_format_from_text(myrlib, "pdf");
rlib.rlib_execute(myrlib);
rlib.rlib_spool(myrlib);
rlib.rlib_free(myrlib);
