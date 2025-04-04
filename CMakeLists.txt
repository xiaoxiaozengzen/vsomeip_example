cmake_minimum_required(VERSION 3.23)
project(vsomeip_example)

find_package(vsomeip3 REQUIRED)

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

# add_subdirectory(commonapi_example)