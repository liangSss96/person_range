/*
 * @Author: windzu
 * @Date: 2022-03-01 14:32:16
 * @LastEditTime: 2022-03-01 14:40:25
 * @LastEditors: windzu
 * @Description:
 * @FilePath: /windzu_ws/src/traffic_light_detection/yolov5_tensorrt/include/yolov5_tensorrt/cuda_utils.h
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#pragma once

// third party headers
// cuda
#include "cuda_runtime_api.h"

#ifndef CUDA_CHECK
#define CUDA_CHECK(callstr)                                                              \
  {                                                                                      \
    cudaError_t error_code = callstr;                                                    \
    if (error_code != cudaSuccess) {                                                     \
      std::cerr << "CUDA error " << error_code << " at " << __FILE__ << ":" << __LINE__; \
      assert(0);                                                                         \
    }                                                                                    \
  }
#endif  // CUDA_CHECK
