set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
aux_source_directory(. DIR_LIB)
add_library(httpserver_lib ${DIR_LIB})
