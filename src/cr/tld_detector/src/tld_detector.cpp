#include "tld_detector/tld_detector.hpp"

bool TLDDetector::init() {
  // 从engine文件中读取其内容至 trtModelStream
  std::ifstream file(engine_file_path_, std::ios::binary);
  if (!file.good()) {
    // ROS_ERROR_STREAM("[ TLDDetector ] Could not read engine file: " <<
    // engine_file_path_);
    return false;
  }
  char *trtModelStream = nullptr;
  size_t size = 0;
  file.seekg(0, file.end);
  size = file.tellg();
  std::cout << size << std::endl;
  file.seekg(0, file.beg);
  trtModelStream = new char[size];
  assert(trtModelStream);
  file.read(trtModelStream, size);
  file.close();

  // prepare input data ---------------------------
  runtime = createInferRuntime(gLogger);
  assert(runtime != nullptr);
  engine = runtime->deserializeCudaEngine(trtModelStream, size);
  assert(engine != nullptr);
  // IExecutionContext* context = engine->createExecutionContext();
  context = engine->createExecutionContext();
  assert(context != nullptr);
  delete[] trtModelStream;
  assert(engine->getNbBindings() == 2);

  // In order to bind the buffers, we need to know the names of the input and
  // output tensors. Note that indices are guaranteed to be less than
  // IEngine::getNbBindings()
  inputIndex = engine->getBindingIndex(INPUT_BLOB_NAME);
  outputIndex = engine->getBindingIndex(OUTPUT_BLOB_NAME);
  assert(inputIndex == 0);
  assert(outputIndex == 1);
  // Create GPU buffers on device
  CUDA_CHECK(cudaMalloc(&buffers[inputIndex],
                        BATCH_SIZE * 3 * INPUT_H * INPUT_W * sizeof(float)));
  CUDA_CHECK(cudaMalloc(&buffers[outputIndex],
                        BATCH_SIZE * OUTPUT_SIZE * sizeof(float)));
  // Create stream
  // cudaStream_t stream;
  CUDA_CHECK(cudaStreamCreate(&stream));

  return true;
}

bool TLDDetector::detect(std::vector<cv::Mat> frame,
                         std::vector<std::vector<cr_object>>  *detected_objects) {
  load_img_to_data(frame);
  _do_inference(*context, stream, buffers, data, prob, BATCH_SIZE);
  post_process(frame, detected_objects);
  if (detected_objects->size() > 0) {
    return true;
  } else {
    return false;
  }
  return true;
}

int TLDDetector::get_width(int x, float gw, int divisor) {
  return static_cast<int>(ceil((x * gw) / divisor)) * divisor;
}

int TLDDetector::get_depth(int x, float gd) {
  if (x == 1)
    return 1;
  int r = round(x * gd);
  if (x * gd - static_cast<int>(x * gd) == 0.5 &&
      (static_cast<int>(x * gd) % 2) == 0) {
    --r;
  }
  return std::max(r, 1);
}

