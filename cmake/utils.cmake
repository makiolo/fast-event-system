option(COVERAGE "active coverage (only clang)" FALSE)
option(SANITIZER "active sanitizers (address,address-full,memory,thread) (only clang)" "")

macro(GENERATE_CLANG)
	# Generate .clang_complete for full completation in vim + clang_complete
	set(extra_parameters "")
	get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
	foreach(dir ${dirs})
	  set(extra_parameters ${extra_parameters} -I${dir})
	endforeach()
	get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
	foreach(dir ${dirs})
	  set(extra_parameters ${extra_parameters} -D${dir})
	endforeach()
	STRING(REGEX REPLACE ";" "\n" extra_parameters "${extra_parameters}")
	FILE(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.clang_complete" "${extra_parameters}\n")
endmacro()

macro(COMMONS_FLAGS)
	if(SANITIZER)
		add_definitions(-g3)
		# "address-full" "memory" "thread"
		SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=${SANITIZER}")
		SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=${SANITIZER}")
		add_definitions(-fsanitize=${SANITIZER})
	endif()
	IF(COVERAGE)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
		add_definitions(-coverage)
	endif()

	include_directories(BEFORE ${TOOLCHAIN_ROOT}/include)
	link_directories(${TOOLCHAIN_ROOT}/lib)
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc")
endmacro()

macro(ENABLE_MODERN_CPP)

    if(WIN32)
        add_definitions(/EHsc)
        #add_definitions(/GR-)
        #add_definitions(/D_HAS_EXCEPTIONS=0)
    else()
        # add_definitions(-fno-rtti -fno-exceptions )
		# activate all warnings and convert in errors
        add_definitions(-Wall -Weffc++ -pedantic -pedantic-errors -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion)
        add_definitions(-Wdisabled-optimization -Werror -Wfloat-equal -Wformat=2 -Wformat-nonliteral -Wformat-security -Wformat-y2k)
        add_definitions(-Wimport  -Winit-self  -Winline -Winvalid-pch -Wlong-long -Wmissing-field-initializers -Wmissing-format-attribute)
		add_definitions(-Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow)
        add_definitions(-Wstack-protector -Wstrict-aliasing=2 -Wswitch-default -Wswitch-enum -Wunreachable-code -Wunused)
        add_definitions(-Wunused-parameter -Wvariadic-macros -Wwrite-strings)
		# only gcc
		#add_definitions(-Wthread-safety)
		# convert error in warnings
		add_definitions(-Wno-error=shadow)
        add_definitions(-Wno-error=long-long)
        add_definitions(-Wno-error=aggregate-return)
		# In Linux default now is not export symbols
		add_definitions(-fvisibility=hidden)
        SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pthread" )
        SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -lpthread" )
    endif()

	if (NOT DEFINED EXTRA_DEF)
		include(CheckCXXCompilerFlag)
		#CHECK_CXX_COMPILER_FLAG("-std=c++1z" COMPILER_SUPPORTS_CXX1Z)
		CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
		CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)
		CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
		CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

		#if(COMPILER_SUPPORTS_CXX1Z)
		#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
		#message("-- C++1z Enabled")
		if(COMPILER_SUPPORTS_CXX14)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
			message("-- C++14 Enabled")
		#elseif(COMPILER_SUPPORTS_CXX1Y)
			#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
			#message("-- C++1y Enabled")
		elseif(COMPILER_SUPPORTS_CXX11)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
			message("-- C++11 Enabled")
		elseif(COMPILER_SUPPORTS_CXX0X)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
			message("-- C++0x Enabled")
		else()
			message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
		endif()
	else()
		add_definitions(${EXTRA_DEF})
	endif()

endmacro()

