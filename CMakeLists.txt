cmake_minimum_required(VERSION 3.14)

# Prefer clang/clang++ if available and no compiler explicitly set
if(NOT CMAKE_C_COMPILER AND NOT CMAKE_CXX_COMPILER)
    find_program(CLANG_BIN clang)
    find_program(CLANGXX_BIN clang++)
    if(CLANG_BIN AND CLANGXX_BIN)
        message(STATUS "Preferring clang: ${CLANG_BIN}, ${CLANGXX_BIN}")
        set(CMAKE_C_COMPILER "${CLANG_BIN}" CACHE STRING "C compiler" FORCE)
        set(CMAKE_CXX_COMPILER "${CLANGXX_BIN}" CACHE STRING "C++ compiler" FORCE)
    endif()
endif()

project(icb C CXX)

# Enable testing
enable_testing()

# Fetch GoogleTest
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(TCL COMPONENTS Tcl)
if(NOT TCL_INCLUDE_PATH OR NOT TCL_LIBRARY)
	message(FATAL_ERROR "TCL not found. Please install TCL and its development headers.")
endif()

# Find Readline
find_path(READLINE_INCLUDE_PATH readline/readline.h)
find_library(READLINE_LIBRARY readline)
if(NOT READLINE_INCLUDE_PATH OR NOT READLINE_LIBRARY)
	message(FATAL_ERROR "readline not found. Please install readline and its development headers.")
endif()

set(DEFAULT_PORT 7326 CACHE STRING "Default port")
set(DEFAULT_HOST "default.icb.net" CACHE STRING "Default server host")

option(ENABLE_SSL "Enable SSL support" OFF)
find_package(OpenSSL)

if(ENABLE_SSL)
    if(OPENSSL_FOUND)
        message(STATUS "OpenSSL found")
        message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
        message(STATUS "OpenSSL SSL library: ${OPENSSL_SSL_LIBRARY}")
        message(STATUS "OpenSSL Crypto library: ${OPENSSL_CRYPTO_LIBRARY}")
    else()
        message(WARNING "OpenSSL not found, disabling SSL support")
        set(ENABLE_SSL OFF)
    endif()
else()
    message(STATUS "SSL support disabled via ENABLE_SSL=OFF")
endif()

# Sanitizers (enabled by default for Debug builds)
option(ENABLE_SANITIZERS "Enable sanitizers" ON)
set(SANITIZERS "address,undefined" CACHE STRING "List of sanitizers to enable")
if(ENABLE_SANITIZERS AND NOT MSVC)
    if (CMAKE_BUILD_TYPE STREQUAL "" OR CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
        add_compile_options(-fno-omit-frame-pointer -g)
        add_link_options(-fno-omit-frame-pointer)
        if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
            add_compile_options("-fsanitize=${SANITIZERS}")
            add_link_options("-fsanitize=${SANITIZERS}")
        endif()
    endif()
endif()

# Define the custom command to generate the C file from the text file
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tcl_init.string
    COMMAND /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/scripts/tcl2string <
            ${CMAKE_CURRENT_SOURCE_DIR}/tcl-scripts/tcl_init.tcl >
	    ${CMAKE_CURRENT_BINARY_DIR}/tcl_init.string
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tcl-scripts/tcl_init.tcl
    		${CMAKE_CURRENT_SOURCE_DIR}/scripts/tcl2string
    COMMENT "Generating tcl_init.string from tcl_init.tcl"
)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tcl_connected.string
    COMMAND /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/scripts/tcl2string <
            ${CMAKE_CURRENT_SOURCE_DIR}/tcl-scripts/tcl_connected.tcl >
	    ${CMAKE_CURRENT_BINARY_DIR}/tcl_connected.string
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tcl-scripts/tcl_connected.tcl
		${CMAKE_CURRENT_SOURCE_DIR}/scripts/tcl2string
    COMMENT "Generating tcl_connected.string from tcl_connected.tcl"
)

include_directories(${TCL_INCLUDE_PATH}
	${READLINE_INCLUDE_PATH}
	${CMAKE_CURRENT_BINARY_DIR}
)

# List all C source files explicitly
set(ICB_SOURCES
	src/c_color.c
	src/c_hclear.c
	src/c_hdel.c
	src/c_help.c
	src/c_hush.c
	src/c_log.c
	src/c_quit.c
	src/c_rand.c
	src/c_replay.c
	src/c_shell.c
	src/c_time.c
	src/c_usage.c
	src/c_version.c
	src/cute.c
	src/dispatch.c
	src/getline.c
	src/getswitch.c
	src/globals.c
	src/helpdata.c
	src/history.c
	src/main.c
	src/msgs.c
	src/oset.c
	src/parse.c
	src/print.c
	src/readlineinit.c
	src/s_admin.c
	src/s_beep.c
	src/s_chpw.c
	src/s_group.c
	src/s_motd.c
	src/s_open.c
	src/s_person.c
	src/s_register.c
	src/s_send.c
	src/s_user.c
	src/s_version.c
	src/s_who.c
	src/send.c
	src/server.c
	src/signals.c
	src/strccmp.c
	src/strings.c
	src/strlist.c
	src/tcl.c
	src/unix.c
	src/usage.c
)

add_executable(icb
	${ICB_SOURCES}
	${CMAKE_CURRENT_BINARY_DIR}/tcl_init.string
	${CMAKE_CURRENT_BINARY_DIR}/tcl_connected.string
)

target_include_directories(icb PRIVATE src)

if(ENABLE_SSL AND OPENSSL_FOUND)
	set(DEFAULT_SSL_PORT 7326 CACHE STRING "Default SSL port")
	target_include_directories(icb PRIVATE ${OPENSSL_INCLUDE_DIR})
	target_link_libraries(icb PRIVATE ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
	target_compile_definitions(icb PRIVATE HAVE_OPENSSL)
endif()

target_compile_definitions(icb PRIVATE
		DEFAULT_PORT=${DEFAULT_PORT} 
		DEFAULT_HOST=\"${DEFAULT_HOST}\"
)

target_link_libraries(icb ${TCL_LIBRARY} ${READLINE_LIBRARY})

#find_library(HISTORY_LIBRARY history)
#if(HISTORY_LIBRARY)
#    target_link_libraries(icb ${HISTORY_LIBRARY})
#endif()

# Install rule
install(TARGETS icb RUNTIME DESTINATION bin)

# Unit tests
add_subdirectory(tests)
