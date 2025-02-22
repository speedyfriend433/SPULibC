cmake_minimum_required(VERSION 3.10)
project(spulibc VERSION 1.0)

# Set iOS platform
set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0")
set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "12.0")
set(CMAKE_OSX_ARCHITECTURES "arm64")
set(CMAKE_OSX_SYSROOT "iphoneos")
set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos")
set(CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")

# Enable languages
enable_language(C)
enable_language(CXX)
enable_language(ASM)

# Set C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Set output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set iOS SDK path
set(IOS_SDK_PATH "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk")
set(IOS_TOOLCHAIN_PATH "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain")

# Set iOS specific flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch arm64 -fembed-bitcode -fobjc-arc -isysroot ${IOS_SDK_PATH} -I${IOS_SDK_PATH}/usr/include -I${IOS_TOOLCHAIN_PATH}/usr/include -Wno-nullability-completeness")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch arm64 -fembed-bitcode -fobjc-arc -isysroot ${IOS_SDK_PATH} -I${IOS_SDK_PATH}/usr/include -I${IOS_TOOLCHAIN_PATH}/usr/include -Wno-nullability-completeness")
set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS} -arch arm64 -fembed-bitcode -fobjc-arc -isysroot ${IOS_SDK_PATH} -I${IOS_SDK_PATH}/usr/include -I${IOS_TOOLCHAIN_PATH}/usr/include -Wno-nullability-completeness")

# Set iOS framework settings
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "@executable_path/Frameworks;@loader_path/Frameworks")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Library settings
set(CMAKE_SHARED_LIBRARY_SUFFIX "")
set(CMAKE_SHARED_MODULE_SUFFIX "")

# Find required frameworks
set(CMAKE_FRAMEWORK_PATH ${IOS_SDK_PATH}/System/Library/Frameworks)
find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
find_library(UIKIT_FRAMEWORK UIKit REQUIRED)

# Framework settings
set(CMAKE_FRAMEWORK ON)
set(CMAKE_FRAMEWORK_PATH ${CMAKE_BINARY_DIR})

# Define source files
set(SPULIBC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/spulibc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dyld_bypass.m
    ${CMAKE_CURRENT_SOURCE_DIR}/include/dyld_patch_platform.m
)

# Static library target
add_library(spulibc_static STATIC ${SPULIBC_SOURCES})
set_target_properties(spulibc_static PROPERTIES
    OUTPUT_NAME spulibc
    CLEAN_DIRECT_OUTPUT 1
    PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/spulibc.h"
)

# Dynamic library/framework target
add_library(spulibc SHARED ${SPULIBC_SOURCES})
set_target_properties(spulibc PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION A
    MACOSX_FRAMEWORK_IDENTIFIER com.spulibc
    MACOSX_FRAMEWORK_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
    PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/spulibc.h"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ""
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/spulibc.entitlements"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.spulibc"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CLEAN_DIRECT_OUTPUT 1
)

# Common settings for both targets
foreach(TARGET spulibc spulibc_static)
    target_link_libraries(${TARGET}
        ${FOUNDATION_FRAMEWORK}
        ${UIKIT_FRAMEWORK}
    )

    target_include_directories(${TARGET} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${IOS_SDK_PATH}/usr/include
        ${IOS_TOOLCHAIN_PATH}/usr/include
    )
endforeach()