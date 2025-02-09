# Create the test excutable
add_executable(test_utf8_validity test_utf8_validity.cpp)

# Link to the required libraries
target_link_libraries(test_utf8_validity Terra::charutil Terra::stf)

# Include the source directory to get access to password_utilities.h
target_include_directories(test_utf8_validity PRIVATE ${PROJECT_SOURCE_DIR}/src)

# Specify the C++ standard to observe
set_target_properties(test_utf8_validity
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)

# Specify the compiler options
target_compile_options(test_utf8_validity
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wpedantic -Wextra -Wall>
        $<$<CXX_COMPILER_ID:MSVC>: /utf-8>)

# Ensure CTest can find the test
add_test(NAME test_utf8_validity
         COMMAND test_utf8_validity)
