# Create the test executable
add_executable(test_utf8_to_utf16 test_utf8_to_utf16.cpp)

# Link to the required libraries
target_link_libraries(test_utf8_to_utf16 Terra::charutil Terra::stf)

# Include the source directory to get access to password_utilities.h
target_include_directories(test_utf8_to_utf16 PRIVATE ${PROJECT_SOURCE_DIR}/src)

# Specify the C++ standard to observe
set_target_properties(test_utf8_to_utf16
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)

# If requesting clang-tidy, try to look for it
if(charutil_CLANG_TIDY)
    find_program(CLANG_TIDY_COMMAND NAMES "clang-tidy")
    if(CLANG_TIDY_COMMAND)
        set_target_properties(test_utf8_to_utf16 PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
    else()
        message(WARNING "Could not find clang-tidy")
    endif()
endif()

# Specify the compiler options
target_compile_options(test_utf8_to_utf16
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wpedantic -Wextra -Wall -Wconversion -Wshadow>
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

# Ensure CTest can find the test
add_test(NAME test_utf8_to_utf16
         COMMAND test_utf8_to_utf16)
