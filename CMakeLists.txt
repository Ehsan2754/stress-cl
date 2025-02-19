cmake_minimum_required(VERSION 3.10)
project(stress-cl)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Read version from version.txt
file(READ "${CMAKE_SOURCE_DIR}/version.txt" VERSION_CONTENTS)
string(STRIP "${VERSION_CONTENTS}" PROJECT_VERSION)

# Find OpenCL package
find_package(OpenCL REQUIRED)

# Add the executable
add_executable(stress-cl stress-cl.cpp)

# Link OpenCL library
target_link_libraries(stress-cl OpenCL::OpenCL)

# Define version macro
add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")

# Install version.txt
install(TARGETS stress-cl DESTINATION bin)

# CPack configuration
set(CPACK_PACKAGE_NAME "stress-cl")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A stress testing tool for OpenCL devices(GPU / CPU / NPU).")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_GENERATOR "DEB;STGZ")
set(CPACK_PACKAGE_CONTACT "Ehsan Shaghaei (contact@byehsan.com)")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Ehsan Shaghaei")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
