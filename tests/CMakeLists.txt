add_executable(ordered_binary_tree_test
    "${CMAKE_CURRENT_SOURCE_DIR}/ordered_binary_tree_test.cpp"
)

target_link_libraries(ordered_binary_tree_test PRIVATE
    Catch2::Catch2WithMain
    ordered_binary_trees
)

catch_discover_tests(ordered_binary_tree_test)
