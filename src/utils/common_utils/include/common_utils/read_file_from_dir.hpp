/*
 * @Author: windzu
 * @Date: 2022-04-16 19:38:50
 * @LastEditTime: 2022-04-16 21:28:39
 * @LastEditors: windzu
 * @Description:
 * @FilePath: /windzu_ws/src/utils/common_utils/include/common_utils/read_file_from_dir.hpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#pragma once
// c system headers
#include <dirent.h>
// c++ system headers
#include <string>
#include <vector>
// third party headers
// opencv
#include "opencv2/opencv.hpp"

/**
 * @description: 根据文件路径读取文件名
 * @param {char*} p_dir_name : 文件夹路径
 * @param {std::vector<std::string> *} file_names : 路径内所以文件名
 * @return {int} : status
 */
int read_file_from_dir(const char *p_dir_name, std::vector<std::string> *file_names);
