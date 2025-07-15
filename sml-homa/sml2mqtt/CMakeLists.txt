cmake_minimum_required(VERSION 3.10)

project(SML2MQTT
    LANGUAGES C CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # CMAKE_INSTALL_PREFIX defaults to /usr/local, but it should go to /usr
    set(CMAKE_INSTALL_PREFIX /usr/local)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# build types: None, Debug, Release, RelWithDebInfo, MinSizeRel
set(CMAKE_BUILD_TYPE Release)

# parts to build
option(OPTION_WITH_SYSTEMD "systemd support" ON)

# directories
include(GNUInstallDirs)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# dependencies
#find_package(LIBSML REQUIRED)
#find_package(LIBMOSQUITTOPP REQUIRED)
#find_package(yaml-cpp REQUIRED)
#message(STATUS "yaml-cpp_FOUND: ${yaml-cpp_FOUND}")
#message(STATUS "YAML_CPP_INCLUDE_DIRS: ${YAML_CPP_INCLUDE_DIRS}" get_target_property(yaml-cpp))
#message(STATUS "YAML_CPP_LIBRARIES: ${yaml-cpp_LIBRARIES}")
# find_package does not work with alpine linux, so we use pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSML REQUIRED IMPORTED_TARGET sml)
pkg_check_modules(LIBMOSQUITTOPP REQUIRED IMPORTED_TARGET libmosquittopp)

# install
install(
    FILES README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR})

# sub directories
add_subdirectory(src)
