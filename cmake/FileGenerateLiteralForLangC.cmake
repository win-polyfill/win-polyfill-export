# Usage:
# cmake -DOUTPUT_VARIABLE_NAME=test -DINPUT_FILE_NAME=input.dll -DOUTPUT_FILE_NAME=input.dll.c -P GenerateLiteralForLangC.cmake
cmake_policy(SET CMP0007 NEW)

# reads source file contents as hex string
file(READ ${INPUT_FILE_NAME} InputFileBinaryString HEX)

macro(adopt_result)
  list(LENGTH result result_len)
  if (${result_len} GREATER ${ARGV0})
    list(JOIN result ", " result)
    list(APPEND all_result "${result},")
    set(result "")
  endif()
endmacro()

function(FileGenerateLiteralForLangC outVar text)
  string(LENGTH ${text} hexLen)
  math(EXPR hexLen "${hexLen} - 1")
  set(all_result "#include <stdint.h>")
  list(APPEND all_result "const unsigned char ${OUTPUT_VARIABLE_NAME}[]=")
  list(APPEND all_result "{")
  set(result "")
  foreach(idx RANGE 0 ${hexLen} 2)
    # Convert a single char to hex
    string(SUBSTRING ${text} ${idx} 2 char)
    list(APPEND result "0x${char}")
    adopt_result(15)
  endforeach()
  adopt_result(0)
  list(JOIN all_result "\n" all_result_output)
  set(final_part "\n}\;\n\nconst size_t ${OUTPUT_VARIABLE_NAME}_size=sizeof(${OUTPUT_VARIABLE_NAME})\;\n")
  set(${outVar} "${all_result_output}${final_part}" PARENT_SCOPE)
endfunction()

FileGenerateLiteralForLangC(RESULT_CONTENT ${InputFileBinaryString})
file(WRITE ${OUTPUT_FILE_NAME} ${RESULT_CONTENT})
