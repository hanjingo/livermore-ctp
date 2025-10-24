cmake_minimum_required(VERSION 3.19.2)

project(ctp VERSION 6.7.11 DESCRIPTION "CTP Trading API Library")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Platform and architecture detection
if(WIN32)
    set(CTP_PLATFORM "winApi")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CTP_ARCH "64")
    else()
        set(CTP_ARCH "32")
    endif()
elseif(UNIX)
    set(CTP_PLATFORM "linux")
    set(CTP_ARCH "64")
endif()

# Function to dynamically search CTP API directory
function(find_ctp_api_directory search_root result_var)
    set(${result_var} "" PARENT_SCOPE)
    
    if(NOT EXISTS "${search_root}/res")
        return()
    endif()
    
    # Search all possible version directories
    file(GLOB version_dirs "${search_root}/res/*traderapi*")
    
    foreach(version_dir ${version_dirs})
        if(NOT IS_DIRECTORY "${version_dir}")
            continue()
        endif()
        
        if(WIN32)
            # Windows platform search
            file(GLOB winapi_dirs "${version_dir}/*winApi*")
            foreach(winapi_dir ${winapi_dirs})
                if(NOT IS_DIRECTORY "${winapi_dir}")
                    continue()
                endif()
                
                file(GLOB trader_dirs "${winapi_dir}/traderapi*" "${winapi_dir}/*traderapi*")
                foreach(trader_dir ${trader_dirs})
                    if(NOT IS_DIRECTORY "${trader_dir}")
                        continue()
                    endif()
                    
                    # Search directories matching architecture
                    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                        # 64-bit: prioritize directories containing 64
                        file(GLOB api_dirs 
                            "${trader_dir}/*64*windows*"
                            "${trader_dir}/*windows*64*"
                            "${trader_dir}/*traderapi64*"
                        )
                    else()
                        # 32-bit: find directories not containing 64
                        file(GLOB all_dirs "${trader_dir}/*windows*")
                        set(api_dirs)
                        foreach(dir ${all_dirs})
                            if(NOT "${dir}" MATCHES "64")
                                list(APPEND api_dirs "${dir}")
                            endif()
                        endforeach()
                    endif()
                    
                    # Check if found directories contain necessary header files
                    foreach(api_dir ${api_dirs})
                        if(IS_DIRECTORY "${api_dir}" AND 
                           EXISTS "${api_dir}/ThostFtdcTraderApi.h" AND
                           EXISTS "${api_dir}/ThostFtdcMdApi.h")
                            set(${result_var} "${api_dir}" PARENT_SCOPE)
                            return()
                        endif()
                    endforeach()
                endforeach()
            endforeach()
            
        elseif(UNIX)
            # Linux platform search
            file(GLOB linux_dirs 
                "${version_dir}/*linux*"
                "${version_dir}/*api*linux*"
            )
            
            foreach(linux_dir ${linux_dirs})
                if(NOT IS_DIRECTORY "${linux_dir}")
                    continue()
                endif()
                
                # Recursively search directories containing API header files
                file(GLOB_RECURSE api_headers "${linux_dir}/ThostFtdcTraderApi.h")
                foreach(header_path ${api_headers})
                    get_filename_component(api_dir "${header_path}" DIRECTORY)
                    if(EXISTS "${api_dir}/ThostFtdcMdApi.h")
                        set(${result_var} "${api_dir}" PARENT_SCOPE)
                        return()
                    endif()
                endforeach()
            endforeach()
        endif()
    endforeach()
endfunction()

# Search for CTP API directory
find_ctp_api_directory("${CMAKE_CURRENT_SOURCE_DIR}" CTP_BASE_PATH)

# Check if CTP API directory exists
if(NOT CTP_BASE_PATH)
    message(FATAL_ERROR "CTP API directory not found. Please run configure.sh or configure.ps1 to download CTP API files, or set CTP_ROOT environment variable.")
endif()

# Find CTP header files
find_path(CTP_INCLUDE_DIR
    NAMES ThostFtdcTraderApi.h ThostFtdcMdApi.h
    PATHS ${CTP_BASE_PATH}
    NO_DEFAULT_PATH
)

if(NOT CTP_INCLUDE_DIR)
    message(FATAL_ERROR "CTP headers not found in ${CTP_BASE_PATH}")
endif()

# Find CTP library files
if(WIN32)
    # Windows platform: find .lib files
    find_library(CTP_TRADER_LIB
        NAMES thosttraderapi_se thosttraderapi
        PATHS ${CTP_BASE_PATH}
        NO_DEFAULT_PATH
    )
    
    find_library(CTP_MD_LIB
        NAMES thostmduserapi_se thostmduserapi
        PATHS ${CTP_BASE_PATH}
        NO_DEFAULT_PATH
    )
    
    # Find DLL files
    find_file(CTP_TRADER_DLL
        NAMES thosttraderapi_se.dll thosttraderapi.dll
        PATHS ${CTP_BASE_PATH}
        NO_DEFAULT_PATH
    )
    
    find_file(CTP_MD_DLL
        NAMES thostmduserapi_se.dll thostmduserapi.dll
        PATHS ${CTP_BASE_PATH}
        NO_DEFAULT_PATH
    )
