file(GLOB httpd_SRCS "*.cc")

add_library(httpd ${httpd_SRCS})
target_link_libraries(httpd network)
set_target_properties(httpd PROPERTIES COMPILE_FLAGS "-std=c++0x")

if(CMAKE_BUILD_INSTALL)
    install(TARGETS httpd DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
    file(GLOB http_HEADERS "*.h")
    install(FILES ${http_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/httpd)
endif()

if(NOT CMAKE_BUILD_NO_EXAMPLES)
	install(DIRECTORY docs DESTINATION ${EXECUTABLE_OUTPUT_PATH})
	add_subdirectory(tests)
endif()
