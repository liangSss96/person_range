/*
 * @Description:
 * @version: 1.0.0
 * @Author: ls
 * @Date: 2022-05-16 15:33:47
 * @LastEditors: ls
 * @LastEditTime: 2022-05-16 15:38:00
 * @todo:
 * @FilePath: /catkin_cone/src/cr/src/cr_node.cpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#include "cr/cr.hpp"
#include "ros/ros.h"

int main(int argc, char **argv) {
  ros::init(argc, argv, "cr");
  ros::NodeHandle nh;
  ros::NodeHandle pnh("~");

  CR cr(nh, pnh);

  if (cr.init()) {
    cr.start();
    return 0;
  } else {
    ROS_ERROR_STREAM("[ main ] CR init failed");
    return 0;
  }
  return 0;
}