file(GLOB libbase_SRCS "*.cpp")

add_library(glue_libbase ${libbase_SRCS})
target_link_libraries(glue_libbase pthread rt)
set_target_properties(glue_libbase PROPERTIES COMPILE_FLAGS "-std=c++0x")

install(TARGETS glue_libbase DESTINATION lib)

file(GLOB libbase_HEADERS "*.h")
install(FILES ${libbase_HEADERS} DESTINATION include/libbase)

if(NOT CMAKE_BUILD_NO_TEST)
	add_subdirectory(tests)
endif()
