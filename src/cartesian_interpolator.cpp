/*
 * CartesianInterpolator.cpp
 *
 *  Created on: 27 lut 2014
 *      Author: konradb3
 */

#include "cartesian_interpolator.h"

#include "rtt_rosclock/rtt_rosclock.h"
#include <Eigen/Geometry>

CartesianInterpolator::CartesianInterpolator(const std::string& name) :
		RTT::TaskContext(name), trajectory_ptr_(0) {

	this->ports()->addPort("CartesianPosition", port_cartesian_position_);
	this->ports()->addPort("CartesianPositionCommand", port_cartesian_command_);
	this->ports()->addPort("CartesianTrajectoryCommand", port_trajectory_);
}

CartesianInterpolator::~CartesianInterpolator() {
}

bool CartesianInterpolator::configureHook() {
	return true;
}

bool CartesianInterpolator::startHook() {
	if(port_cartesian_position_.read(setpoint_) == RTT::NoData) {
		return false;
	}

	//std::cout << "[ " << setpoint_.position.x << ", " << setpoint_.position.y << ", " << setpoint_.position.z << ", " << setpoint_.orientation.w << ", " << setpoint_.orientation.x << ", " << setpoint_.orientation.y << ", " << setpoint_.orientation.z << std::endl;

	return true;
}

void CartesianInterpolator::updateHook() {

	if (port_trajectory_.read(trajectory_) == RTT::NewData) {
		trajectory_ptr_ = 0;
		old_point_ = setpoint_;
	}

	ros::Time now = rtt_rosclock::host_rt_now();
	if (trajectory_ && (trajectory_->header.stamp < now)) {
		for (; trajectory_ptr_ < trajectory_->points.size();
				trajectory_ptr_++) {

			ros::Time trj_time = trajectory_->header.stamp
					+ trajectory_->points[trajectory_ptr_].time_from_start;
			if (trj_time > now) {
				break;
			}
		}

		if(trajectory_ptr_ < trajectory_->points.size()) {
			if(trajectory_ptr_ == 0) {
				controller_common::CartesianTrajectoryPoint p0;
				p0.time_from_start.fromSec(0.0);
				p0.pose = old_point_;
				setpoint_ = interpolate(p0, trajectory_->points[trajectory_ptr_], now);
			} else {
				setpoint_ = interpolate(trajectory_->points[trajectory_ptr_-1], trajectory_->points[trajectory_ptr_], now);
			}
			//std::cout << "[ " << setpoint_.position.x << ", " << setpoint_.position.y << ", " << setpoint_.position.z << ", " << setpoint_.orientation.w << ", " << setpoint_.orientation.x << ", " << setpoint_.orientation.y << ", " << setpoint_.orientation.z << "]" << std::endl;
		}
	}
	port_cartesian_command_.write(setpoint_);
}

geometry_msgs::Pose CartesianInterpolator::interpolate(const controller_common::CartesianTrajectoryPoint& p0, const controller_common::CartesianTrajectoryPoint& p1, ros::Time t) {
	geometry_msgs::Pose pose;

	ros::Time t0 = trajectory_->header.stamp + p0.time_from_start;
	ros::Time t1 = trajectory_->header.stamp + p1.time_from_start;

	pose.position.x = interpolate(p0.pose.position.x, p1.pose.position.x, t0.toSec(), t1.toSec(), t.toSec());
	pose.position.y = interpolate(p0.pose.position.y, p1.pose.position.y, t0.toSec(), t1.toSec(), t.toSec());
	pose.position.z = interpolate(p0.pose.position.z, p1.pose.position.z, t0.toSec(), t1.toSec(), t.toSec());

	Eigen::Quaterniond q0(p0.pose.orientation.w, p0.pose.orientation.x, p0.pose.orientation.y, p0.pose.orientation.z);
	Eigen::Quaterniond q1(p1.pose.orientation.w, p1.pose.orientation.x, p1.pose.orientation.y, p1.pose.orientation.z);

	double a = interpolate(0.0, 1.0, t0.toSec(), t1.toSec(), t.toSec());
	Eigen::Quaterniond q = q0.slerp(a, q1);
	pose.orientation.w = q.w();
	pose.orientation.x = q.x();
	pose.orientation.y = q.y();
	pose.orientation.z = q.z();

	return pose;
}

double CartesianInterpolator::interpolate(double p0, double p1, double t0, double t1, double t) {
	return (p0 + (p1 - p0) * (t - t0)/(t1 - t0));
}