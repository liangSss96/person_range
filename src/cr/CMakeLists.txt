cmake_minimum_required(VERSION 3.0.2)
project(cr)

# opencv
# set(OpenCV_DIR /home/ls/Downloads/opencv-4.1.0/build/)
# find_package(OpenCV REQUIRED)
find_package(OpenCV REQUIRED)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  roslib
  std_msgs
  cv_bridge
  image_transport
  sensor_msgs
  enum
  common_utils
  base_structure
  wind_zmq
)

# cuda and tensorrt
find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})

# link_directories(/usr/local/cuda-11/lib64)
catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS roscpp roslib std_msgs cv_bridge image_transport sensor_msgs enum common_utils base_structure wind_zmq
  DEPENDS OpenCV
)

include_directories(
  ./include
  ${catkin_INCLUDE_DIRS}
)

# tld_detector依赖tensorrt和cuda，无法加入catkin_package中，需要单独编译
include_directories(./tld_detector/include)
add_subdirectory(./tld_detector)

aux_source_directory(./src SRC)
add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME}
  tld_detector nvinfer cudart
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
