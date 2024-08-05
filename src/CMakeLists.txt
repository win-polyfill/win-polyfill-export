# Copyright 2024 Yonggang Luo
# SPDX-License-Identifier: MIT

add_executable(win-polyfill-export-gen-sa suffix-array-gen.cpp)
set_target_properties(win-polyfill-export-gen-sa PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/"
)
# target_link_libraries(win-polyfill-export-gen-sa PRIVATE onecore_apiset.lib)
# target_link_options(win-polyfill-export-gen-sa PRIVATE /NODEFAULTLIB:kernel32.lib)
target_link_libraries(win-polyfill-export-gen-sa PRIVATE PathCch.lib)
set_property(TARGET win-polyfill-export-gen-sa PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

add_library(fake SHARED fake.cpp)
target_link_options(fake PRIVATE /DEF:${CMAKE_CURRENT_LIST_DIR}/api-ms-win-core-path-l1-1-0_fake.def)

add_executable(win-polyfill-link-fake-dll fake-linker.cpp)
set_target_properties(win-polyfill-link-fake-dll PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/"
)
target_link_libraries(win-polyfill-link-fake-dll PRIVATE fake)

add_executable(one-core-linker one-core-linker.cpp)
target_link_options(one-core-linker PRIVATE
  /NODEFAULTLIB:kernel32.lib
  onecoreuap_apiset.lib
)

add_executable(gen-exports
  gen-exports.cpp
  gen-exports-api.cpp
  gen-exports-function-info.cpp
)

target_link_libraries(gen-exports win-polyfill-deps)
target_compile_options(gen-exports PRIVATE -Oi- -bigobj)
