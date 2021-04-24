add_executable(tabbed tabbed.cpp)
target_compile_features(tabbed PRIVATE cxx_std_17)

target_link_libraries(tabbed PRIVATE autocomplete)
target_link_libraries(tabbed PRIVATE gui)
target_link_libraries(tabbed PRIVATE automation)
