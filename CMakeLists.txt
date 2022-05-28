cmake_minimum_required(VERSION 3.22)
set(NAME "autoBodyAE")
set(VERSION 2.0.0)
set(PROJECT_VERSION ${VERSION})
option(COPY_OUTPUT "copy the output of build operations to the game directory" OFF)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

macro(set_from_environment VARIABLE)
	if(NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
		set(${VARIABLE} $ENV{${VARIABLE}})
	endif()
endmacro()


set_from_environment(Skyrim64Path)
set(SkyrimPath ${Skyrim64Path})
set(SkyrimVersion "all runtimes")
set(VERSION ${VERSION}.0)


message(
	STATUS
	"Building ${NAME} ${VERSION} for ${SkyrimVersion} at ${SkyrimPath}."
)

project(
	${NAME}
	VERSION ${VERSION}
	LANGUAGES CXX
)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(FATAL_ERROR "in-source builds are not allowed")
endif()


list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_subdirectory(src)
