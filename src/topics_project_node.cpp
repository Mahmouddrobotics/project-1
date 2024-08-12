#include "ros/ros.h"
#include "sensor_msgs/LaserScan.h"
#include "geometry_msgs/Twist.h"

ros::Publisher velocity_publisher;

// Constants
const float SAFETY_DISTANCE = 0.8;  // Safety distance to avoid collision
const float REVERSE_DISTANCE = 0.3; // Reverse distance when too close to an obstacle
const float REVERSE_TIME = 2.0;     // Time in seconds to reverse

void scanCallback(const sensor_msgs::LaserScan::ConstPtr& scan) {
    geometry_msgs::Twist move;

    ROS_INFO("-------------------------------------------");
    ROS_INFO("Range data at 0 deg:   %f", scan->ranges[0]);
    ROS_INFO("Range data at 15 deg:  %f", scan->ranges[15]);
    ROS_INFO("Range data at 345 deg: %f", scan->ranges[345]);
    ROS_INFO("-------------------------------------------");

    if (scan->ranges[0] > SAFETY_DISTANCE && scan->ranges[15] > SAFETY_DISTANCE && scan->ranges[345] > SAFETY_DISTANCE) {
        // Move forward (linear velocity)
        move.linear.x = 0.5;
        move.angular.z = 0.0;
    } else {
        // Stop and rotate counter-clockwise (angular velocity)
        move.linear.x = 0.0;
        move.angular.z = 0.5;

        if (scan->ranges[0] < REVERSE_DISTANCE) {
            // If too close to an obstacle, reverse
            ROS_WARN("Too close to obstacle! Reversing...");
            move.linear.x = -0.5;  // Reverse
            move.angular.z = 0.0;
            velocity_publisher.publish(move);
            ros::Duration(REVERSE_TIME).sleep();  // Reverse for a certain amount of time
        }

        // Check again if the path is clear after reversing
        if (scan->ranges[0] > SAFETY_DISTANCE && scan->ranges[15] > SAFETY_DISTANCE && scan->ranges[345] > SAFETY_DISTANCE) {
            move.linear.x = 0.5;
            move.angular.z = 0.0;
        }
    }

    velocity_publisher.publish(move); // Publish the move command
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "topics_project_node");
    ros::NodeHandle nh;

    velocity_publisher = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 10);
    ros::Subscriber scan_subscriber = nh.subscribe("/scan", 10, scanCallback);

    ros::spin(); // Keep the node running

    return 0;
}
