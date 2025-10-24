# Example CMakeLists.txt for external projects using CTP library

cmake_minimum_required(VERSION 3.19.2)
project(ExampleTradingApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Method 1: Use find_package to find installed CTP library
# find_package(ctp REQUIRED)

# Method 2: Add CTP as subdirectory (for development use)
# Since the example is part of the main project, use parent project targets directly
# add_subdirectory(../livermore-ctp ${CMAKE_BINARY_DIR}/ctp)

# Or use FindCTP module
# list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../livermore-ctp/cmake)
# find_package(CTP REQUIRED COMPONENTS trader md)

# Create example application
add_executable(example_app
    main.cpp
    trader_example.cpp
    md_example.cpp
)

# Link CTP library
target_link_libraries(example_app PRIVATE ctp)

# Or link specific components
# target_link_libraries(example_app PRIVATE ctp_trader ctp_md)

# Set output directory
set_target_properties(example_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Windows specific configuration
if(WIN32)
    # Copy DLLs to output directory
    if(CTP_TRADER_DLL)
        add_custom_command(TARGET example_app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CTP_TRADER_DLL} $<TARGET_FILE_DIR:example_app>
            COMMENT "Copying CTP Trader DLL"
        )
    endif()
    
    if(CTP_MD_DLL)
        add_custom_command(TARGET example_app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CTP_MD_DLL} $<TARGET_FILE_DIR:example_app>
            COMMENT "Copying CTP Market Data DLL"
        )
    endif()
endif()