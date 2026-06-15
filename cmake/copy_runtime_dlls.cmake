if(NOT DEFINED INPUT_DIR OR NOT DEFINED OUTPUT_DIR)
    message(FATAL_ERROR "INPUT_DIR and OUTPUT_DIR are required.")
endif()

file(GLOB runtime_dlls "${INPUT_DIR}/*.dll")
foreach(runtime_dll IN LISTS runtime_dlls)
    file(COPY "${runtime_dll}" DESTINATION "${OUTPUT_DIR}")
endforeach()
