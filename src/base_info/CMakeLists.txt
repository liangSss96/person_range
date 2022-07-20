cmake_minimum_required(VERSION 3.0.2)
project(base_info)

find_package(OpenCV REQUIRED)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  roslib
  std_msgs
  cv_bridge
  image_transport
  sensor_msgs
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS roscpp roslib rospy std_msgs cv_bridge image_transport sensor_msgs
  DEPENDS OpenCV
)

include_directories(
  ./include
  ${catkin_INCLUDE_DIRS}
)

aux_source_directory(./src SRC)
add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(
  ${PROJECT_NAME}
  ${OpenCV_LIBS}
  ${catkin_LIBRARIES}
)

add_dependencies(${PROJECT_NAME}
  ${catkin_EXPORTED_TARGETS}
)

install(TARGETS
  ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)