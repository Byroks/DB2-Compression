cmake_minimum_required(VERSION 3.14)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0076 NEW)
project(db2_programming_project VERSION 2022 LANGUAGES CXX)

#download and include cmake package manager
set(CPM_DOWNLOAD_VERSION 0.34.0)

if(CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
            https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
            ${CPM_DOWNLOAD_LOCATION}
            )
endif()

include(${CPM_DOWNLOAD_LOCATION})

#add dependencies through cmake package manager

#unittests
CPMAddPackage("gh:catchorg/Catch2@2.13.8")

#serialization
CPMAddPackage(
        NAME cereal
        VERSION 1.3.2
        GITHUB_REPOSITORY USCiLab/cereal
        OPTIONS "SKIP_PORTABILITY_TEST ON" "JUST_INSTALL_CEREAL ON"
)

#documentation target
option(BUILD_DOC "Build documentation" ON)
if (BUILD_DOC)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        add_custom_target(documentation
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc
                COMMENT "Generating API documentation..."
                VERBATIM)
    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#configure path for test data
configure_file(include/config.hpp.in include/config.hpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

#enable support for testing with catch in cmake
enable_testing()
include(CTest)
include(${Catch2_SOURCE_DIR}/contrib/Catch.cmake)

#add source
add_subdirectory(src)

#add support for packaging sources for submission
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES
        \\.git/
        \\.git*
        \\.idea*
        \\.vscode/
        \\.mypy_cache/
        \\.archive/
        "build*/"
        "build.*/"
        "cmake.*"
        doc/html
        .editorconfig
        .gdbinit
        ".*~$"
        )
set(CPACK_VERBATIM_VARIABLES YES)
include(CPack)