# Usage:
# cmake -DEXECUTABLE_PATH=some_exe.exe
# -DWORKING_DIRECTORY=$WORK_DIR
# -DOUTPUT_FILE_NAME=output.txt
# -DERROR_FILE_NAME=error.txt
# -DRESULT_FILE_NAME=result.txt
# -P FileExecuteCaptureOutput.cmake
cmake_policy(SET CMP0007 NEW)

if ("${WORKING_DIRECTORY}" STREQUAL "")
  set(WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

execute_process(
  COMMAND
    ${EXECUTABLE_PATH}
  WORKING_DIRECTORY
    ${WORKING_DIRECTORY}
  TIMEOUT 10000
  OUTPUT_VARIABLE EXECUTABLE_OUTPUT
  ERROR_VARIABLE EXECUTABLE_ERROR
  RESULT_VARIABLE EXECUTABLE_RESULT
)

if (NOT ${OUTPUT_FILE_NAME} STREQUAL "")
  file(WRITE ${OUTPUT_FILE_NAME} ${EXECUTABLE_OUTPUT})
endif()

if (NOT ${ERROR_FILE_NAME} STREQUAL "")
  file(WRITE ${ERROR_FILE_NAME} ${EXECUTABLE_ERROR})
endif()

if (NOT ${RESULT_FILE_NAME} STREQUAL "")
  file(WRITE ${RESULT_FILE_NAME} ${EXECUTABLE_RESULT})
endif()
