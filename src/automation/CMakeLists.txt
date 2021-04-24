get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(INCLUDES_LIST ${PROJECT_SOURCE_DIR}/include)
aux_source_directory(./ SRC)

add_library(${MODULE_NAME} STATIC ${SRC} ${INCLUDES_LIST})
target_include_directories(${MODULE_NAME} PUBLIC ${INCLUDES_LIST})
target_compile_features(${MODULE_NAME} PUBLIC cxx_std_11)