file(GLOB network_SRCS "*.cc")

add_library(glue_network ${network_SRCS})
target_link_libraries(glue_network glue_libbase pthread )
set_target_properties(glue_network PROPERTIES COMPILE_FLAGS "-std=c++0x")

install(TARGETS glue_network DESTINATION lib)

file(GLOB network_HEADERS "*.h")
install(FILES ${network_HEADERS} DESTINATION include/network)

if(NOT CMAKE_BUILD_NO_TEST)
	add_subdirectory(tests)
endif()
