cmake_minimum_required(VERSION 2.8)

set(PRJ_NAME HelloWorld)

set(CMAKE_VERBOSE_MAKEFILE on)

option(USE_FILE "Set to OFF to disable file logging" OFF)
message(STATUS "USE_FILE is set to value: ${USE_FILE}")

option(USE_CONSOLE "Set to OFF to disable console logging" OFF)
message(STATUS "USE_CONSOLE is set to value: ${USE_CONSOLE}")

if(USE_FILE)
  add_definitions(-DUSE_FILE)
endif(USE_FILE)
if(USE_CONSOLE)
  add_definitions(-DUSE_CONSOLE)
endif(USE_CONSOLE)

set(MAX_LOG_LEVEL
    "DEBUG"
    CACHE STRING "maximum log level")
message(STATUS "MAX_LOG_LEVEL is set to value: ${MAX_LOG_LEVEL}")
add_definitions(-DCOMMONAPI_LOGLEVEL=COMMONAPI_LOGLEVEL_${MAX_LOG_LEVEL})

if(MSVC)
  # Visual C++ is not always sure whether he is really C++
  set(CMAKE_C_FLAGS
      "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS /EHsc /wd\\\"4503\\\"")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS /wd\\\"4503\\\"")
else()
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -pthread -Wall -O0 -std=c++11 -D_GLIBCXX_USE_NANOSLEEP -DLINUX"
  )
endif()

message(STATUS "Compiler options: ${CMAKE_CXX_FLAGS}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      "Debug"
      CACHE STRING "Choose the type of build, options are: Debug Release."
            FORCE)
