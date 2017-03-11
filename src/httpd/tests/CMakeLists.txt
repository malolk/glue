add_executable(get_cgi_test get_cgi_test.cc)
target_link_libraries(get_cgi_test network libbase httpd)

add_executable(httpd_test httpd_test.cc)
target_link_libraries(httpd_test network libbase httpd)

