cmake_minimum_required(VERSION 3.5)

project(enum)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

set(DEV_MODE FALSE)


# ros
find_package(catkin REQUIRED COMPONENTS
    roscpp
    roslib
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME} 
  CATKIN_DEPENDS roscpp roslib
)

include_directories( ${catkin_INCLUDE_DIRS} )
include_directories(./include)
aux_source_directory(./src SRC)
add_library(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME}
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