ICudaEngine *TLDDetector::build_engine(unsigned int maxBatchSize,
                                       IBuilder *builder,
                                       IBuilderConfig *config, DataType dt,
                                       float &gd, float &gw,
                                       std::string &wts_name) {
  INetworkDefinition *network = builder->createNetworkV2(0U);

  // Create input tensor of shape {3, INPUT_H, INPUT_W} with name
  // INPUT_BLOB_NAME
  ITensor *data =
      network->addInput(INPUT_BLOB_NAME, dt, Dims3{3, INPUT_H, INPUT_W});
  assert(data);

  std::map<std::string, Weights> weightMap = loadWeights(wts_name);

  /* ------ yolov5 backbone------ */
  auto focus0 =
      focus(network, weightMap, *data, 3, get_width(64, gw), 3, "model.0");
  auto conv1 = convBlock(network, weightMap, *focus0->getOutput(0),
                         get_width(128, gw), 3, 2, 1, "model.1");
  auto bottleneck_CSP2 =
      C3(network, weightMap, *conv1->getOutput(0), get_width(128, gw),
         get_width(128, gw), get_depth(3, gd), true, 1, 0.5, "model.2");
  auto conv3 = convBlock(network, weightMap, *bottleneck_CSP2->getOutput(0),
                         get_width(256, gw), 3, 2, 1, "model.3");
  auto bottleneck_csp4 =
      C3(network, weightMap, *conv3->getOutput(0), get_width(256, gw),
         get_width(256, gw), get_depth(9, gd), true, 1, 0.5, "model.4");
  auto conv5 = convBlock(network, weightMap, *bottleneck_csp4->getOutput(0),
                         get_width(512, gw), 3, 2, 1, "model.5");
  auto bottleneck_csp6 =
      C3(network, weightMap, *conv5->getOutput(0), get_width(512, gw),
         get_width(512, gw), get_depth(9, gd), true, 1, 0.5, "model.6");
  auto conv7 = convBlock(network, weightMap, *bottleneck_csp6->getOutput(0),
                         get_width(1024, gw), 3, 2, 1, "model.7");
  auto spp8 = SPP(network, weightMap, *conv7->getOutput(0), get_width(1024, gw),
                  get_width(1024, gw), 5, 9, 13, "model.8");

  /* ------ yolov5 head ------ */
  auto bottleneck_csp9 =
      C3(network, weightMap, *spp8->getOutput(0), get_width(1024, gw),
         get_width(1024, gw), get_depth(3, gd), false, 1, 0.5, "model.9");
  auto conv10 = convBlock(network, weightMap, *bottleneck_csp9->getOutput(0),
                          get_width(512, gw), 1, 1, 1, "model.10");

  auto upsample11 = network->addResize(*conv10->getOutput(0));
  assert(upsample11);
  upsample11->setResizeMode(ResizeMode::kNEAREST);
  upsample11->setOutputDimensions(
      bottleneck_csp6->getOutput(0)->getDimensions());

  ITensor *inputTensors12[] = {upsample11->getOutput(0),
                               bottleneck_csp6->getOutput(0)};
  auto cat12 = network->addConcatenation(inputTensors12, 2);
  auto bottleneck_csp13 =
      C3(network, weightMap, *cat12->getOutput(0), get_width(1024, gw),
         get_width(512, gw), get_depth(3, gd), false, 1, 0.5, "model.13");
  auto conv14 = convBlock(network, weightMap, *bottleneck_csp13->getOutput(0),
                          get_width(256, gw), 1, 1, 1, "model.14");

  auto upsample15 = network->addResize(*conv14->getOutput(0));
  assert(upsample15);
  upsample15->setResizeMode(ResizeMode::kNEAREST);
  upsample15->setOutputDimensions(
      bottleneck_csp4->getOutput(0)->getDimensions());

  ITensor *inputTensors16[] = {upsample15->getOutput(0),
                               bottleneck_csp4->getOutput(0)};
  auto cat16 = network->addConcatenation(inputTensors16, 2);

  auto bottleneck_csp17 =
      C3(network, weightMap, *cat16->getOutput(0), get_width(512, gw),
         get_width(256, gw), get_depth(3, gd), false, 1, 0.5, "model.17");

  /* ------ detect ------ */
  IConvolutionLayer *det0 = network->addConvolutionNd(
      *bottleneck_csp17->getOutput(0), 3 * (Yolo::CLASS_NUM + 5), DimsHW{1, 1},
      weightMap["model.24.m.0.weight"], weightMap["model.24.m.0.bias"]);
  auto conv18 = convBlock(network, weightMap, *bottleneck_csp17->getOutput(0),
                          get_width(256, gw), 3, 2, 1, "model.18");
  ITensor *inputTensors19[] = {conv18->getOutput(0), conv14->getOutput(0)};
  auto cat19 = network->addConcatenation(inputTensors19, 2);
  auto bottleneck_csp20 =
      C3(network, weightMap, *cat19->getOutput(0), get_width(512, gw),
         get_width(512, gw), get_depth(3, gd), false, 1, 0.5, "model.20");
  IConvolutionLayer *det1 = network->addConvolutionNd(
      *bottleneck_csp20->getOutput(0), 3 * (Yolo::CLASS_NUM + 5), DimsHW{1, 1},
      weightMap["model.24.m.1.weight"], weightMap["model.24.m.1.bias"]);
  auto conv21 = convBlock(network, weightMap, *bottleneck_csp20->getOutput(0),
                          get_width(512, gw), 3, 2, 1, "model.21");
  ITensor *inputTensors22[] = {conv21->getOutput(0), conv10->getOutput(0)};
  auto cat22 = network->addConcatenation(inputTensors22, 2);
  auto bottleneck_csp23 =
      C3(network, weightMap, *cat22->getOutput(0), get_width(1024, gw),
         get_width(1024, gw), get_depth(3, gd), false, 1, 0.5, "model.23");
  IConvolutionLayer *det2 = network->addConvolutionNd(
      *bottleneck_csp23->getOutput(0), 3 * (Yolo::CLASS_NUM + 5), DimsHW{1, 1},
      weightMap["model.24.m.2.weight"], weightMap["model.24.m.2.bias"]);

  auto yolo = addYoLoLayer(network, weightMap, "model.24",
                           std::vector<IConvolutionLayer *>{det0, det1, det2});
  yolo->getOutput(0)->setName(OUTPUT_BLOB_NAME);
  network->markOutput(*yolo->getOutput(0));

  // Build engine
  builder->setMaxBatchSize(maxBatchSize);
  config->setMaxWorkspaceSize(16 * (1 << 20)); // 16MB
#if defined(USE_FP16)
  config->setFlag(BuilderFlag::kFP16);
#elif defined(USE_INT8)
  std::cout << "Your platform support int8: "
            << (builder->platformHasFastInt8() ? "true" : "false") << std::endl;
  assert(builder->platformHasFastInt8());
  config->setFlag(BuilderFlag::kINT8);
  Int8EntropyCalibrator2 *calibrator = new Int8EntropyCalibrator2(
      1, INPUT_W, INPUT_H, "./coco_calib/", "int8calib.table", INPUT_BLOB_NAME);
  config->setInt8Calibrator(calibrator);
#endif

  std::cout << "Building engine, please wait for a while..." << std::endl;
  ICudaEngine *engine = builder->buildEngineWithConfig(*network, *config);
  std::cout << "Build engine successfully!" << std::endl;

  // Don't need the network any more
  network->destroy();

  // Release host memory
  for (auto &mem : weightMap) {
    free((void *)(mem.second.values));
  }

  return engine;
}

