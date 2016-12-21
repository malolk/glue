file(GLOB httpd_SRCS "*.cc")

add_library(glue_httpd ${httpd_SRCS})
target_link_libraries(glue_httpd glue_network)
set_target_properties(glue_httpd PROPERTIES COMPILE_FLAGS "-std=c++0x")

install(TARGETS glue_httpd DESTINATION lib)

file(GLOB http_HEADERS "*.h")
install(FILES ${http_HEADERS} DESTINATION include/httpd)

if(NOT CMAKE_BUILD_NO_EXAMPLES)
	install(DIRECTORY docs DESTINATION ${EXECUTABLE_OUTPUT_PATH})
	add_subdirectory(tests)
endif()
