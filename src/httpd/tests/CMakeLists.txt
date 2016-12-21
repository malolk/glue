add_executable(get_cgi_test get_cgi_test.cc)
target_link_libraries(get_cgi_test glue_network glue_libbase glue_httpd)

add_executable(httpd_test httpd_test.cc)
target_link_libraries(httpd_test glue_network glue_libbase glue_httpd)