elseif(UNIX)
    # Linux platform: find .so files
    find_library(CTP_TRADER_LIB
        NAMES thosttraderapi_se thosttraderapi
        PATHS ${CTP_BASE_PATH}
        NO_DEFAULT_PATH
    )
    
    find_library(CTP_MD_LIB
        NAMES thostmduserapi_se thostmduserapi
        PATHS ${CTP_BASE_PATH}
        NO_DEFAULT_PATH
    )
endif()

# Verify found library files
if(WIN32)
    if(NOT CTP_TRADER_LIB OR NOT CTP_MD_LIB)
        message(WARNING "CTP libraries not found in ${CTP_BASE_PATH}")
        message(STATUS "Trader lib: ${CTP_TRADER_LIB}")
        message(STATUS "MD lib: ${CTP_MD_LIB}")
    endif()
elseif(UNIX)
    if(NOT CTP_TRADER_LIB OR NOT CTP_MD_LIB)
        message(WARNING "CTP libraries not found in ${CTP_BASE_PATH}")
        message(STATUS "Trader lib: ${CTP_TRADER_LIB}")
        message(STATUS "MD lib: ${CTP_MD_LIB}")
    endif()
endif()

# Create CTP interface library
add_library(ctp INTERFACE)

# Set header file paths
target_include_directories(ctp INTERFACE
    $<BUILD_INTERFACE:${CTP_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link libraries
if(CTP_TRADER_LIB AND CTP_MD_LIB)
    target_link_libraries(ctp INTERFACE ${CTP_TRADER_LIB} ${CTP_MD_LIB})
endif()

# Compile definitions
if(WIN32)
    target_compile_definitions(ctp INTERFACE WIN32 _WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_compile_definitions(ctp INTERFACE WIN64 _WIN64)
    endif()
endif()

# Create CTP Trader library target
add_library(ctp_trader INTERFACE)
target_include_directories(ctp_trader INTERFACE
    $<BUILD_INTERFACE:${CTP_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
if(CTP_TRADER_LIB)
    target_link_libraries(ctp_trader INTERFACE ${CTP_TRADER_LIB})
endif()

# Create CTP Market Data library target
add_library(ctp_md INTERFACE)
target_include_directories(ctp_md INTERFACE
    $<BUILD_INTERFACE:${CTP_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
if(CTP_MD_LIB)
    target_link_libraries(ctp_md INTERFACE ${CTP_MD_LIB})
endif()

# Installation configuration
include(GNUInstallDirs)

# Install header files
install(FILES
    ${CTP_INCLUDE_DIR}/ThostFtdcTraderApi.h
    ${CTP_INCLUDE_DIR}/ThostFtdcMdApi.h  
    ${CTP_INCLUDE_DIR}/ThostFtdcUserApiStruct.h
    ${CTP_INCLUDE_DIR}/ThostFtdcUserApiDataType.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ctp
)

# Install custom header files
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )
endif()

# Install library files and DLLs
if(WIN32)
    if(CTP_TRADER_LIB)
        install(FILES ${CTP_TRADER_LIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
    if(CTP_MD_LIB)
        install(FILES ${CTP_MD_LIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
    if(CTP_TRADER_DLL)
        install(FILES ${CTP_TRADER_DLL} DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
    if(CTP_MD_DLL)
        install(FILES ${CTP_MD_DLL} DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
elseif(UNIX)
    if(CTP_TRADER_LIB)
        install(FILES ${CTP_TRADER_LIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
    if(CTP_MD_LIB)
        install(FILES ${CTP_MD_LIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
endif()

# Install targets
install(TARGETS ctp ctp_trader ctp_md
    EXPORT ctpTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Create and install package configuration files
install(EXPORT ctpTargets
    FILE ctpTargets.cmake
    NAMESPACE ctp::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ctp
)

# Create version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ctpConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Configure package configuration file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ctpConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ctpConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ctp
)

# Install configuration files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ctpConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ctpConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ctp
)

# Optional: build examples and tests
option(BUILD_EXAMPLES "Build example programs" OFF)
if(BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/example)
    add_subdirectory(example)
endif()

# Display configuration information
message(STATUS "CTP Configuration:")
message(STATUS "  Platform: ${CTP_PLATFORM}")
message(STATUS "  Architecture: ${CTP_ARCH}")
message(STATUS "  API Directory: ${CTP_BASE_PATH}")
message(STATUS "  Include Directory: ${CTP_INCLUDE_DIR}")
if(CTP_TRADER_LIB)
    message(STATUS "  Trader Library: ${CTP_TRADER_LIB}")
endif()
if(CTP_MD_LIB)
    message(STATUS "  Market Data Library: ${CTP_MD_LIB}")
endif()