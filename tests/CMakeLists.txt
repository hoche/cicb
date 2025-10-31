# Test sources - compile only what we need
# Create a minimal strings.c for testing that doesn't depend on globals
set(SAFE_STRINGS_SRC ${CMAKE_SOURCE_DIR}/tests/strings_test_src.c)

# String utilities test - minimal version
add_executable(strings_test strings_test.cpp ${SAFE_STRINGS_SRC})
target_include_directories(strings_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(strings_test PRIVATE
    gtest
    gtest_main
)

add_test(NAME StringsTest COMMAND strings_test)

# Safe ATOI test - minimal version
add_executable(safe_atoi_test safe_atoi_test.cpp ${SAFE_STRINGS_SRC})
target_include_directories(safe_atoi_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(safe_atoi_test PRIVATE gtest gtest_main)
add_test(NAME SafeAtoiTest COMMAND safe_atoi_test)

# String list test - needs globals.c for STRLIST definition
set(STRLIST_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/strlist.c
    ${CMAKE_SOURCE_DIR}/src/globals.c
)

add_executable(strlist_test strlist_test.cpp ${STRLIST_TEST_SOURCES})
target_include_directories(strlist_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${TCL_INCLUDE_PATH}
)
target_link_libraries(strlist_test PRIVATE gtest gtest_main)
add_test(NAME StrlistTest COMMAND strlist_test)
