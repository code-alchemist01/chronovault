# CMake generated Testfile for 
# Source directory: C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/tests
# Build directory: C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/build/tests/tcfs_tests[1]_include.cmake")
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[tcfs_unit_tests]=] "C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/build/tests/Debug/tcfs_tests.exe")
  set_tests_properties([=[tcfs_unit_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/tests/CMakeLists.txt;59;add_test;C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[tcfs_unit_tests]=] "C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/build/tests/Release/tcfs_tests.exe")
  set_tests_properties([=[tcfs_unit_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/tests/CMakeLists.txt;59;add_test;C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[tcfs_unit_tests]=] "C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/build/tests/MinSizeRel/tcfs_tests.exe")
  set_tests_properties([=[tcfs_unit_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/tests/CMakeLists.txt;59;add_test;C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[tcfs_unit_tests]=] "C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/build/tests/RelWithDebInfo/tcfs_tests.exe")
  set_tests_properties([=[tcfs_unit_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/tests/CMakeLists.txt;59;add_test;C:/Users/excalibur/Desktop/Projeler/Rust/tcfs/tests/CMakeLists.txt;0;")
else()
  add_test([=[tcfs_unit_tests]=] NOT_AVAILABLE)
endif()
