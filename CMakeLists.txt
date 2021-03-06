cmake_minimum_required(VERSION 2.8.3)
project(controller_common)

### ROS Dependencies ###
# Find the RTT-ROS package (this transitively includes the Orocos CMake macros)
find_package(catkin REQUIRED COMPONENTS
  rtt_ros
  cartesian_trajectory_msgs
  eigen_conversions
  kdl_conversions cmake_modules
  )

find_package(Eigen REQUIRED)
find_package(orocos_kdl REQUIRED)

link_directories(${orocos_kdl_LIBRARY_DIRS})

include_directories(${catkin_INCLUDE_DIRS} ${Eigen_INCLUDE_DIRS})

### Orocos Dependencies ###
# Note that orocos_use_package() does not need to be called for any dependency
# listed in the package.xml file

include_directories(${USE_OROCOS_INCLUDE_DIRS} include)
ADD_DEFINITIONS(-DEIGEN_RUNTIME_NO_MALLOC)
### Orocos Targets ###

orocos_component(controller_common src/controller_common.cpp src/cartesian_interpolator.cpp src/cartesian_trajectory_action.cpp src/joint_limit_avoidance.cpp src/robot_mass_matrix.cpp src/mass_test.cpp src/cartesian_impedance_interpolator.cpp src/cartesian_impedance_action.cpp src/pose_transform.cpp src/tf_publisher.cpp src/torque_publisher.cpp)
target_link_libraries(controller_common ${catkin_LIBRARIES} ${USE_OROCOS_LIBRARIES} ${orocos_kdl_LIBRARIES})
add_dependencies(controller_common ${PROJECT_NAME}_gencpp)

### Orocos Package Exports and Install Targets ###

# Export package information (replaces catkin_package() macro) 
orocos_generate_package(
  INCLUDE_DIRS include
  DEPENDS rtt_ros geometry_msgs std_msgs message_runtime rtt_roscomm
)

orocos_install_headers(
  DIRECTORY include/${PROJECT_NAME}/
)