macro(CREATE_TEST TESTNAME TESTDEPENDS)
	include_directories(..)
	COMMONS_FLAGS()
	ADD_EXECUTABLE(${TESTNAME} ${TESTNAME}.cpp)
	target_link_libraries(${TESTNAME} ${TESTDEPENDS})
	ADD_TEST(	NAME ${TESTNAME}
		COMMAND ${CMAKE_SOURCE_DIR}/cmake/run_test.sh ${TESTNAME}
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endmacro()

macro(GET_SYSTEM_INFO)

    if(MSVC12)
        set(MSVC_NAME_LOWER "vc120")
        set(MSVC_NAME_UPPER "VC120")
    elseif(MSVC11)
        set(MSVC_NAME_LOWER "vc110")
        set(MSVC_NAME_UPPER "VC110")
    elseif(MSVC10)
        set(MSVC_NAME_LOWER "vc100")
        set(MSVC_NAME_UPPER "VC100")
    elseif(MSVC9)
        set(MSVC_NAME_LOWER "vc90")
        set(MSVC_NAME_UPPER "VC90")
    else(MSVC8)
        set(MSVC_NAME_LOWER "vc80")
        set(MSVC_NAME_UPPER "VC80")
    endif()

    IF(WIN32)
        SET(COMPILER "${MSVC_NAME_UPPER}")
    ELSE()
        SET(compiler "")
        get_filename_component(compiler ${CMAKE_CXX_COMPILER} NAME)
        SET(COMPILER "${compiler}")
    ENDIF()

    if(CMAKE_SYSTEM_NAME MATCHES Linux)
        set(PLATFORM "linux_amd64")
    elseif(CMAKE_SYSTEM_NAME MATCHES SunOS)
        set(PLATFORM "solaris_sparc32")
    elseif(CMAKE_SYSTEM_NAME MATCHES Windows)
        if(CMAKE_CL_64)
            set(PLATFORM "win64")
        else(CMAKE_CL_64)
            set(PLATFORM "win32")
        endif(CMAKE_CL_64)
    endif()

endmacro()

macro(generate_vcxproj_user _EXECUTABLE_NAME)
    IF(MSVC)
        set(project_vcxproj_user "${CMAKE_CURRENT_BINARY_DIR}/${_EXECUTABLE_NAME}.vcxproj.user")
        if (NOT EXISTS ${project_vcxproj_user})
            FILE(WRITE "${project_vcxproj_user}"
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            "<Project ToolsVersion=\"12.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
            "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
            "<LocalDebuggerWorkingDirectory>$(TargetDir)</LocalDebuggerWorkingDirectory>\n"
            "<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>\n"
            "</PropertyGroup>\n"
            "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='RelWithDebInfo|x64'\">\n"
            "<LocalDebuggerWorkingDirectory>$(TargetDir)</LocalDebuggerWorkingDirectory>\n"
            "<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>\n"
            "</PropertyGroup>\n"
            "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">\n"
            "<LocalDebuggerWorkingDirectory>$(TargetDir)</LocalDebuggerWorkingDirectory>\n"
            "<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>\n"
            "</PropertyGroup>\n"
            "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='MinSizeRel|x64'\">\n"
            "<LocalDebuggerWorkingDirectory>$(TargetDir)</LocalDebuggerWorkingDirectory>\n"
            "<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>\n"
            "</PropertyGroup>\n"
            "</Project>\n")
        endif()
    ENDIF()
endmacro()

function(GENERATE_EXE _EXECUTABLE_NAME _SOURCE_FILES _HEADERS_FILES)

    source_group( "Source Files" FILES ${_SOURCE_FILES} )
    source_group( "Header Files" FILES ${_HEADERS_FILES} )

	# -g -fsanitize=address-full (buffer overflows)
	# -g -fsanitize=thread (condiciones de carrera)
	# -g -fsanitize=memory (detectar variables no inicializadas)

	COMMONS_FLAGS()
    IF(WIN32)
		add_definitions(/WX /W4)
        ADD_EXECUTABLE(${_EXECUTABLE_NAME} WIN32 ${_SOURCE_FILES} ${_HEADERS_FILES})
    ELSEIF()
        ADD_EXECUTABLE(${_EXECUTABLE_NAME} ${_SOURCE_FILES} ${_HEADERS_FILES})
    ENDIF()

	# For detect stackoverflows / buffer-overflow...
	# sanitizer address
	# -g -fsanitize=address
	# -g -fsanitize=address-full (buffer overflows)
	# -g -fsanitize=thread (condiciones de carrera)
	# -g -fsanitize=memory (detectar variables no inicializadas)
	#
	# ASAN_OPTIONS="detect_leaks=1" executable

    generate_vcxproj_user(${_EXECUTABLE_NAME})

endfunction()

function(GENERATE_LIB)

    set(PARAMETERS ${ARGV})
    list(GET PARAMETERS 0 LIBNAME)
    list(REMOVE_AT PARAMETERS 0)

    SET(HAVE_TESTS FALSE)
    SET(HAVE_PCH FALSE)
    set(TARGET_DEPENDENCIES)
    set(EXTRA_SOURCES)
    set(TESTS_SOURCES)
    set(PCH_SOURCE)
    while(PARAMETERS)
        list(GET PARAMETERS 0 PARM)
        if(PARM STREQUAL DEPENDENCIES)
          set(NOW_IN DEPENDENCIES)
        elseif(PARM STREQUAL EXTRA_SOURCES)
          set(NOW_IN EXTRA_SOURCES)
        elseif(PARM STREQUAL TESTS)
          set(NOW_IN TESTS)
        elseif(PARM STREQUAL PCH)
          set(NOW_IN PCH)
        else()
          if(NOW_IN STREQUAL DEPENDENCIES)
            set(TARGET_DEPENDENCIES ${TARGET_DEPENDENCIES} ${PARM})
          elseif(NOW_IN STREQUAL EXTRA_SOURCES)
            set(EXTRA_SOURCES ${EXTRA_SOURCES} ${PARM})
          elseif(NOW_IN STREQUAL TESTS)
            set(TESTS_SOURCES ${TESTS_SOURCES} ${PARM})
            SET(HAVE_TESTS TRUE)
          elseif(NOW_IN STREQUAL PCH)
            set(PCH_SOURCE ${PARM})
            SET(HAVE_PCH TRUE)
          else()
            message(FATAL_ERROR "Unknown argument ${PARM}.")
          endif()
        endif()
        list(REMOVE_AT PARAMETERS 0)
    endwhile()

    INCLUDE_DIRECTORIES(..)

    file( GLOB SOURCE_FILES [Cc]/*.c [Cc]/*.cpp [Cc]/*.cxx *.cpp *.c *.cxx )
    file( GLOB HEADERS_FILES [Hh]/*.h [Hh]/*.hpp [Hh]/*.hxx [Hh][Pp][Pp]/*.hpp [Hh][Pp][Pp]/*.hxx *.h *.hpp *.hxx )
	IF(WIN32)
		file( GLOB SPECIFIC_PLATFORM c/win32/*.cpp )
	ELSEIF(MAC)
		file( GLOB SPECIFIC_PLATFORM c/mac/*.cpp )
	ELSEIF(LINUX)
		file( GLOB SPECIFIC_PLATFORM c/linux/*.cpp )
	ELSEIF(ANDROID)
		file( GLOB SPECIFIC_PLATFORM c/android/*.cpp )
	ENDIF()

	SET(SOURCE_FILES ${SOURCE_FILES} "${SPECIFIC_PLATFORM}")
    source_group( "c" FILES ${SOURCE_FILES})
    source_group( "h" FILES ${HEADERS_FILES})

	if(WIN32)
		# c++ exceptions and RTTI
		#add_definitions(/D_HAS_EXCEPTIONS=0)
		#add_definitions(/GR-)
		add_definitions(/wd4251)
		add_definitions(/wd4275)
		# Avoid warning as error with / WX / W4
		# conversion from 'std::reference_wrapper<Chunk>' to 'std::reference_wrapper<Chunk> &
		add_definitions(/wd4239)
		# warning C4316: 'PhysicsManager' : object allocated on the heap may not be aligned 16
		add_definitions(/wd4316)
		add_definitions(/WX /W4)
	endif()

	COMMONS_FLAGS()
    ADD_LIBRARY(${LIBNAME} SHARED ${SOURCE_FILES} ${HEADERS_FILES} ${EXTRA_SOURCES})
    TARGET_LINK_LIBRARIES(${LIBNAME} ${TARGET_DEPENDENCIES} ${TARGET_3RDPARTY_DEPENDENCIES})
	#set_target_properties(${LIBNAME} PROPERTIES SUFFIX .pyd)
	IF(WIN32)
		set_target_properties(${LIBNAME} PROPERTIES SUFFIX .dll)
	ENDIF()

    if(HAVE_PCH)
        add_definitions(-Zm200)
        #include(cotire)
        set_target_properties(${LIBNAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "h/${PCH_SOURCE}")
        set_target_properties(${LIBNAME} PROPERTIES COTIRE_UNITY_LINK_LIBRARIES_INIT "COPY")
        #cotire(${LIBNAME})
	endif()

	GENERATE_CLANG()

    foreach(BUILD_TYPE ${CMAKE_BUILD_TYPE})
        INSTALL(    TARGETS ${LIBNAME}
                    DESTINATION ${BUILD_TYPE}
                    CONFIGURATIONS ${BUILD_TYPE})
    endforeach()

    if(HAVE_TESTS)

        # ctest
        # http://johnlamp.net/cmake-tutorial-5-functionally-improved-testing.html
        enable_testing()

        #find_package(google-gmock ${GMOCK_REQUIRED_VERSION})

        source_group("Tests" FILES ${TESTS_SOURCES})

		COMMONS_FLAGS()
        ADD_EXECUTABLE(${LIBNAME}.Tests ${TESTS_SOURCES})
        TARGET_LINK_LIBRARIES(${LIBNAME}.Tests ${TARGET_DEPENDENCIES} ${TARGET_3RDPARTY_DEPENDENCIES} ${LIBNAME})
        ADD_TEST(   NAME ${LIBNAME}.Tests
                    COMMAND ${LIBNAME}.Tests
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

        generate_vcxproj_user(${LIBNAME}.Tests)

    endif()
endfunction()

function(PACK_FILES FROM TO)
    foreach(BUILD_TYPE ${CMAKE_BUILD_TYPE})
        FILE(GLOB files ${FROM})
        INSTALL(FILES ${files} DESTINATION ${BUILD_TYPE}/${TO} CONFIGURATIONS ${BUILD_TYPE})
    endforeach()
endfunction()

function(PACK_FILE_AND_RENAME FROM TO NEWNAME)
    foreach(BUILD_TYPE ${CMAKE_BUILD_TYPE})
        INSTALL(FILES ${FROM}
                                DESTINATION ${BUILD_TYPE}/${TO}
                                CONFIGURATIONS ${BUILD_TYPE}
                                RENAME ${NEWNAME})
    endforeach()
endfunction()

macro(PACK_FOLDER _DESTINE)
    file(GLOB DEPLOY_FILES_AND_DIRS "${_DESTINE}/*")
    foreach(ITEM ${DEPLOY_FILES_AND_DIRS})
       IF( IS_DIRECTORY "${ITEM}" )
          LIST( APPEND DIRS_TO_DEPLOY "${ITEM}" )
       ELSE()
          IF(ITEM STREQUAL "${_DESTINE}/CMakeLists.txt")
            MESSAGE("skipped file: ${_DESTINE}/CMakeLists.txt")
          ELSE()
            LIST(APPEND FILES_TO_DEPLOY "${ITEM}")
          ENDIF()
       ENDIF()
    endforeach()
    INSTALL( FILES ${FILES_TO_DEPLOY} DESTINATION ${CMAKE_INSTALL_PREFIX}/${GLOBAL_PLATFORM}/${GLOBAL_COMPILER})
    INSTALL( DIRECTORY ${DIRS_TO_DEPLOY} DESTINATION ${CMAKE_INSTALL_PREFIX}/${GLOBAL_PLATFORM}/${GLOBAL_COMPILER})
endmacro()
