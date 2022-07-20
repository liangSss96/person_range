/*
 * @Description:
 * @version: 1.0.0
 * @Author: ls
 * @Date: 2022-05-16 09:35:01
 * @LastEditors: ls
 * @LastEditTime: 2022-07-20 09:04:56
 * @todo:
 * @FilePath: /catkin_cr_batch/src/cr/include/cr/cr.hpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */

#pragma once
#include <mutex>

#include "iostream"

// opcncv
#include "opencv2/opencv.hpp"
// ros
#include "ros/ros.h"
#include "std_msgs/String.h"
// ros img
#include "cv_bridge/cv_bridge.h"
#include "image_transport/image_transport.h"
#include "sensor_msgs/CompressedImage.h"
#include "sensor_msgs/Image.h"
#include "sensor_msgs/image_encodings.h"

// loacl header
#include "common_utils/split_string.hpp"
#include "cr_send_result.hpp"
#include "enum/enum.hpp"
#include "postprocess.hpp"
#include "tld_detector/tld_detector.hpp"
#include "wind_zmq/wind_zmq.hpp"

#define BATCH_SIZE 4
class CR {
private:
  // ros
  ros::NodeHandle nh_;
  ros::NodeHandle pnh_;
  int loop_rate_hz_ = 5;
  std::string zmq_pub_topic_ = std::string("peddet");
  std::string zmq_pub_port_ = std::string("tcp://0.0.0.0:1973");
  std::unique_ptr<cr_send_result> cr_send_result_ptr_;
  std::unique_ptr<TLDDetector> detector_ptr_;
  std::unique_ptr<CRPostProcess> postprocess_ptr_;
  std::unique_ptr<ZeroMQPublisher> zmq_publish;

  // msgs topic
  std::string img_topic_;
  std::string img_topic_1;
  std::string img_topic_2;
  std::string img_topic_3;

  // msgs received
  cv::Mat locked_img_;
  cv::Mat locked_img_1;
  cv::Mat locked_img_2;
  cv::Mat locked_img_3;

  // msgs subscriber
  ros::Subscriber img_sub_;
  ros::Subscriber img_sub_1;
  ros::Subscriber img_sub_2;
  ros::Subscriber img_sub_3;

  // msgs updated flag
  bool img_updated_ = false;
  bool img_updated_1 = false;
  bool img_updated_2 = false;
  bool img_updated_3 = false;

  // mutex
  std::mutex mutex_;
  std::mutex mutex_1;
  std::mutex mutex_2;
  std::mutex mutex_3;

  // detector weight path
  std::string cr_detector_weight_path_;

  std::vector<std::pair<std::string, ros::Subscriber>> topic_list;

  std::mutex *key[4] = {&mutex_, &mutex_1, &mutex_2, &mutex_3};
  bool *flag[4] = {&img_updated_, &img_updated_1, &img_updated_2,
                   &img_updated_3};
  cv::Mat *img[4] = {&locked_img_, &locked_img_1, &locked_img_2, &locked_img_3};

public:
  CR(ros::NodeHandle nh, ros::NodeHandle pnh) : nh_(nh), pnh_(pnh) {
    pnh_.param("image_topic", img_topic_, std::string("/front/image_raw"));
    pnh_.param("image_topic1", img_topic_1, std::string("/back/image_raw"));
    pnh_.param("image_topic2", img_topic_2, std::string("/left/image_raw"));
    pnh_.param("image_topic3", img_topic_3, std::string("/right/image_raw"));
    pnh_.param("loop_rate_hz", loop_rate_hz_, static_cast<int>(5));
    pnh_.param("cr_detector_weight_path", cr_detector_weight_path_,
               std::string(""));
  }
  bool init();
  void start();

private:
  bool msgs_sub_init();
  void receive_raw_img_callback(const sensor_msgs::ImageConstPtr &img_msg,
                                cv::Mat *get_img, bool *flag, std::mutex *key);
  void receive_compressed_img_callback(
      const sensor_msgs::CompressedImageConstPtr &img_msg, cv::Mat *get_img,
      bool *flag, std::mutex *key);
};
