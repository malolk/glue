add_executable(TestGetCGI TestGetCGI.cpp)
target_link_libraries(TestGetCGI glue_network glue_libbase)

add_executable(TestHttpd TestHttpd.cpp)
target_link_libraries(TestHttpd glue_network glue_libbase glue_httpd)

