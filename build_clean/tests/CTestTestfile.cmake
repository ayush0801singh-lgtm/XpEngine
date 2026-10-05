# CMake generated Testfile for 
# Source directory: D:/Programming/New C++ Project/expression-engine/tests
# Build directory: D:/Programming/New C++ Project/expression-engine/build_clean/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("D:/Programming/New C++ Project/expression-engine/build_clean/tests/expression_tests_74785a0e_include.cmake")
add_test(e2e_tests "C:/Program Files/Python313/python.exe" "D:/Programming/New C++ Project/expression-engine/tests/run_e2e_tests.py" "D:/Programming/New C++ Project/expression-engine/build_clean/app/expression_engine.exe")
set_tests_properties(e2e_tests PROPERTIES  _BACKTRACE_TRIPLES "D:/Programming/New C++ Project/expression-engine/tests/CMakeLists.txt;44;add_test;D:/Programming/New C++ Project/expression-engine/tests/CMakeLists.txt;0;")
subdirs("../_deps/googletest-build")
