/*
 * @Description:
 * @version: 1.0.0
 * @Author: ls
 * @Date: 2022-05-18 10:15:37
 * @LastEditors: ls
 * @LastEditTime: 2022-07-07 15:08:06
 * @todo:
 * @FilePath: /catkin_cone_batch/src/cr/src/postprocess.cpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#include "cr/postprocess.hpp"

bool CRPostProcess::init() { return true; }

bool CRPostProcess::process(cr_result *result, int i) {
  // 针对部分出现在画面中不优雅的处理及预估距离
  Partial_target_processing(result, i);
  return true;
}

void CRPostProcess::Partial_target_processing(cr_result *result, int i) {
  float base = 0;
  int limit = 0;
  switch (i) {
  case 0:
    base = 2037.2;
    limit = 850;
    break;
  case 1:
    base = 2037.2;
    limit = 850;
    break;
  case 2:
    base = 2037.2;
    limit = 850;
    break;
  case 3:
    base = 2037.2;
    limit = 850;
    break;

  default:
    break;
  }
  for (auto &object : result->object) {
    if (object.bbox.y + object.bbox.height >= base) {
      object.depth = -2.0;
    } else {
      object.depth = cal_depth(object.bbox.height, base);
    }
    if (object.depth < 5) {
      result->someone = true;
    }
  }
}

float CRPostProcess::cal_depth(float height, float base) {
  float depth_init = 2037.2 / height;
  return round(depth_init * 100) / 100;
}
