add_executable(logger_test logger_test.cc)
target_link_libraries(logger_test libbase)

add_executable(timeutil_test timeutil_test.cc)
target_link_libraries(timeutil_test libbase)

add_executable(bounded_blocking_queue_test bounded_blocking_queue_test.cc)
target_link_libraries(bounded_blocking_queue_test libbase)

add_executable(heap_test heap_test.cc)
target_link_libraries(heap_test libbase)

add_executable(thread_test thread_test.cc)
target_link_libraries(thread_test libbase)

add_executable(threadpool_test threadpool_test.cc)
target_link_libraries(threadpool_test libbase)

add_executable(waitmember_test waitmember_test.cc)
target_link_libraries(waitmember_test libbase)

add_executable(buffer_test buffer_test.cc)
target_link_libraries(buffer_test libbase)
