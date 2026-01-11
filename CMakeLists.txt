cmake_minimum_required(VERSION 3.8)
project(calibration_imu)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(rosbag2_cpp REQUIRED)
find_package(rosbag2_storage REQUIRED) # For storage options if needed
find_package(ifopt REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread)
find_package(Eigen3 REQUIRED NO_MODULE)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS
    Widgets
    DataVisualization
    Charts
)

include_directories(include)

# Magnetometer Executable
add_executable(magnetometer
  src/magnetometer/main.cpp
  src/magnetometer/gui/fmain.ui
  src/magnetometer/gui/fmain.cpp
  src/magnetometer/data/data_interface.cpp
  src/common/geometry/ellipsoid.cpp
  src/magnetometer/graph/graph.cpp
  src/common/calibration/variables_center.cpp
  src/magnetometer/calibration/variables_rotation.cpp
  src/common/calibration/variables_radius.cpp
  src/magnetometer/calibration/cost_term.cpp
  src/magnetometer/calibration/calibrator.cpp
)

target_include_directories(magnetometer PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>)

target_link_libraries(magnetometer
  rclcpp::rclcpp
  ${sensor_msgs_TARGETS}
  rosbag2_cpp::rosbag2_cpp
  rosbag2_storage::rosbag2_storage
  ifopt::ifopt_ipopt
  Qt5::Widgets
  Qt5::DataVisualization
  Boost::thread
  Eigen3::Eigen
)

# Accelerometer Executable
add_executable(accelerometer
  src/accelerometer/main.cpp
  src/accelerometer/gui/fmain.ui
  src/accelerometer/gui/fmain.cpp
  src/accelerometer/data/data_interface.cpp
  src/accelerometer/graph/graph.cpp
  src/common/geometry/ellipsoid.cpp
  src/common/calibration/variables_center.cpp
  src/common/calibration/variables_radius.cpp
  src/accelerometer/calibration/cost_term.cpp
  src/accelerometer/calibration/calibrator.cpp
)

target_include_directories(accelerometer PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>)

target_link_libraries(accelerometer
  rclcpp::rclcpp
  ${sensor_msgs_TARGETS}
  rosbag2_cpp::rosbag2_cpp
  rosbag2_storage::rosbag2_storage
  ifopt::ifopt_ipopt
  Qt5::Widgets
  Qt5::Charts
  Boost::thread
  Eigen3::Eigen
)

# Install
install(TARGETS
  magnetometer
  accelerometer
  DESTINATION lib/${PROJECT_NAME}
)

# Install specific directories if needed, e.g. launch or params not present here but good practice
# install(DIRECTORY launch
#   DESTINATION share/${PROJECT_NAME}
# )

ament_package()
