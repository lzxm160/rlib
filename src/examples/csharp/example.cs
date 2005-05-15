public class Hello1
{
   public static void Main()
   {
		SWIGTYPE_p_rlib r = rlib.rlib_init();
		rlib.rlib_add_datasource_xml(r, "local_xml");
		rlib.rlib_add_query_as(r, "local_xml", "data.xml", "data");
		rlib.rlib_add_report(r, "array.xml");
		rlib.rlib_set_output_format_from_text(r, "pdf");
		rlib.rlib_execute(r);
		rlib.rlib_spool(r);
		rlib.rlib_free(r);
   }
}
