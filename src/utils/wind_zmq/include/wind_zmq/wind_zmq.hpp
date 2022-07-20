/*
 * @Author: windzu
 * @Date: 2022-03-01 14:50:34
 * @LastEditTime: 2022-07-19 15:16:07
 * @LastEditors: ls
 * @Description:
 * @FilePath: /catkin_cr_batch/src/utils/wind_zmq/include/wind_zmq/wind_zmq.hpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#pragma once
// c system headers
#include <assert.h>
#include <unistd.h>
#include <zmq.h>
// cpp system headers
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
// third party headers
// opencv
#include "opencv2/opencv.hpp"
// ros
#include "ros/ros.h"

class ZeroMQPublisher {
public:
  ZeroMQPublisher(std::string zmq_pub_topic, std::string zmq_pub_port)
      : zmq_pub_topic_(zmq_pub_topic), zmq_pub_port_(zmq_pub_port) {}
  bool init();
  bool publish_str(std::string msg);
  bool send_msg(std::string msg);
  bool publish_img(const cv::Mat &image);

private:
  void *context_;
  void *zmq_send_publisher_;
  std::string zmq_pub_topic_;
  std::string zmq_pub_port_;
};

class ZeroMQSubscriber {
public:
  ZeroMQSubscriber(std::string zmq_sub_topic, std::string zmq_sub_port,
                   std::string msgs_type, int cols, int rows)
      : zmq_sub_topic_(zmq_sub_topic), zmq_sub_port_(zmq_sub_port),
        msgs_type_(msgs_type), cols_(cols), rows_(rows) {}
  bool init();
  bool get_str(std::string *str);
  bool get_img(cv::Mat *img);
  void recv_str();
  void recv_img();

private:
  void *context_;
  void *zmq_recv_subscriber_;
  std::string zmq_sub_topic_;
  std::string zmq_sub_port_;
  std::string msgs_type_; // msgs_type_ = "str" or "img"
  int cols_;
  int rows_;
  bool recv_topic_flag_ = false;
  bool recv_msg_if_update_flag_ = false;
  std::thread receive_info_thread_;
  std::string store_str_;
  int buffer_size_ = 1000;
  cv::Mat store_img_;
  bool end_thread_flag_ = false;
};
