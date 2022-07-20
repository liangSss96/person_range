/*
 * @Author: windzu
 * @Date: 2022-03-01 14:32:16
 * @LastEditTime: 2022-05-18 18:19:19
 * @LastEditors: ls
 * @Description:
 * @FilePath:

 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#pragma once
// c system headers
#include <yaml-cpp/yaml.h>
// cpp system headers
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
// third party headers
// opencv
#include "opencv2/dnn/dnn.hpp"
#include "opencv2/opencv.hpp"
// local headers
#include "common_utils/opencv_extension.hpp"
#include "tld_detector/calibrator.hpp"
#include "tld_detector/common.hpp"
#include "tld_detector/cuda_utils.hpp"
#include "tld_detector/logging.hpp"

#define USE_FP16 // set USE_INT8 or USE_FP16 or USE_FP32
#define DEVICE 0 // GPU id
#define NMS_THRESH 0.25
#define CONF_THRESH 0.6
#define BATCH_SIZE 4

class TLDDetector {
public:
  // yolov5 openvino构造
  // xml_path ： xml文件路径
  // cof_threshold ： 框置信度乘以物品种类置信度
  // nms_area_threshold ： nms最小重叠面积阈值
  explicit TLDDetector(std::string engine_file_path)
      : engine_file_path_(engine_file_path) {}

  ~TLDDetector() {
    // Release stream and buffers
    cudaStreamDestroy(stream);
    CUDA_CHECK(cudaFree(buffers[inputIndex]));
    CUDA_CHECK(cudaFree(buffers[outputIndex]));
    // Destroy the engine
    context->destroy();
    engine->destroy();
    runtime->destroy();
  }

  bool init();
  bool detect(std::vector<cv::Mat> frame, std::vector<std::vector<cr_object>> *detected_objects);

private:
  bool engine_init();
  static int get_width(int x, float gw, int divisor = 8);
  static int get_depth(int x, float gd);
  void post_process(const std::vector<cv::Mat> &img,
                    std::vector<std::vector<cr_object>> *detected_objects);

  // load img from cpu memory to gpu memory
  void load_img_to_data(const std::vector<cv::Mat> &img);

private:
  std::string engine_file_path_;

  // stuff we know about the network and the input/output blobs
  static const int INPUT_H = Yolo::INPUT_H;
  static const int INPUT_W = Yolo::INPUT_W;
  static const int CLASS_NUM = Yolo::CLASS_NUM;
  static const int OUTPUT_SIZE =
      Yolo::MAX_OUTPUT_BBOX_COUNT * sizeof(Yolo::Detection) / sizeof(float) +
      1; // we assume the yololayer outputs no more than MAX_OUTPUT_BBOX_COUNT
         // boxes that conf >= 0.1
  const char *INPUT_BLOB_NAME = "data";
  const char *OUTPUT_BLOB_NAME = "prob";
  Logger gLogger;

  float data[BATCH_SIZE * 3 * INPUT_H * INPUT_W];
  float prob[BATCH_SIZE * OUTPUT_SIZE];

  IRuntime *runtime;
  ICudaEngine *engine;
  IExecutionContext *context;
  cudaStream_t stream;

  void *buffers[2];
  int inputIndex;
  int outputIndex;

  ICudaEngine *build_engine(unsigned int maxBatchSize, IBuilder *builder,
                            IBuilderConfig *config, DataType dt, float &gd,
                            float &gw, std::string &wts_name);

  ICudaEngine *build_engine_p6(unsigned int maxBatchSize, IBuilder *builder,
                               IBuilderConfig *config, DataType dt, float &gd,
                               float &gw, std::string &wts_name);

  void api_to_model(unsigned int maxBatchSize, IHostMemory **modelStream,
                    bool &is_p6, float &gd, float &gw, std::string &wts_name);

  void _do_inference(IExecutionContext &context, cudaStream_t &stream,
                     void **buffers, float *input, float *output,
                     int batchSize);

  float calculate_depth(cv::Rect box);
};
