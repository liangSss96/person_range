cmake_minimum_required(VERSION 3.5)

project(tld_detector)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

# opencv
find_package(OpenCV REQUIRED)

# cuda and tensorrt
find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})
link_directories(/usr/local/cuda/lib64)
cuda_add_library(yololayer SHARED src/yololayer.cu)
target_link_libraries(yololayer
  nvinfer
  cudart
)

include_directories(./include)
aux_source_directory(src SRC)
add_library(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME}
  ${OpenCV_LIBS}
  yololayer nvinfer cudart
)
