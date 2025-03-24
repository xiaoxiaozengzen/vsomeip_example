cmake_minimum_required(VERSION 3.23)
project(vsomeip_example)

find_package(vsomeip3 REQUIRED)
find_package(CommonAPI-SomeIP REQUIRED)

add_executable(request src/request.cpp)
target_include_directories(
  request PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${vsomeip3_INCLUDE_DIRS}
  ${CommonAPI-SomeIP_INCLUDE_DIRS}
)
target_link_libraries(
  request PUBLIC
  ${vsomeip3_LIBRARIES}
  ${CommonAPI-SomeIP_LIBRARIES}
  pthread
)

add_executable(response src/response.cpp)
target_include_directories(
  response PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${vsomeip3_INCLUDE_DIRS}
  ${CommonAPI-SomeIP_INCLUDE_DIRS}
)
target_link_libraries(
  response PUBLIC
  ${vsomeip3_LIBRARIES}
  ${CommonAPI-SomeIP_LIBRARIES}
  pthread
)

add_executable(publisher src/publisher.cpp)
target_include_directories(
  publisher PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${vsomeip3_INCLUDE_DIRS}
  ${CommonAPI-SomeIP_INCLUDE_DIRS}
)
target_link_libraries(
  publisher PUBLIC
  ${vsomeip3_LIBRARIES}
  ${CommonAPI-SomeIP_LIBRARIES}
  pthread
)

add_executable(subscriber src/subscriber.cpp)
target_include_directories(
  subscriber PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${vsomeip3_INCLUDE_DIRS}
  ${CommonAPI-SomeIP_INCLUDE_DIRS}
)
target_link_libraries(
  subscriber PUBLIC
  ${vsomeip3_LIBRARIES}
  ${CommonAPI-SomeIP_LIBRARIES}
  pthread
)