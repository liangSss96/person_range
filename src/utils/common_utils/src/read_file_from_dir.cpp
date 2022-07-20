/*
 * @Author: windzu
 * @Date: 2022-04-16 19:39:02
 * @LastEditTime: 2022-04-16 21:28:46
 * @LastEditors: windzu
 * @Description:
 * @FilePath: /windzu_ws/src/utils/common_utils/src/read_file_from_dir.cpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#include "common_utils/read_file_from_dir.hpp"
int read_file_from_dir(const char *p_dir_name, std::vector<std::string> *file_names) {
  DIR *p_dir = opendir(p_dir_name);
  if (p_dir == nullptr) {
    return -1;
  }

  struct dirent *p_file = nullptr;
  while ((p_file = readdir(p_dir)) != nullptr) {
    if (strcmp(p_file->d_name, ".") != 0 && strcmp(p_file->d_name, "..") != 0) {
      std::string cur_file_name(p_file->d_name);
      file_names->push_back(cur_file_name);
    }
  }

  closedir(p_dir);
  return 0;
}
