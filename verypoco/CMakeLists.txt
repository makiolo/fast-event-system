cmake_minimum_required(VERSION 2.8)
project(Foundation)

set(LIBNAME "poco")
set(POCO_LIBNAME "very${LIBNAME}")

macro(POCO_SOURCES_PLAT out name platform)
    source_group("${name}\\Source Files" FILES ${ARGN})
    list(APPEND ${out} ${ARGN})
    if(NOT (${platform}))
        set_source_files_properties(${ARGN} PROPERTIES HEADER_FILE_ONLY TRUE)
    endif()
endmacro()

macro(POCO_SOURCES out name)
    POCO_SOURCES_PLAT( ${out} ${name} ON ${ARGN})
endmacro()

macro(POCO_SOURCES_AUTO out)
    POCO_SOURCES_AUTO_PLAT( ${out} ON ${ARGN})
endmacro()

macro(POCO_SOURCES_AUTO_PLAT out platform)
    foreach( f ${ARGN})
		
        get_filename_component(fname ${f} NAME)

        # Read the package name from the source file
        file(STRINGS ${f} package REGEX "// Package: (.*)")
        if(package)
            string(REGEX REPLACE ".*: (.*)" "\\1" name ${package})

            # Files of the Form X_UNIX.cpp are treated as headers
            if(${fname} MATCHES ".*_.*\\..*")
                #message(STATUS "Platform: ${name} ${f} ${platform}")
                POCO_SOURCES_PLAT( ${out} ${name} OFF ${f})
            else()
                #message(STATUS "Source: ${name} ${f} ${platform}")
                POCO_SOURCES_PLAT( ${out} ${name} ${platform} ${f})
            endif()
        else()
            #message(STATUS "Source: Unknown ${f} ${platform}")
            POCO_SOURCES_PLAT( ${out} Unknown ${platform} ${f})
        endif()
    endforeach()
endmacro()

macro(POCO_HEADERS_AUTO out)
    foreach( f ${ARGN})

        get_filename_component(fname ${f} NAME)

        # Read the package name from the source file
        file(STRINGS ${f} package REGEX "// Package: (.*)")
        if(package)
            string(REGEX REPLACE ".*: (.*)" "\\1" name ${package})
            #message(STATUS "Header: ${name} ${f}")
            POCO_HEADERS( ${out} ${name} ${f})
        else()
            #message(STATUS "Header: Unknown ${f}")
            POCO_HEADERS( ${out} Unknown ${f})
        endif()
    endforeach()
endmacro()

macro(POCO_HEADERS out name)
    set_source_files_properties(${ARGN} PROPERTIES HEADER_FILE_ONLY TRUE)
    source_group("${name}\\Header Files" FILES ${ARGN})
    list(APPEND ${out} ${ARGN})
endmacro()
macro(POCO_INSTALL target_name)
install(
    DIRECTORY include/Poco
    DESTINATION include
    COMPONENT Devel
    PATTERN ".svn" EXCLUDE
    )

install(
    TARGETS "${target_name}" EXPORT "${target_name}Targets"
    LIBRARY DESTINATION lib${LIB_SUFFIX}
    ARCHIVE DESTINATION lib${LIB_SUFFIX}
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    )
endmacro()

# OS Detection
if(WIN32)
  add_definitions( -DPOCO_OS_FAMILY_WINDOWS -DUNICODE -D_UNICODE)
  #set(SYSLIBS iphlpapi gdi32 odbc32)
endif(WIN32)

