/*
 * @Description:
 * @version: 1.0.0
 * @Author: ls
 * @Date: 2022-05-16 11:09:35
 * @LastEditors: ls
 * @LastEditTime: 2022-05-17 17:15:02
 * @todo:
 * @FilePath:
 * /catkin_cone/src/utils/common_utils/include/common_utils/split_string.hpp
 * /catkin_cone/src/utils/common_utils/include/common_utils/split_string.hpp
 * /catkin_cone/src/utils/common_utils/include/common_utils/split_string.hpp
 * /catkin_cone/src/utils/common_utils/include/common_utils/split_string.hpp
 * /catkin_cone/src/utils/common_utils/include/common_utils/split_string.hpp
 * /catkin_cone/src/utils/common_utils/include/common_utils/split_string.hpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#pragma once
// c++ system headers
#include <string>
#include <vector>

#include "iomanip"
#include "sstream"

/**
 * @description: split string by delimiter
 * @param {string&} s string
 * @param {vector} result
 * @param {string&} c
 * @return {void}
 */
void split_string(const std::string &s, std::vector<std::string> *v,
                  const std::string &c);
