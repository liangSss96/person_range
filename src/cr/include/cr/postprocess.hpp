/*
 * @Description:
 * @version: 1.0.0
 * @Author: ls
 * @Date: 2022-05-18 09:54:29
 * @LastEditors: ls
 * @LastEditTime: 2022-07-07 13:09:34
 * @todo:
 * @FilePath: /catkin_cone_batch/src/cr/include/cr/postprocess.hpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#pragma once

#include <string>
#include <vector>
// ros
#include "ros/ros.h"
// local header
#include "base_structure/cr_object.hpp"
#include "base_structure/cr_result.hpp"
#include "enum/enum.hpp"

class CRPostProcess {
private:
  // ros
  ros::NodeHandle nh_;
  ros::NodeHandle pnh_;

public:
  CRPostProcess(ros::NodeHandle &nh, ros::NodeHandle &pnh)
      : nh_(nh), pnh_(pnh) {}
  bool init();
  bool process(cr_result *result, int i);

private:
  void Partial_target_processing(cr_result *result, int i);
  float cal_depth(float height, float base);
};
