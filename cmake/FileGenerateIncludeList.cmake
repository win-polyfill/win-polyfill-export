# cmake -DINPUT_LIST_DIRECTORY=test -DOUTPUT_FILE_NAME=win-polyfill-gen-list.h -P FileGenerateIncludeList.cmake

file(GLOB sources_list LIST_DIRECTORIES true ${INPUT_LIST_DIRECTORY}/**/*)

set(all_result "")

foreach(dir_or_file ${sources_list})
  if(IS_DIRECTORY ${dir_or_file})
    continue()
  else()
    file(RELATIVE_PATH filename ${INPUT_LIST_DIRECTORY} ${dir_or_file})
    if(${filename} MATCHES ".*\\.hpp")
      list(APPEND all_result "#include <${filename}>")
    endif()
  endif()
endforeach()

list(JOIN all_result "\n" all_result_output)

file(WRITE ${OUTPUT_FILE_NAME} ${all_result_output})
