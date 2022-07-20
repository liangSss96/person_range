/*
 * @Description:
 * @version: 1.0.0
 * @Author: ls
 * @Date: 2022-05-16 11:44:29
 * @LastEditors: ls
 * @LastEditTime: 2022-05-18 10:25:12
 * @todo:
 * @FilePath:
 * /catkin_cone/src/common/base_structure/include/base_structure/cr_object.hpp
 * /catkin_cone/src/common/base_structure/include/base_structure/cr_object.hpp
 * /catkin_cone/src/common/base_structure/include/base_structure/cr_object.hpp
 * /catkin_cone/src/common/base_structure/include/base_structure/cr_object.hpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#pragma once
// third party header
// opencv
#include "opencv2/opencv.hpp"
// local header
#include "enum/enum.hpp"

struct cr_object {
  float prob = -1;                   //置信度
  crclass oblcass = crclass::unknow; //目标类别ID
  float depth = -1;                  //距离
  cv::Rect bbox = cv::Rect(-1, -1, -1, -1);
};