endif(NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# option(USE_INSTALLED_COMMONAPI
#        "Set to OFF to use the local (build tree) version of CommonAPI" ON)
message(
  STATUS "USE_INSTALLED_COMMONAPI is set to value: ${USE_INSTALLED_COMMONAPI}")

if("${USE_INSTALLED_COMMONAPI}" STREQUAL "ON")
  find_package(CommonAPI 3.2.0 REQUIRED CONFIG NO_CMAKE_PACKAGE_REGISTRY)
  find_package(CommonAPI-DBus 3.2.0 REQUIRED CONFIG NO_CMAKE_PACKAGE_REGISTRY)
else()
  find_package(CommonAPI 3.2.0 REQUIRED CONFIG NO_SYSTEM_ENVIRONMENT_PATH
               NO_CMAKE_SYSTEM_PATH)
#   find_package(CommonAPI-DBus 3.2.0 REQUIRED CONFIG NO_SYSTEM_ENVIRONMENT_PATH
#                NO_CMAKE_SYSTEM_PATH)
endif()

message(STATUS "CommonAPI_CONSIDERED_CONFIGS: ${CommonAPI_CONSIDERED_CONFIGS}")
message(STATUS "COMMONAPI_INCLUDE_DIRS: ${COMMONAPI_INCLUDE_DIRS}")
message(
  STATUS
    "CommonAPI-DBus_CONSIDERED_CONFIGS: ${CommonAPI-DBus_CONSIDERED_CONFIGS}")
message(STATUS "COMMONAPI_DBUS_INCLUDE_DIRS: ${COMMONAPI_DBUS_INCLUDE_DIRS}")

# CommonAPI
include(FindPkgConfig)
# ##############################################################################
# find DBus by using the 'pkg-config' tool
if(MSVC)
  # Not beautiful, but it works
  if(DBus_DIR)
    if(DBus_BUILD_DIR)
      set(DBus_INCLUDE_DIRS "${DBus_DIR};${DBus_BUILD_DIR};")
    else()
      message(FATAL_ERROR "DBus_BUILD_DIR not set! Cannot continue.")
    endif()
  else()
    message(FATAL_ERROR "DBus_DIR not set! Cannot continue.")
  endif()
else()
  pkg_check_modules(DBus REQUIRED dbus-1>=1.4)
endif()

# SOME/IP
find_package(CommonAPI-SomeIP 3.2.0 REQUIRED)
find_package(vsomeip3 3.1.0 REQUIRED)

# Source Files
set(PRJ_SRC_PATH src)
set(PRJ_SRC_GEN_PATH src-gen)
set(PRJ_SRC_GEN_COMMONAPI_PATH ${PRJ_SRC_GEN_PATH}/core/v0/commonapi/examples)
# set(PRJ_SRC_GEN_COMMONAPI_DBUS_PATH
#     ${PRJ_SRC_GEN_PATH}/dbus/v0/commonapi/examples)
set(PRJ_SRC_GEN_COMMONAPI_SOMEIP_PATH
    ${PRJ_SRC_GEN_PATH}/someip/v0/commonapi/examples)

set(PRJ_NAME_CLIENT ${PRJ_NAME}Client)
set(PRJ_NAME_SERVICE ${PRJ_NAME}Service)

# Application
file(GLOB PRJ_PROXY_GEN_SRCS ${PRJ_SRC_GEN_COMMONAPI_PATH}/*Proxy.cpp)
file(GLOB PRJ_STUB_GEN_SRCS ${PRJ_SRC_GEN_COMMONAPI_PATH}/*Stub*.cpp)
file(GLOB PRJ_STUB_IMPL_SRCS ${PRJ_SRC_COMMONAPI_PATH}/*Stub*.cpp)
set(PRJ_CLIENT_SRCS ${PRJ_SRC_PATH}/${PRJ_NAME_CLIENT}.cpp
                    ${PRJ_PROXY_GEN_SRCS})
set(PRJ_SERVICE_SRCS
    ${PRJ_SRC_PATH}/${PRJ_NAME_SERVICE}.cpp
    ${PRJ_SRC_PATH}/${PRJ_NAME}StubImpl.cpp ${PRJ_STUB_GEN_SRCS}
    ${PRJ_STUB_IMPL_SRCS})

# Boost
find_package(
  Boost 1.54
  COMPONENTS system thread log
  REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

# DBus library
# file(GLOB PRJ_DBUS_LIB_SRCS ${PRJ_SRC_GEN_COMMONAPI_DBUS_PATH}/*cpp)

# SOME/IP library
file(GLOB PRJ_SOMEIP_LIB_SRCS ${PRJ_SRC_GEN_COMMONAPI_SOMEIP_PATH}/*cpp)

# Paths
option(USE_INSTALLED_DBUS
       "Set to OFF to use the local (patched) version of dbus" ON)
message(STATUS "USE_INSTALLED_DBUS is set to value: ${USE_INSTALLED_DBUS}")

include_directories(
  src-gen/core
  # src-gen/dbus
  src-gen/someip
  ${CommonAPI_INCLUDE_DIRS}
  # ${COMMONAPI_DBUS_INCLUDE_DIRS}
  ${CommonAPI-SomeIP_INCLUDE_DIRS}
  # ${DBus_INCLUDE_DIRS}
  ${vsomeip3_INCLUDE_DIRS})

# if("${USE_INSTALLED_DBUS}" STREQUAL "ON")
#   link_directories(
#     ${COMMONAPI_LIBDIR} ${COMMONAPI_DBUS_LIBDIR}
#     ${COMMONAPI_SOMEIP_CMAKE_DIR}/build ${DBus_LIBRARY_DIRS}
#     ${Boost_LIBRARY_DIR})
# else()
#   link_directories(
#     ${COMMONAPI_LIBDIR} ${COMMONAPI_DBUS_LIBDIR}
#     ${COMMONAPI_SOMEIP_CMAKE_DIR}/build ${DBus_INCLUDE_DIRS}/dbus/.libs
#     ${Boost_LIBRARY_DIR})
# endif()

if(MSVC)
  set(LINK_LIBRARIES CommonAPI)
else()
  set(LINK_LIBRARIES -Wl,--as-needed ${CommonAPI_LIBRARIES} ${CommonAPI-SomeIP_LIBRARIES} ${Boost_LIBRARIES} ${vsomeip3_LIBRARIES})
endif()

# Build Client
add_executable(${PRJ_NAME_CLIENT} ${PRJ_CLIENT_SRCS})
target_link_libraries(${PRJ_NAME_CLIENT} ${LINK_LIBRARIES})

# Build service
add_executable(${PRJ_NAME_SERVICE} ${PRJ_SERVICE_SRCS})
target_link_libraries(${PRJ_NAME_SERVICE} ${LINK_LIBRARIES})

# # Build DBus library
# add_library(${PRJ_NAME}-dbus SHARED ${PRJ_DBUS_LIB_SRCS})
# target_link_libraries(${PRJ_NAME}-dbus CommonAPI-DBus)

# Build SOME/IP library
add_library(${PRJ_NAME}-someip SHARED ${PRJ_SOMEIP_LIB_SRCS})
target_link_libraries(${PRJ_NAME}-someip ${CommonAPI-SomeIP_LIBRARIES})
