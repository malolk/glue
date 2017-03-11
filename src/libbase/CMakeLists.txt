file(GLOB libbase_SRCS "*.cc")

add_library(libbase ${libbase_SRCS})
target_link_libraries(libbase pthread rt)
set_target_properties(libbase PROPERTIES COMPILE_FLAGS "-std=c++0x")

if(CMAKE_BUILD_INSTALL)
    install(TARGETS libbase DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
    file(GLOB libbase_HEADERS "*.h")
    install(FILES ${libbase_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libbase)
endif()

if(NOT CMAKE_BUILD_NO_TEST)
	add_subdirectory(tests)
endif()
