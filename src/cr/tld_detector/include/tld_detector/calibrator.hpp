/*
 * @Author: windzu
 * @Date: 2022-03-01 14:32:16
 * @LastEditTime: 2022-04-16 21:15:25
 * @LastEditors: windzu
 * @Description:
 * @FilePath: /windzu_ws/src/tld/tld/tld_detector/include/tld_detector/calibrator.hpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#pragma once
// cpp system headers
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
// third party headers
// opencv
#include "opencv2/dnn/dnn.hpp"
// tensorrt
#include "NvInfer.h"

// local headers
#include "common_utils/opencv_extension.hpp"
#include "common_utils/read_file_from_dir.hpp"
#include "tld_detector/cuda_utils.hpp"
#if NV_TENSORRT_MAJOR >= 8
#define TRT_NOEXCEPT noexcept
#else
#define TRT_NOEXCEPT
#endif

//! \class Int8EntropyCalibrator2
//!
//! \brief Implements Entropy calibrator 2.
//!  CalibrationAlgoType is kENTROPY_CALIBRATION_2.
//!
class Int8EntropyCalibrator2 : public nvinfer1::IInt8EntropyCalibrator2 {
 public:
  Int8EntropyCalibrator2(int batchsize, int input_w, int input_h, const char* img_dir, const char* calib_table_name,
                         const char* input_blob_name, bool read_cache = true);

  virtual ~Int8EntropyCalibrator2();
  int getBatchSize() const TRT_NOEXCEPT override;
  bool getBatch(void* bindings[], const char* names[], int nbBindings) TRT_NOEXCEPT override;
  const void* readCalibrationCache(size_t& length) TRT_NOEXCEPT override;
  void writeCalibrationCache(const void* cache, size_t length) TRT_NOEXCEPT override;

 private:
  int batchsize_;
  int input_w_;
  int input_h_;
  int img_idx_;
  std::string img_dir_;
  std::vector<std::string> img_files_;
  size_t input_count_;
  std::string calib_table_name_;
  const char* input_blob_name_;
  bool read_cache_;
  void* device_input_;
  std::vector<char> calib_cache_;
};
