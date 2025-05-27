cmake_minimum_required(VERSION 3.10)
project(icb C)

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

if(ENABLE_SSL AND OPENSSL_FOUND)
	set(DEFAULT_SSL_PORT 7326 CACHE STRING "Default SSL port")
	target_include_directories(icb PRIVATE ${OPENSSL_INCLUDE_DIR})
	target_link_libraries(icb PRIVATE ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
	target_compile_definitions(icb PRIVATE HAVE_OPENSSL)
endif()

# Define the custom command to generate the C file from the text file
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tcl_init.string
    COMMAND /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/scripts/tcl2string <
            ${CMAKE_CURRENT_SOURCE_DIR}/icb/tcl_init.tcl >
	    ${CMAKE_CURRENT_BINARY_DIR}/tcl_init.string
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/icb/tcl_init.tcl
    		${CMAKE_CURRENT_SOURCE_DIR}/scripts/tcl2string
    COMMENT "Generating tcl_init.string from tcl_init.tcl"
)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tcl_connected.string
    COMMAND /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/scripts/tcl2string <
            ${CMAKE_CURRENT_SOURCE_DIR}/icb/tcl_connected.tcl >
	    ${CMAKE_CURRENT_BINARY_DIR}/tcl_connected.string
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/icb/tcl_connected.tcl
		${CMAKE_CURRENT_SOURCE_DIR}/scripts/tcl2string
    COMMENT "Generating tcl_connected.string from tcl_connected.tcl"
)

include_directories(${TCL_INCLUDE_PATH}
	${READLINE_INCLUDE_PATH}
	${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB SOURCES icb/*.c)

add_executable(icb
	${SOURCES}
	${CMAKE_CURRENT_BINARY_DIR}/tcl_init.string
	${CMAKE_CURRENT_BINARY_DIR}/tcl_connected.string
)

target_include_directories(icb PRIVATE src)

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

