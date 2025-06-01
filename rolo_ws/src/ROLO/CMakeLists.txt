cmake_minimum_required(VERSION 3.0.2)
project(rolo)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-O3 -Wall -g")
set(CMAKE_CXX_STANDARD 14)

if(DEFINED ENV{ROS_VERSION})
  set(ROS_VERSION $ENV{ROS_VERSION})
endif()

find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  nav_msgs
  cv_bridge
  pcl_ros
  roscpp
  rospy
  std_msgs
  sensor_msgs
  tf
  visualization_msgs
  message_generation
)

find_package(PCL REQUIRED QUIET)
find_package(OpenCV REQUIRED QUIET)
find_package(GTSAM REQUIRED QUIET)
find_package(Eigen3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS timer)
find_package(OpenMP)
if (OPENMP_FOUND)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# pmc (Parallel Maximum Clique) - Offline mode
set(PMC_SRC_DIR "${CMAKE_SOURCE_DIR}/../pmc-src")
set(PMC_BUILD_DIR "${CMAKE_BINARY_DIR}/pmc-build")

if(EXISTS "${PMC_SRC_DIR}/CMakeLists.txt")
  message(STATUS "Using pre-downloaded PMC source at ${PMC_SRC_DIR}")
  add_subdirectory("${PMC_SRC_DIR}" "${PMC_BUILD_DIR}")
else()
  message(FATAL_ERROR "PMC source not found at ${PMC_SRC_DIR}. Please manually place it there.")
endif()

add_message_files(
  FILES
  CloudInfoStamp.msg
)

generate_messages(
  DEPENDENCIES
  geometry_msgs nav_msgs std_msgs sensor_msgs
)

catkin_package(
  INCLUDE_DIRS include
  DEPENDS PCL GTSAM
  CATKIN_DEPENDS
  std_msgs
  nav_msgs
  geometry_msgs
  sensor_msgs
  message_runtime
  visualization_msgs
)

catkin_install_python(PROGRAMS test/translation_plot.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${PCL_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${GTSAM_INCLUDE_DIR}
)

link_directories(
  include
  ${PCL_LIBRARY_DIRS}
  ${OpenCV_LIBRARY_DIRS}
  ${GTSAM_LIBRARY_DIRS}
)

# Add icp lib
add_library(rot_gicp SHARED
  src/rot_gicp/gicp/lsq_registration.cpp
  src/rot_gicp/gicp/rot_vgicp.cpp
)
target_link_libraries(rot_gicp
  ${PCL_LIBRARIES}
)
if (OPENMP_FOUND)
  if (TARGET OpenMP::OpenMP_CXX)
    target_link_libraries(rot_gicp OpenMP::OpenMP_CXX)
  endif ()
endif ()
target_include_directories(rot_gicp PUBLIC
  include
  ${PCL_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIR}
)

add_executable(${PROJECT_NAME}_imageProjection src/imageProjection.cpp)
add_dependencies(${PROJECT_NAME}_imageProjection ${PROJECT_NAME}_generate_messages_cpp)
target_link_libraries(${PROJECT_NAME}_imageProjection ${catkin_LIBRARIES} ${PCL_LIBRARIES} ${OpenCV_LIBRARIES})

add_executable(${PROJECT_NAME}_featureExtraction src/featureExtraction.cpp)
add_dependencies(${PROJECT_NAME}_featureExtraction ${PROJECT_NAME}_generate_messages_cpp)
target_link_libraries(${PROJECT_NAME}_featureExtraction ${catkin_LIBRARIES} ${PCL_LIBRARIES} ${OpenCV_LIBRARIES})

add_executable(${PROJECT_NAME}_lidarOdometry src/lidarOdometry.cpp)
add_dependencies(${PROJECT_NAME}_lidarOdometry ${PROJECT_NAME}_generate_messages_cpp rot_gicp)
target_compile_options(${PROJECT_NAME}_lidarOdometry PRIVATE ${OpenMP_CXX_FLAGS})
target_link_libraries(${PROJECT_NAME}_lidarOdometry ${catkin_LIBRARIES} ${PCL_LIBRARIES} ${OpenCV_LIBRARIES} ${OpenMP_CXX_FLAGS} ${EIGEN_LIBRARIES} gtsam rot_gicp)

add_executable(${PROJECT_NAME}_backMapping src/backMapping.cpp)
add_dependencies(${PROJECT_NAME}_backMapping ${PROJECT_NAME}_generate_messages_cpp)
target_compile_options(${PROJECT_NAME}_backMapping PRIVATE ${OpenMP_CXX_FLAGS})
target_link_libraries(${PROJECT_NAME}_backMapping ${catkin_LIBRARIES} ${PCL_LIBRARIES} ${OpenCV_LIBRARIES} ${OpenMP_CXX_FLAGS} ${EIGEN_LIBRARIES} gtsam)

# Test executable
add_executable(rotation_test test/rotation_test.cpp)
target_link_libraries(rotation_test ${catkin_LIBRARIES} ${PCL_LIBRARIES} ${OpenCV_LIBRARIES})

