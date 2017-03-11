file(GLOB network_SRCS "*.cc")

add_library(network ${network_SRCS})
target_link_libraries(network libbase pthread )
set_target_properties(network PROPERTIES COMPILE_FLAGS "-std=c++0x")

if(CMAKE_BUILD_INSTALL)
    install(TARGETS network DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
    file(GLOB network_HEADERS "*.h")
    install(FILES ${network_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/network)
endif()

if(NOT CMAKE_BUILD_NO_TEST)
	add_subdirectory(tests)
endif()
