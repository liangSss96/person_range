/*
 * @Description:
 * @version: 1.0.0
 * @Author: ls
 * @Date: 2022-05-16 11:10:09
 * @LastEditors: ls
 * @LastEditTime: 2022-05-17 17:14:58
 * @todo:
 * @FilePath: /catkin_cone/src/utils/common_utils/src/split_string.cpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#include "common_utils/split_string.hpp"

void split_string(const std::string &s, std::vector<std::string> *v,
                  const std::string &c) {
  v->clear();
  std::string::size_type pos1, pos2;
  pos2 = s.find(c);
  pos1 = 0;
  while (pos2 != std::string::npos) {
    v->push_back(s.substr(pos1, pos2 - pos1));
    pos1 = pos2 + c.size();
    pos2 = s.find(c, pos1);
  }
  if (pos1 != s.length())
    v->push_back(s.substr(pos1));
}
