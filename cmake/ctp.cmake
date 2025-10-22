cmake_minimum_required(VERSION 3.19.2)

# ctp.cmake - detect local CTP (Thost) headers and libraries under the project's res/ directory
# This file will search for ThostFtdcMdApi.h and set CTP_INCLUDE_DIRS and CTP_LIBRARIES
# as CACHE variables if they are not provided by the user.

# Default search root (repo res directory)
if (NOT DEFINED CTP_SEARCH_ROOT)
	set(CTP_SEARCH_ROOT "${CMAKE_CURRENT_LIST_DIR}/../res")
endif()

message(STATUS "CTP: search root = ${CTP_SEARCH_ROOT}")

# Only auto-detect if user didn't explicitly set CTP_INCLUDE_DIRS
if(NOT DEFINED CTP_INCLUDE_DIRS OR CTP_INCLUDE_DIRS STREQUAL "")
	if(EXISTS "${CTP_SEARCH_ROOT}")
		file(GLOB_RECURSE _ctp_headers RELATIVE "${CTP_SEARCH_ROOT}" "${CTP_SEARCH_ROOT}/*ThostFtdcMdApi.h")
		if(_ctp_headers)
			# Prefer Windows x64-like paths when building on Windows
			set(_selected_header "")
			if(WIN32)
				foreach(_h IN LISTS _ctp_headers)
					string(TOLOWER "${_h}" _h_lower)
					if(_h_lower MATCHES "windows" AND (_h_lower MATCHES "64" OR _h_lower MATCHES "traderapi64"))
						set(_selected_header "${_h}")
						break()
					endif()
				endforeach()
			endif()
			if(NOT _selected_header)
				list(GET _ctp_headers 0 _selected_header)
			endif()

			# Build absolute path to selected header directory
			get_filename_component(_abs_header "${CTP_SEARCH_ROOT}/${_selected_header}" ABSOLUTE)
			get_filename_component(_inc_dir ${_abs_header} DIRECTORY)

			set(CTP_INCLUDE_DIRS "${_inc_dir}")
			message(STATUS "CTP: detected header dir = ${CTP_INCLUDE_DIRS}")
		else()
			message(STATUS "CTP: no ThostFtdcMdApi.h found under ${CTP_SEARCH_ROOT}")
		endif()
	else()
		message(STATUS "CTP: search root does not exist: ${CTP_SEARCH_ROOT}")
	endif()
else()
	message(STATUS "CTP: user-provided CTP_INCLUDE_DIRS = ${CTP_INCLUDE_DIRS}")
endif()

# Only auto-detect CTP_LIBRARIES if not set by user
if(NOT DEFINED CTP_LIBRARIES OR CTP_LIBRARIES STREQUAL "")
	if(EXISTS "${CTP_SEARCH_ROOT}")
		# Collect only platform-appropriate library file extensions to avoid
		# passing incompatible files (e.g. Linux .so) to the MSVC linker on Windows.
		if(WIN32)
			# On Windows, collect import libraries (.lib) for linking and DLLs for runtime copying
			file(GLOB_RECURSE _ctp_libs "${CTP_SEARCH_ROOT}/*.lib")
			file(GLOB_RECURSE _ctp_dlls "${CTP_SEARCH_ROOT}/*.dll")
			list(REMOVE_DUPLICATES _ctp_libs)
			list(REMOVE_DUPLICATES _ctp_dlls)
			if(_ctp_libs)
				set(CTP_LIBRARIES "${_ctp_libs}")
				message(STATUS "CTP: detected import libraries: ${CTP_LIBRARIES}")
			else()
				message(STATUS "CTP: no import libraries (.lib) found under ${CTP_SEARCH_ROOT}")
			endif()
			if(_ctp_dlls)
				set(CTP_RUNTIME_DLLS "${_ctp_dlls}")
				message(STATUS "CTP: detected runtime DLLs: ${CTP_RUNTIME_DLLS}")
			else()
				message(STATUS "CTP: no DLLs found under ${CTP_SEARCH_ROOT}")
			endif()
		elseif(APPLE)
			file(GLOB_RECURSE _ctp_libs "${CTP_SEARCH_ROOT}/*.dylib" "${CTP_SEARCH_ROOT}/*.a")
			if(_ctp_libs)
				list(REMOVE_DUPLICATES _ctp_libs)
				set(CTP_LIBRARIES "${_ctp_libs}")
				message(STATUS "CTP: detected libraries: ${CTP_LIBRARIES}")
			else()
				message(STATUS "CTP: no libraries found under ${CTP_SEARCH_ROOT}")
			endif()
		else()
			# generic Unix/Linux
			file(GLOB_RECURSE _ctp_libs "${CTP_SEARCH_ROOT}/*.so" "${CTP_SEARCH_ROOT}/*.a")
			if(_ctp_libs)
				list(REMOVE_DUPLICATES _ctp_libs)
				set(CTP_LIBRARIES "${_ctp_libs}")
				message(STATUS "CTP: detected libraries: ${CTP_LIBRARIES}")
			else()
				message(STATUS "CTP: no libraries found under ${CTP_SEARCH_ROOT}")
			endif()
		endif()
	else()
		message(STATUS "CTP: search root does not exist: ${CTP_SEARCH_ROOT}")
	endif()
else()
	message(STATUS "CTP: user-provided CTP_LIBRARIES = ${CTP_LIBRARIES}")
endif()

# Export cache variables if we detected them (do not overwrite user-provided values)
if(DEFINED CTP_INCLUDE_DIRS AND NOT CTP_INCLUDE_DIRS STREQUAL "")
	set(CTP_INCLUDE_DIRS "${CTP_INCLUDE_DIRS}" CACHE PATH "CTP include directory (auto-detected)")
endif()
if(DEFINED CTP_LIBRARIES AND NOT CTP_LIBRARIES STREQUAL "")
	set(CTP_LIBRARIES "${CTP_LIBRARIES}" CACHE STRING "CTP library files (auto-detected)")
endif()

# Helpful note
message(STATUS "CTP: final CTP_INCLUDE_DIRS=${CTP_INCLUDE_DIRS}")
message(STATUS "CTP: final CTP_LIBRARIES=${CTP_LIBRARIES}")

