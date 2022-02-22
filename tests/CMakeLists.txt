function(add_unit_test module_name)
  add_executable("${module_name}" ${ARGN})
  target_link_libraries("${module_name}" PRIVATE
    ordered_binary_trees
    Catch2::Catch2WithMain
  )
  target_compile_options("${module_name}" PRIVATE
    -g -O0 -Wall
  )
  catch_discover_tests("${module_name}")
endfunction()

function(add_benchmark_test module_name)
  add_executable("${module_name}" ${ARGN})
  target_link_libraries("${module_name}" PRIVATE
    ordered_binary_trees
    Catch2::Catch2WithMain
  )
  target_compile_options("${module_name}" PRIVATE
    -g -O3 -DNDEBUG -fomit-frame-pointer
  )
endfunction()

add_unit_test(ordered_binary_tree_test
  "${CMAKE_CURRENT_SOURCE_DIR}/ordered_binary_tree_test.cpp"
)

add_unit_test(managed_tree_test
  "${CMAKE_CURRENT_SOURCE_DIR}/managed_tree_test.cpp"
)

add_benchmark_test(managed_tree_benchmark
  "${CMAKE_CURRENT_SOURCE_DIR}/managed_tree_benchmark.cpp"
)