ICudaEngine *TLDDetector::build_engine_p6(unsigned int maxBatchSize,
                                          IBuilder *builder,
                                          IBuilderConfig *config, DataType dt,
                                          float &gd, float &gw,
                                          std::string &wts_name) {
  INetworkDefinition *network = builder->createNetworkV2(0U);

  // Create input tensor of shape {3, INPUT_H, INPUT_W} with name
  // INPUT_BLOB_NAME
  ITensor *data =
      network->addInput(INPUT_BLOB_NAME, dt, Dims3{3, INPUT_H, INPUT_W});
  assert(data);

  std::map<std::string, Weights> weightMap = loadWeights(wts_name);

  /* ------ yolov5 backbone------ */
  auto focus0 =
      focus(network, weightMap, *data, 3, get_width(64, gw), 3, "model.0");
  auto conv1 = convBlock(network, weightMap, *focus0->getOutput(0),
                         get_width(128, gw), 3, 2, 1, "model.1");
  auto c3_2 = C3(network, weightMap, *conv1->getOutput(0), get_width(128, gw),
                 get_width(128, gw), get_depth(3, gd), true, 1, 0.5, "model.2");
  auto conv3 = convBlock(network, weightMap, *c3_2->getOutput(0),
                         get_width(256, gw), 3, 2, 1, "model.3");
  auto c3_4 = C3(network, weightMap, *conv3->getOutput(0), get_width(256, gw),
                 get_width(256, gw), get_depth(9, gd), true, 1, 0.5, "model.4");
  auto conv5 = convBlock(network, weightMap, *c3_4->getOutput(0),
                         get_width(512, gw), 3, 2, 1, "model.5");
  auto c3_6 = C3(network, weightMap, *conv5->getOutput(0), get_width(512, gw),
                 get_width(512, gw), get_depth(9, gd), true, 1, 0.5, "model.6");
  auto conv7 = convBlock(network, weightMap, *c3_6->getOutput(0),
                         get_width(768, gw), 3, 2, 1, "model.7");
  auto c3_8 = C3(network, weightMap, *conv7->getOutput(0), get_width(768, gw),
                 get_width(768, gw), get_depth(3, gd), true, 1, 0.5, "model.8");
  auto conv9 = convBlock(network, weightMap, *c3_8->getOutput(0),
                         get_width(1024, gw), 3, 2, 1, "model.9");
  auto spp10 =
      SPP(network, weightMap, *conv9->getOutput(0), get_width(1024, gw),
          get_width(1024, gw), 3, 5, 7, "model.10");
  auto c3_11 =
      C3(network, weightMap, *spp10->getOutput(0), get_width(1024, gw),
         get_width(1024, gw), get_depth(3, gd), false, 1, 0.5, "model.11");

  /* ------ yolov5 head ------ */
  auto conv12 = convBlock(network, weightMap, *c3_11->getOutput(0),
                          get_width(768, gw), 1, 1, 1, "model.12");
  auto upsample13 = network->addResize(*conv12->getOutput(0));
  assert(upsample13);
  upsample13->setResizeMode(ResizeMode::kNEAREST);
  upsample13->setOutputDimensions(c3_8->getOutput(0)->getDimensions());
  ITensor *inputTensors14[] = {upsample13->getOutput(0), c3_8->getOutput(0)};
  auto cat14 = network->addConcatenation(inputTensors14, 2);
  auto c3_15 =
      C3(network, weightMap, *cat14->getOutput(0), get_width(1536, gw),
         get_width(768, gw), get_depth(3, gd), false, 1, 0.5, "model.15");

  auto conv16 = convBlock(network, weightMap, *c3_15->getOutput(0),
                          get_width(512, gw), 1, 1, 1, "model.16");
  auto upsample17 = network->addResize(*conv16->getOutput(0));
  assert(upsample17);
  upsample17->setResizeMode(ResizeMode::kNEAREST);
  upsample17->setOutputDimensions(c3_6->getOutput(0)->getDimensions());
  ITensor *inputTensors18[] = {upsample17->getOutput(0), c3_6->getOutput(0)};
  auto cat18 = network->addConcatenation(inputTensors18, 2);
  auto c3_19 =
      C3(network, weightMap, *cat18->getOutput(0), get_width(1024, gw),
         get_width(512, gw), get_depth(3, gd), false, 1, 0.5, "model.19");

  auto conv20 = convBlock(network, weightMap, *c3_19->getOutput(0),
                          get_width(256, gw), 1, 1, 1, "model.20");
  auto upsample21 = network->addResize(*conv20->getOutput(0));
  assert(upsample21);
  upsample21->setResizeMode(ResizeMode::kNEAREST);
  upsample21->setOutputDimensions(c3_4->getOutput(0)->getDimensions());
  ITensor *inputTensors21[] = {upsample21->getOutput(0), c3_4->getOutput(0)};
  auto cat22 = network->addConcatenation(inputTensors21, 2);
  auto c3_23 =
      C3(network, weightMap, *cat22->getOutput(0), get_width(512, gw),
         get_width(256, gw), get_depth(3, gd), false, 1, 0.5, "model.23");

  auto conv24 = convBlock(network, weightMap, *c3_23->getOutput(0),
                          get_width(256, gw), 3, 2, 1, "model.24");
  ITensor *inputTensors25[] = {conv24->getOutput(0), conv20->getOutput(0)};
  auto cat25 = network->addConcatenation(inputTensors25, 2);
  auto c3_26 =
      C3(network, weightMap, *cat25->getOutput(0), get_width(1024, gw),
         get_width(512, gw), get_depth(3, gd), false, 1, 0.5, "model.26");

  auto conv27 = convBlock(network, weightMap, *c3_26->getOutput(0),
                          get_width(512, gw), 3, 2, 1, "model.27");
  ITensor *inputTensors28[] = {conv27->getOutput(0), conv16->getOutput(0)};
  auto cat28 = network->addConcatenation(inputTensors28, 2);
  auto c3_29 =
      C3(network, weightMap, *cat28->getOutput(0), get_width(1536, gw),
         get_width(768, gw), get_depth(3, gd), false, 1, 0.5, "model.29");

  auto conv30 = convBlock(network, weightMap, *c3_29->getOutput(0),
                          get_width(768, gw), 3, 2, 1, "model.30");
  ITensor *inputTensors31[] = {conv30->getOutput(0), conv12->getOutput(0)};
  auto cat31 = network->addConcatenation(inputTensors31, 2);
  auto c3_32 =
      C3(network, weightMap, *cat31->getOutput(0), get_width(2048, gw),
         get_width(1024, gw), get_depth(3, gd), false, 1, 0.5, "model.32");

  /* ------ detect ------ */
  IConvolutionLayer *det0 = network->addConvolutionNd(
      *c3_23->getOutput(0), 3 * (Yolo::CLASS_NUM + 5), DimsHW{1, 1},
      weightMap["model.33.m.0.weight"], weightMap["model.33.m.0.bias"]);
  IConvolutionLayer *det1 = network->addConvolutionNd(
      *c3_26->getOutput(0), 3 * (Yolo::CLASS_NUM + 5), DimsHW{1, 1},
      weightMap["model.33.m.1.weight"], weightMap["model.33.m.1.bias"]);
  IConvolutionLayer *det2 = network->addConvolutionNd(
      *c3_29->getOutput(0), 3 * (Yolo::CLASS_NUM + 5), DimsHW{1, 1},
      weightMap["model.33.m.2.weight"], weightMap["model.33.m.2.bias"]);
  IConvolutionLayer *det3 = network->addConvolutionNd(
      *c3_32->getOutput(0), 3 * (Yolo::CLASS_NUM + 5), DimsHW{1, 1},
      weightMap["model.33.m.3.weight"], weightMap["model.33.m.3.bias"]);

  auto yolo =
      addYoLoLayer(network, weightMap, "model.33",
                   std::vector<IConvolutionLayer *>{det0, det1, det2, det3});
  yolo->getOutput(0)->setName(OUTPUT_BLOB_NAME);
  network->markOutput(*yolo->getOutput(0));

  // Build engine
  builder->setMaxBatchSize(maxBatchSize);
  config->setMaxWorkspaceSize(16 * (1 << 20)); // 16MB
#if defined(USE_FP16)
  config->setFlag(BuilderFlag::kFP16);
#elif defined(USE_INT8)
  std::cout << "Your platform support int8: "
            << (builder->platformHasFastInt8() ? "true" : "false") << std::endl;
  assert(builder->platformHasFastInt8());
  config->setFlag(BuilderFlag::kINT8);
  Int8EntropyCalibrator2 *calibrator = new Int8EntropyCalibrator2(
      1, INPUT_W, INPUT_H, "./coco_calib/", "int8calib.table", INPUT_BLOB_NAME);
  config->setInt8Calibrator(calibrator);
#endif

  std::cout << "Building engine, please wait for a while..." << std::endl;
  ICudaEngine *engine = builder->buildEngineWithConfig(*network, *config);
  std::cout << "Build engine successfully!" << std::endl;

  // Don't need the network any more
  network->destroy();

  // Release host memory
  for (auto &mem : weightMap) {
    free((void *)(mem.second.values));
  }

  return engine;
}

