cmake_minimum_required(VERSION 3.21)

# Define the Character Utilities project
project(charutil
        VERSION 1.0.0.0
        DESCRIPTION "Character Utilities Library"
        LANGUAGES CXX)

# Set options depending on whether this is a subproject
if(PROJECT_IS_TOP_LEVEL)
    # Option to control whether tests are built
    option(charutil_BUILD_TESTS "Build Tests for the Character Utilities Library" ON)
else()
    # Option to control whether tests are built
    option(charutil_BUILD_TESTS "Build Tests for the Character Utilities Library" OFF)
endif()

# Option to control ability to install the library
option(charutil_INSTALL "Install the Character Utilities Library" ON)

add_subdirectory(dependencies)
add_subdirectory(src)

include(CTest)

if(BUILD_TESTING AND charutil_BUILD_TESTS)
    add_subdirectory(test)
endif()
