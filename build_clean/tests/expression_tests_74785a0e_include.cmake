if(EXISTS "D:/Programming/New C++ Project/expression-engine/build_clean/tests/expression_tests.exe")
  discover_tests(
    COMMAND "C:/msys64/mingw64/bin/cmake.exe"
      -D [[TEST_EXECUTABLE=D:/Programming/New C++ Project/expression-engine/build_clean/tests/expression_tests.exe]]
      -D [[TEST_EXECUTOR=]]
    DISCOVERY_ARGS
      -D [[TEST_FILTER=]]
      -D [[TEST_DISCOVERY_EXTRA_ARGS=]]
      -D [[NO_PRETTY_TYPES=FALSE]]
      -D [[NO_PRETTY_VALUES=FALSE]]
      -P [[C:/msys64/mingw64/share/cmake/Modules/GoogleTest/DiscoverTests.cmake]]
    DISCOVERY_MATCH
      "-- ([^#]+)#([^#]+)#DISABLED=([^#]+)#LOCATION=([^#]*)#"
    DISCOVERY_PROPERTIES
      TIMEOUT [[5]]
      WORKING_DIRECTORY [[D:/Programming/New C++ Project/expression-engine/build_clean/tests]]
    TEST_NAME [[\1]]
    TEST_ARGS
      -D [[TEST_FILTER=\2]]
      -D [[TEST_XML_OUTPUT=]]
      -D [[TEST_EXTRA_ARGS=]]
      -P [[C:/msys64/mingw64/share/cmake/Modules/GoogleTest/LaunchTest.cmake]]
    TEST_PROPERTIES
      DISABLED [[\3]]
      DEF_SOURCE_LINE [[\4]]
      SKIP_REGULAR_EXPRESSION "\\[  SKIPPED \\]"
      WORKING_DIRECTORY [[D:/Programming/New C++ Project/expression-engine/build_clean/tests]]
      
    TEST_LIST expression_tests_TESTS
  )
  list(TRANSFORM expression_tests_TESTS REPLACE ";" "\n")
  list(FILTER expression_tests_TESTS EXCLUDE REGEX "(\\[|])")
  list(TRANSFORM expression_tests_TESTS REPLACE "\n" [[\\;]])
else()
  add_test(expression_tests_NOT_BUILT expression_tests_NOT_BUILT)
endif()
