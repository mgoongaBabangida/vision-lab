# OpenCV's official Windows package also has a dynamically loaded video decoder.
foreach(runtime IN LISTS RUNTIME_FILES)
    if(EXISTS "${runtime}")
        file(COPY "${runtime}" DESTINATION "${DESTINATION}")
    endif()
endforeach()