void TLDDetector::api_to_model(unsigned int maxBatchSize,
                               IHostMemory **modelStream, bool &is_p6,
                               float &gd, float &gw, std::string &wts_name) {
  // Create builder
  IBuilder *builder = createInferBuilder(gLogger);
  IBuilderConfig *config = builder->createBuilderConfig();

  // Create model to populate the network, then set the outputs and create an
  // engine
  ICudaEngine *engine = nullptr;
  if (is_p6) {
    engine = build_engine_p6(maxBatchSize, builder, config, DataType::kFLOAT,
                             gd, gw, wts_name);
  } else {
    engine = build_engine(maxBatchSize, builder, config, DataType::kFLOAT, gd,
                          gw, wts_name);
  }
  assert(engine != nullptr);

  // Serialize the engine
  (*modelStream) = engine->serialize();

  // Close everything down
  engine->destroy();
  builder->destroy();
  config->destroy();
}

void TLDDetector::load_img_to_data(const std::vector<cv::Mat> &img) {
  // if (img.empty()) {
  //   // ROS_ERROR_STREAM("[ TLDDetector ] load_img_to_data: image is empty!");
  //   return;
  // }
  for(int j = 0; j < img.size(); j++){
    if (img[j].empty()) continue;
    cv::Mat pr_img = resize_img(img[j], INPUT_W, INPUT_H); // letterbox BGR to RGB and resize to target size
    int i = 0;
    for (int row = 0; row < INPUT_H; ++row) {
      uchar *uc_pixel = pr_img.data + row * pr_img.step;
      for (int col = 0; col < INPUT_W; ++col) {
        data[j * 3 * INPUT_H * INPUT_W+i] = static_cast<float>(uc_pixel[2]) / 255.0;
        data[j * 3 * INPUT_H * INPUT_W+i + INPUT_H * INPUT_W] = static_cast<float>(uc_pixel[1]) / 255.0;
        data[j * 3 * INPUT_H * INPUT_W+i + 2 * INPUT_H * INPUT_W] = static_cast<float>(uc_pixel[0]) / 255.0;
        uc_pixel += 3;
        ++i;
      }
    }
  }
}

