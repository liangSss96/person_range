/*
 * @Description:
 * @version: 1.0.0
 * @Author: ls
 * @Date: 2022-05-16 13:37:46
 * @LastEditors: ls
 * @LastEditTime: 2022-05-16 14:41:21
 * @todo:
 * @FilePath:
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#pragma once
// c++ system headers
#include <string>
// third party headers
#include "vector"
// opencv
#include "opencv2/opencv.hpp"
// local headers
#include "base_structure/cr_object.hpp"
#include "enum/enum.hpp"

struct cr_result {
  std::vector<cr_object> object;
  bool someone = false;
};