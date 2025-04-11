cmake_minimum_required(VERSION 3.23)
project(vsomeip_example)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(vsomeip3 REQUIRED)
message(STATUS "vsomeip3_INCLUDE_DIRS: ${vsomeip3_INCLUDE_DIRS}")
message(STATUS "vsomeip3_LIBRARIES: ${vsomeip3_LIBRARIES}")
message(STATUS "vsomeip3_VERSION: ${vsomeip3_VERSION}")
message(STATUS "vsomeip3_VERSION_STRING: ${vsomeip3_VERSION_STRING}")
message(STATUS "vsomeip3_VERSION_MAJOR: ${vsomeip3_VERSION_MAJOR}")
message(STATUS "vsomeip3_VERSION_MINOR: ${vsomeip3_VERSION_MINOR}")
message(STATUS "vsomeip3_VERSION_PATCH: ${vsomeip3_VERSION_PATCH}")

add_executable(request src/request.cpp)
target_include_directories(
  request PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${vsomeip3_INCLUDE_DIRS}
)
target_link_libraries(
  request PUBLIC
  ${vsomeip3_LIBRARIES}
  pthread
)

add_executable(response src/response.cpp)
target_include_directories(
  response PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${vsomeip3_INCLUDE_DIRS}
)
target_link_libraries(
  response PUBLIC
  ${vsomeip3_LIBRARIES}
  pthread
)

add_executable(publisher src/publisher.cpp)
target_include_directories(
  publisher PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${vsomeip3_INCLUDE_DIRS}
)
target_link_libraries(
  publisher PUBLIC
  ${vsomeip3_LIBRARIES}
  pthread
)

add_executable(subscriber src/subscriber.cpp)
target_include_directories(
  subscriber PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${vsomeip3_INCLUDE_DIRS}
)
target_link_libraries(
  subscriber PUBLIC
  ${vsomeip3_LIBRARIES}
  pthread
)

add_executable(routing src/routing.cpp)
target_include_directories(
  routing PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${vsomeip3_INCLUDE_DIRS}
)
target_link_libraries(
  routing PUBLIC
  ${vsomeip3_LIBRARIES}
  pthread
)

add_executable(field_server src/field_server.cpp)
target_include_directories(
  field_server PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${vsomeip3_INCLUDE_DIRS}
)
target_link_libraries(
  field_server PUBLIC
  ${vsomeip3_LIBRARIES}
  pthread
)

add_executable(field_client src/field_client.cpp)
target_include_directories(
  field_client PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${vsomeip3_INCLUDE_DIRS}
)
target_link_libraries(
  field_client PUBLIC
  ${vsomeip3_LIBRARIES}
  pthread
)

if(DEFINED COMMONAPI_USING)
  add_subdirectory(commonapi_example)
endif()