# CTP (China Trading Platform) API Library

This is a CMake project for integrating CTP trading API, supporting Windows and Linux platforms, as well as different architectures (x86/x64).

## Directory Structure

```
├── CMakeLists.txt              # Main CMake configuration file
├── cmake/
│   ├── FindCTP.cmake          # CTP find module
│   └── ctpConfig.cmake.in     # CTP package configuration template
├── include/                   # Custom header files directory
│   ├── ctp_quote.h
│   └── ctp_trader.h
├── src/                       # Source code directory
│   ├── ctp_quote.cpp
│   └── ctp_trader.cpp
├── res/                       # CTP API resources directory
│   └── v6.7.11_20250714_traderapi/
│       ├── v6.7.11_20250617_winApi/
│       │   └── traderapi/
│       │       ├── 20250617_traderapi_se_windows/      # Windows 32-bit
│       │       └── 20250617_traderapi64_se_windows/    # Windows 64-bit
│       └── v6.7.11_20250617_api_traderapi_linux64/
│           └── v6.7.11_20250617_api/
│               └── v6.7.11_20250617_api_traderapi_se_linux64/  # Linux 64-bit
└── example/                   # Example code directory
```

## Platform Support

This project supports dynamic detection and search of CTP API directories, without hardcoding specific version paths. The system automatically searches for the following platforms:

### Windows
- **32-bit**: Automatically searches for directories containing `windows` but not `64`
- **64-bit**: Automatically searches for directories containing both `64` and `windows`

### Linux  
- **64-bit**: Automatically searches for directories containing `linux`

### Download CTP API

Use the provided scripts to download the latest CTP API:

**Windows (PowerShell):**
```powershell
.\configure.ps1
```

**Linux/macOS (Bash):**
```bash
./configure.sh
```

Scripts support custom download URL and output directory:
```bash
# Custom URL and directory
./configure.sh "https://custom-url/ctp-api.zip" "./custom_res" --force

# Windows PowerShell
.\configure.ps1 -Url "https://custom-url/ctp-api.zip" -OutDir ".\custom_res" -Force
```

## Build and Installation

### Basic Build
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Installation
```bash
cmake --install . --prefix /path/to/install
```

### Build Options
- `BUILD_EXAMPLES`: Build example programs (default: OFF)

## Usage in External Projects

### Method 1: Using find_package (Recommended)

In your project's CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.19.2)
project(MyTradingApp)

# Find CTP library
find_package(ctp REQUIRED)

# Or specify specific components
# find_package(ctp REQUIRED COMPONENTS trader md)

# Create your executable
add_executable(my_app main.cpp)

# Link CTP library
target_link_libraries(my_app PRIVATE ctp::ctp)

# Or link specific components
# target_link_libraries(my_app PRIVATE ctp::trader ctp::md)
```

### Method 2: Using FindCTP Module

Copy `cmake/FindCTP.cmake` to your project's cmake module directory, then:

```cmake
cmake_minimum_required(VERSION 3.19.2)
project(MyTradingApp)

# Add cmake module path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find CTP
find_package(CTP REQUIRED COMPONENTS trader md)

add_executable(my_app main.cpp)

# Link CTP
target_link_libraries(my_app PRIVATE CTP::ctp)
```

### Method 3: Setting CTP_ROOT Environment Variable

If CTP is not in the standard location, you can set environment variables:

```bash
# Linux/macOS
export CTP_ROOT=/path/to/ctp/root
cmake ..

# Windows
set CTP_ROOT=C:\path\to\ctp\root
cmake ..

# Or set directly in CMake
cmake -DCTP_ROOT=/path/to/ctp/root ..
```

## Dynamic Search Functionality

This project implements intelligent CTP API directory search functionality, without hardcoding specific version paths:

### Search Strategy

1. **Version Agnostic**: Automatically searches all version folders containing `traderapi` under the `res/` directory
2. **Platform Detection**: Automatically selects the corresponding API directory based on the current build platform
3. **Architecture Matching**: 
   - Windows 64-bit: Searches for directories containing both `64` and `windows`
   - Windows 32-bit: Searches for directories containing `windows` but not `64`
   - Linux 64-bit: Searches for directories containing `linux`
4. **Header File Validation**: Ensures that found directories contain the necessary header files (`ThostFtdcTraderApi.h`, `ThostFtdcMdApi.h`)

### Search Path Priority

1. `${CTP_ROOT}` (CMake variable)
2. `$ENV{CTP_ROOT}` (Environment variable)
3. `${CMAKE_CURRENT_SOURCE_DIR}` (Current source directory)
4. `${CMAKE_SOURCE_DIR}` (Project root directory)

### Example Directory Structure

The system can automatically recognize any of the following structures:

```
res/
├── v6.7.11_20250714_traderapi/           # Any version number
│   ├── v6.7.11_20250617_winApi/
│   │   └── traderapi/
│   │       ├── 20250617_traderapi_se_windows/      # 32-bit auto-detection
│   │       └── 20250617_traderapi64_se_windows/    # 64-bit auto-detection
│   └── v6.7.11_20250617_api_traderapi_linux64/
│       └── v6.7.11_20250617_api/
│           └── v6.7.11_20250617_api_traderapi_se_linux64/  # Linux auto-detection
├── v6.8.0_20260101_traderapi/            # New versions can also be automatically recognized
│   └── ...
└── any_custom_name_traderapi/            # Custom names are also supported
    └── ...
```

## Available CMake Targets

- `ctp::ctp` - Complete CTP interface (includes trader and md)
- `ctp::trader` - CTP trading interface
- `ctp::md` - CTP market data interface

## Example Code

### Basic Usage Example

```cpp
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcMdApi.h"
#include <iostream>

int main() {
    // Create trading API instance
    CThostFtdcTraderApi* pTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
    
    // Create market data API instance
    CThostFtdcMdApi* pMdApi = CThostFtdcMdApi::CreateFtdcMdApi();
    
    std::cout << "CTP API initialized successfully!" << std::endl;
    
    // Release resources
    pTraderApi->Release();
    pMdApi->Release();
    
    return 0;
}
```

## API Documentation

CTP API contains the following main header files:

- `ThostFtdcTraderApi.h` - Trading API interface
- `ThostFtdcMdApi.h` - Market data API interface
- `ThostFtdcUserApiStruct.h` - User API structure definitions
- `ThostFtdcUserApiDataType.h` - User API data type definitions

## Version Information

- CTP API Version: 6.7.11
- Release Date: 2025-07-14
- Supported Operating Systems: Windows (32/64-bit), Linux (64-bit)

## Troubleshooting

### Common Issues

1. **CTP library not found**
   - Ensure CTP API files are in the correct directory structure
   - Set `CTP_ROOT` environment variable or CMake variable

2. **Linking errors**
   - Ensure target platform and architecture match
   - On Windows, ensure .dll files are in PATH or in the same directory as the executable

3. **Header files not found**
   - Check if include directories are set correctly
   - Ensure header files exist at the specified location

### Debug Information

The following information will be displayed during build:
```
-- CTP Configuration:
--   Platform: winApi (or linux)
--   Architecture: 64 (or 32)
--   API Directory: /path/to/ctp/api
--   Include Directory: /path/to/headers
--   Trader Library: /path/to/trader/lib
--   Market Data Library: /path/to/md/lib
```

## License

Please comply with the license terms and conditions of the CTP API.