void TLDDetector::_do_inference(IExecutionContext &context,
                                cudaStream_t &stream, void **buffers,
                                float *input, float *output, int batchSize) {
  // DMA input batch data to device, infer on the batch asynchronously, and DMA
  // output back to host
  CUDA_CHECK(cudaMemcpyAsync(buffers[0], input,
                             batchSize * 3 * INPUT_H * INPUT_W * sizeof(float),
                             cudaMemcpyHostToDevice, stream));
  context.enqueue(batchSize, buffers, stream, nullptr);
  CUDA_CHECK(cudaMemcpyAsync(output, buffers[1],
                             batchSize * OUTPUT_SIZE * sizeof(float),
                             cudaMemcpyDeviceToHost, stream));
  cudaStreamSynchronize(stream);
}

void TLDDetector::post_process(const std::vector<cv::Mat> &img,
                               std::vector<std::vector<cr_object>> *detected_objects) {
  std::vector<std::vector<Yolo::Detection>> batch_res(BATCH_SIZE);
  
  for (int b = 0; b < BATCH_SIZE; b++) {
    auto& res = batch_res[b];
    nms(res, &prob[b * OUTPUT_SIZE], CONF_THRESH, NMS_THRESH);
  }
  
  for (int b = 0; b < BATCH_SIZE; b++){
    auto& res = batch_res[b];
    for (size_t j = 0; j < res.size(); j++) {
    // 构造 cr_Object
      float prob = res[j].conf;
      crclass tl_class = (crclass)(res[j].class_id);
      // debug
      // std::cout << "class id :" << res[j].class_id << std::endl;

      cv::Rect rect = get_rect(img[b], res[j].bbox, INPUT_W,
                              INPUT_H); // 得到相对于输入detector的img尺寸的bbox
      // float depth = calculate_depth(rect);
      detected_objects->at(b).push_back(cr_object{prob, tl_class, -1, rect});
    }
  }
}

float TLDDetector::calculate_depth(cv::Rect box){
  float depth_init = 1650.0/ box.height;
  float a1 = 1.62 * 0.18 / 0.71;
  float c1 = 0.18;

  float a = 1.0;
  float b = -(c1 + depth_init);
  float c = depth_init + c1 - depth_init + a1;
  float temp = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
  return round(temp * 100) / 100;
}
