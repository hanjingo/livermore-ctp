# FindCTP.cmake
# Find CTP (China Trading Platform) API library
#
# Usage:
#   find_package(CTP REQUIRED)
#   find_package(CTP REQUIRED COMPONENTS trader md)
#
# Defined variables:
#   CTP_FOUND - TRUE if CTP library is found
#   CTP_INCLUDE_DIRS - CTP header file directories
#   CTP_LIBRARIES - CTP library file list
#   CTP_VERSION - CTP version number
#
# Defined targets:
#   CTP::ctp - Complete CTP interface library
#   CTP::trader - CTP trading interface library
#   CTP::md - CTP market data interface library

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

# Set search paths
set(CTP_SEARCH_PATHS
    ${CTP_ROOT}
    $ENV{CTP_ROOT}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)

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

# Search CTP root directory
foreach(search_path ${CTP_SEARCH_PATHS})
    find_ctp_api_directory("${search_path}" CTP_BASE_PATH)
    if(CTP_BASE_PATH)
        break()
    endif()
endforeach()

if(NOT CTP_BASE_PATH)
    if(CTP_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find CTP API directory. Please set CTP_ROOT or ensure CTP is in the expected location.")
    endif()
    return()
endif()

# Find header files
find_path(CTP_INCLUDE_DIR
    NAMES ThostFtdcTraderApi.h ThostFtdcMdApi.h
    PATHS ${CTP_BASE_PATH}
    NO_DEFAULT_PATH
)

# Find library files
if(WIN32)
    find_library(CTP_TRADER_LIBRARY
        NAMES thosttraderapi_se thosttraderapi
        PATHS ${CTP_BASE_PATH}
        NO_DEFAULT_PATH
    )
    
    find_library(CTP_MD_LIBRARY
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
    find_library(CTP_TRADER_LIBRARY
        NAMES thosttraderapi_se thosttraderapi
        PATHS ${CTP_BASE_PATH}
        NO_DEFAULT_PATH
    )
    
    find_library(CTP_MD_LIBRARY
        NAMES thostmduserapi_se thostmduserapi
        PATHS ${CTP_BASE_PATH}
        NO_DEFAULT_PATH
    )
endif()

# Set variables
set(CTP_VERSION "6.7.11")
set(CTP_INCLUDE_DIRS ${CTP_INCLUDE_DIR})
set(CTP_LIBRARIES)

if(CTP_TRADER_LIBRARY)
    list(APPEND CTP_LIBRARIES ${CTP_TRADER_LIBRARY})
endif()

if(CTP_MD_LIBRARY)
    list(APPEND CTP_LIBRARIES ${CTP_MD_LIBRARY})
endif()

# Handle components
set(CTP_REQUIRED_COMPONENTS)
set(CTP_AVAILABLE_COMPONENTS trader md)

if(CTP_FIND_COMPONENTS)
    foreach(component ${CTP_FIND_COMPONENTS})
        if(NOT component IN_LIST CTP_AVAILABLE_COMPONENTS)
            if(CTP_FIND_REQUIRED_${component})
                message(FATAL_ERROR "CTP component '${component}' not available")
            endif()
        else()
            list(APPEND CTP_REQUIRED_COMPONENTS ${component})
        endif()
    endforeach()
else()
    set(CTP_REQUIRED_COMPONENTS ${CTP_AVAILABLE_COMPONENTS})
endif()

# Check if components are found
foreach(component ${CTP_REQUIRED_COMPONENTS})
    if(component STREQUAL "trader")
        if(CTP_TRADER_LIBRARY)
            set(CTP_${component}_FOUND TRUE)
        else()
            set(CTP_${component}_FOUND FALSE)
        endif()
    elseif(component STREQUAL "md")
        if(CTP_MD_LIBRARY)
            set(CTP_${component}_FOUND TRUE)
        else()
            set(CTP_${component}_FOUND FALSE)
        endif()
    endif()
    
    if(CTP_FIND_REQUIRED_${component} AND NOT CTP_${component}_FOUND)
        message(FATAL_ERROR "CTP component '${component}' is required but not found")
    endif()
endforeach()

# Use standard find package handling
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CTP
    REQUIRED_VARS CTP_INCLUDE_DIR CTP_LIBRARIES
    VERSION_VAR CTP_VERSION
    HANDLE_COMPONENTS
)

# Create imported targets
if(CTP_FOUND AND NOT TARGET CTP::ctp)
    # Create trading interface target
    if(CTP_TRADER_LIBRARY AND "trader" IN_LIST CTP_REQUIRED_COMPONENTS)
        add_library(CTP::trader SHARED IMPORTED)
        set_target_properties(CTP::trader PROPERTIES
            IMPORTED_LOCATION ${CTP_TRADER_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${CTP_INCLUDE_DIR}
        )
        
        if(WIN32 AND CTP_TRADER_DLL)
            set_target_properties(CTP::trader PROPERTIES
                IMPORTED_IMPLIB ${CTP_TRADER_LIBRARY}
                IMPORTED_LOCATION ${CTP_TRADER_DLL}
            )
        endif()
    endif()
    
    # Create market data interface target
    if(CTP_MD_LIBRARY AND "md" IN_LIST CTP_REQUIRED_COMPONENTS)
        add_library(CTP::md SHARED IMPORTED)
        set_target_properties(CTP::md PROPERTIES
            IMPORTED_LOCATION ${CTP_MD_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${CTP_INCLUDE_DIR}
        )
        
        if(WIN32 AND CTP_MD_DLL)
            set_target_properties(CTP::md PROPERTIES
                IMPORTED_IMPLIB ${CTP_MD_LIBRARY}
                IMPORTED_LOCATION ${CTP_MD_DLL}
            )
        endif()
    endif()
    
    # Create complete CTP interface target
    add_library(CTP::ctp INTERFACE IMPORTED)
    set_target_properties(CTP::ctp PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${CTP_INCLUDE_DIR}
    )
    
    set(CTP_LINK_LIBRARIES)
    if(TARGET CTP::trader)
        list(APPEND CTP_LINK_LIBRARIES CTP::trader)
    endif()
    if(TARGET CTP::md)
        list(APPEND CTP_LINK_LIBRARIES CTP::md)
    endif()
    
    if(CTP_LINK_LIBRARIES)
        set_target_properties(CTP::ctp PROPERTIES
            INTERFACE_LINK_LIBRARIES "${CTP_LINK_LIBRARIES}"
        )
    endif()
    
    # Set compile definitions
    if(WIN32)
        set_target_properties(CTP::ctp PROPERTIES
            INTERFACE_COMPILE_DEFINITIONS "WIN32;_WIN32"
        )
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set_property(TARGET CTP::ctp APPEND PROPERTY
                INTERFACE_COMPILE_DEFINITIONS "WIN64;_WIN64"
            )
        endif()
    endif()
endif()

# Mark as advanced variables
mark_as_advanced(
    CTP_INCLUDE_DIR
    CTP_TRADER_LIBRARY
    CTP_MD_LIBRARY
    CTP_TRADER_DLL
    CTP_MD_DLL
)