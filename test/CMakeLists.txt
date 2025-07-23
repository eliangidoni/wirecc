include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/test")
include_directories("${PROJECT_BINARY_DIR}/include")

FILE(GLOB_RECURSE wirecc_TEST_SOURCESCPP ${PROJECT_SOURCE_DIR}/test/*_test.cpp)
set(wirecc_TEST_SOURCES ${wirecc_TEST_SOURCESCPP})

foreach(testsource ${wirecc_TEST_SOURCES})
  get_filename_component(name ${testsource} NAME_WE)
  add_executable(${name} ${testsource} ${PROJECT_SOURCE_DIR}/test/wirecc.cpp)
  add_test(${name} ${name})
endforeach(testsource)

find_program(LCOV_PATH lcov)
find_program(GENHTML_PATH genhtml)
if(LCOV_PATH AND GENHTML_PATH)
  add_test(lcov ${LCOV_PATH} --capture --directory ${PROJECT_SOURCE_DIR} --output-file cov.info)
  add_test(genhtml ${GENHTML_PATH} cov.info --output-directory html-coverage)
endif(LCOV_PATH AND GENHTML_PATH)
