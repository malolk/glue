file(GLOB network_SRCS "*.cpp")

add_library(glue_network ${network_SRCS})
target_link_libraries(glue_network glue_libbase pthread )
set_target_properties(glue_network PROPERTIES COMPILE_FLAGS "-std=c++0x")

install(TARGETS glue_network DESTINATION ${PROJECT_SOURCE_DIR}/lib)

file(GLOB network_HEADERS "*.h")
install(FILES ${network_HEADERS} DESTINATION ${PROJECT_SOURCE_DIR}/include/network)

if(NOT CMAKE_BUILD_NO_TEST)
	add_subdirectory(tests)
endif()
