add_library(MSUnitTestFramework INTERFACE)
if (MSVC_VERSION VERSION_GREATER 1900)
  set_target_properties(MSUnitTestFramework PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "$ENV{VCInstallDir}Auxiliary/VS/UnitTest/include"
  )
  target_link_directories(MSUnitTestFramework INTERFACE "$ENV{VCInstallDir}Auxiliary/VS/UnitTest/lib/")
else()
  set_target_properties(MSUnitTestFramework PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "$ENV{VCInstallDir}UnitTest/include"
  )
  target_link_directories(MSUnitTestFramework INTERFACE "$ENV{VCInstallDir}UnitTest/lib/")
endif()