if (UNIX AND NOT ANDROID )
  add_definitions( -DPOCO_OS_FAMILY_UNIX )
  # Standard 'must be' defines
  if (APPLE)
    add_definitions( -DPOCO_HAVE_IPv6 -DPOCO_NO_STAT64)
    set(SYSLIBS  dl)
  else (APPLE)
    add_definitions( -D_XOPEN_SOURCE=500 -D_REENTRANT -D_THREAD_SAFE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
    set(SYSLIBS  pthread dl rt)
  endif (APPLE)
endif(UNIX AND NOT ANDROID )

if (CMAKE_SYSTEM MATCHES "SunOS")
  add_definitions( -DPOCO_OS_FAMILY_UNIX )
  # Standard 'must be' defines
  add_definitions( -D_XOPEN_SOURCE=500 -D_REENTRANT -D_THREAD_SAFE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 )
  set(SYSLIBS  pthread socket xnet nsl resolv rt dl)
endif(CMAKE_SYSTEM MATCHES "SunOS")

if (CMAKE_COMPILER_IS_MINGW)
  add_definitions(-DWC_NO_BEST_FIT_CHARS=0x400  -DPOCO_WIN32_UTF8)
  add_definitions(-D_WIN32 -DMINGW32 -DWINVER=0x500 -DODBCVER=0x0300 -DPOCO_THREAD_STACK_SIZE)
endif (CMAKE_COMPILER_IS_MINGW)

if (CYGWIN)
#    add_definitions(-DWC_NO_BEST_FIT_CHARS=0x400  -DPOCO_WIN32_UTF8)
endif (CYGWIN)

# SunPro C++
if (${CMAKE_CXX_COMPILER_ID} MATCHES "SunPro")
  add_definitions( -D_BSD_SOURCE -library=stlport4)
endif (${CMAKE_CXX_COMPILER_ID} MATCHES "SunPro")

# iOS
if (IOS)
  add_definitions( -DPOCO_HAVE_IPv6 -DPOCO_NO_FPENVIRONMENT -DPOCO_NO_STAT64 -DPOCO_NO_SHAREDLIBS -DPOCO_NO_NET_IFTYPES )
endif(IOS)

#Android
if (ANDROID)
  add_definitions( -DPOCO_ANDROID -DPOCO_NO_FPENVIRONMENT -DPOCO_NO_WSTRING -DPOCO_NO_SHAREDMEMORY )
endif(ANDROID)

# Sources
file(GLOB SRCS_G "src/*.cpp")
POCO_SOURCES_AUTO( SRCS ${SRCS_G})

# Headers
file(GLOB_RECURSE HDRS_G "include/*.h" )
POCO_HEADERS_AUTO( SRCS ${HDRS_G})

# If POCO_UNBUNDLED is enabled we try to find the required packages
# The configuration will fail if the packages are not found
if (POCO_UNBUNDLED)
    find_package(PCRE REQUIRED)
    set(SYSLIBS ${SYSLIBS} ${PCRE_LIBRARIES})
    include_directories(${PCRE_INCLUDE_DIRS})

    #HACK: Unicode.cpp requires functions from these files. The can't be taken from the library
    POCO_SOURCES( SRCS RegExp
        src/pcre_ucd.c
        src/pcre_tables.c
    )

    find_package(ZLIB REQUIRED)
    set(SYSLIBS ${SYSLIBS} ${ZLIB_LIBRARIES})
    include_directories(${ZLIB_INCLUDE_DIRS})
else()
    # pcre
    POCO_SOURCES( SRCS pcre
        src/pcre_config.c
        src/pcre_byte_order.c
        src/pcre_chartables.c
        src/pcre_compile.c
        src/pcre_exec.c
        src/pcre_fullinfo.c
        src/pcre_globals.c
        src/pcre_maketables.c
        src/pcre_newline.c
        src/pcre_ord2utf8.c
        src/pcre_study.c
        src/pcre_tables.c
        src/pcre_dfa_exec.c
        src/pcre_get.c
        src/pcre_jit_compile.c
        src/pcre_refcount.c
        src/pcre_string_utils.c
        src/pcre_version.c
        src/pcre_ucd.c
        src/pcre_valid_utf8.c
        src/pcre_xclass.c
    )

    # zlib
    POCO_HEADERS( SRCS zlib
        include/Poco/zconf.h
        include/Poco/zlib.h
        )

    POCO_SOURCES( SRCS zlib
        src/adler32.c
        src/compress.c
        src/crc32.c
        src/deflate.c
        src/infback.c
        src/inffast.c
        src/inflate.c
        src/inftrees.c
        src/trees.c
        src/zutil.c
    )
endif (POCO_UNBUNDLED)

if(WIN32)
    set(SYSLIBS ${SYSLIBS} iphlpapi)
endif(WIN32)
if (${CMAKE_CXX_COMPILER_ID} MATCHES "SunPro")
    set_target_properties( "${LIBNAME}" PROPERTIES LINK_FLAGS "-library=stlport4")
endif (${CMAKE_CXX_COMPILER_ID} MATCHES "SunPro")

add_definitions( -D${LIBNAME}_EXPORTS)

add_library( "${LIBNAME}" ${LIB_MODE} ${SRCS})
add_library( "${POCO_LIBNAME}" ALIAS "${LIBNAME}")
set_target_properties( "${LIBNAME}"
    PROPERTIES
    VERSION ${PROJECT_VERSION} SOVERSION ${SHARED_LIBRARY_VERSION}
    OUTPUT_NAME ${POCO_LIBNAME}
    DEFINE_SYMBOL ${LIBNAME}_EXPORTS
    )

target_link_libraries( "${LIBNAME}" ${SYSLIBS})
target_include_directories( "${LIBNAME}"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

POCO_INSTALL("${LIBNAME}")
