/*
 * @Author: windzu
 * @Date: 2022-03-01 14:52:01
 * @LastEditTime: 2022-07-19 15:15:17
 * @LastEditors: ls
 * @Description:
 * @FilePath: /catkin_cr_batch/src/utils/wind_zmq/src/wind_zmq.cpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#include "wind_zmq/wind_zmq.hpp"

bool ZeroMQPublisher::init() {
  context_ = zmq_ctx_new();
  if (context_ == NULL) {
    ROS_ERROR_STREAM("[ ZeroMQPublisher ] zmq_ctx_new failed");
    return false;
  }

  zmq_send_publisher_ = zmq_socket(context_, ZMQ_PUB);
  if (zmq_send_publisher_ == NULL) {
    ROS_ERROR_STREAM("[ ZeroMQPublisher ] zmq_socket failed");
    return false;
  }
  int ret = zmq_bind(zmq_send_publisher_, zmq_pub_port_.c_str());
  if (ret != 0) {
    ROS_ERROR_STREAM("[ ZeroMQPublisher ] zmq_bind failed");
    return false;
  }

  return true;
}

bool ZeroMQPublisher::publish_str(const std::string msg) {
  int ret = -1;
  ret = zmq_send(zmq_send_publisher_, zmq_pub_topic_.c_str(),
                 strlen(zmq_pub_topic_.c_str()), ZMQ_SNDMORE);
  if (ret == -1) {
    ROS_WARN_STREAM("[ ZeroMQPublisher ] publish failed");
    return false;
  }
  ret = zmq_send(zmq_send_publisher_, msg.c_str(), strlen(msg.c_str()), 0);
  if (ret == -1) {
    ROS_WARN_STREAM("[ ZeroMQPublisher ] publish failed");
    return false;
  }
  return true;
}

bool ZeroMQPublisher::send_msg(std::string msg) {
  int ret = -1;
  ret = zmq_send(zmq_send_publisher_, msg.c_str(), strlen(msg.c_str()), 0);
  if (ret == -1) {
    ROS_WARN_STREAM("[ ZeroMQPublisher ] send_msg failed");
    return false;
  }
  return true;
}

bool ZeroMQPublisher::publish_img(const cv::Mat &image) {
  int ret = -1;
  ret = zmq_send(zmq_send_publisher_, zmq_pub_topic_.c_str(),
                 strlen(zmq_pub_topic_.c_str()), ZMQ_SNDMORE);
  if (ret == -1) {
    ROS_WARN_STREAM("[ ZeroMQPublisher ] publish failed");
    return false;
  }

  int height = image.rows;
  int width = image.cols;

  ret = zmq_send(zmq_send_publisher_, image.data,
                 (height * width * 3 * sizeof(uint8_t)), ZMQ_NOBLOCK);
  if (ret == -1) {
    ROS_WARN_STREAM("[ ZeroMQPublisher ] publish_img failed");
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
bool ZeroMQSubscriber::init() {
  context_ = zmq_ctx_new();
  if (context_ == NULL) {
    ROS_ERROR_STREAM("[ ZeroMQSubscriber ] zmq_ctx_new failed");
    return false;
  }
  zmq_recv_subscriber_ = zmq_socket(context_, ZMQ_SUB);
  if (zmq_recv_subscriber_ == NULL) {
    ROS_ERROR_STREAM("[ ZeroMQSubscriber ] zmq_socket failed");
    return false;
  }
  int ret = zmq_connect(zmq_recv_subscriber_, zmq_sub_port_.c_str());
  if (ret != 0) {
    ROS_ERROR_STREAM("[ ZeroMQSubscriber ] zmq_connect failed");
    ROS_ERROR_STREAM(
        "[ ZeroMQSubscriber ] zmq_sub_topic_ : " << zmq_sub_topic_);
    ROS_ERROR_STREAM("[ ZeroMQSubscriber ] zmq_sub_port_ : " << zmq_sub_port_);
    return false;
  }

  ret = zmq_setsockopt(zmq_recv_subscriber_, ZMQ_SUBSCRIBE, "", 0);
  if (ret != 0) {
    ROS_ERROR_STREAM("[ ZeroMQSubscriber ] zmq_setsockopt failed");
    return false;
  }
  if (msgs_type_ == "str") {
    receive_info_thread_ = std::thread(&ZeroMQSubscriber::recv_str, this);
    receive_info_thread_.detach();
  } else if (msgs_type_ == "img") {
    buffer_size_ = (rows_ * cols_ * 3);
    receive_info_thread_ = std::thread(&ZeroMQSubscriber::recv_img, this);
    receive_info_thread_.detach();
  } else {
    ROS_ERROR_STREAM("[ ZeroMQSubscriber ] msgs_type_ error");
    return false;
  }
  return true;
}

bool ZeroMQSubscriber::get_str(std::string *str) {
  if (recv_msg_if_update_flag_) {
    *str = store_str_;
    recv_msg_if_update_flag_ = false;
    return true;
  } else {
    ROS_DEBUG_STREAM("[ ZeroMQSubscriber ] get_msg msg has not updated");
    return false;
  }
}

bool ZeroMQSubscriber::get_img(cv::Mat *img) {
  if (!store_img_.empty()) {
    *img = store_img_.clone();
    return true;
  } else {
    ROS_DEBUG_STREAM("[ ZeroMQSubscriber ] get_img img has not updated");
    return false;
  }
}

void ZeroMQSubscriber::recv_str() {
  while (1) {
    char *info_buffer_ptr = new char[buffer_size_];
    int ret = zmq_recv(zmq_recv_subscriber_, info_buffer_ptr, buffer_size_, 0);
    if (ret == -1) {
      ROS_WARN_STREAM("[ ZeroMQSubscriber ] recv_msg failed");
      continue;
    }

    std::string tmp = "";
    for (int i = 0; i < ret; ++i) {
      tmp += info_buffer_ptr[i];
    }

    if (recv_topic_flag_ == false && tmp[0] == zmq_sub_topic_[0]) {
      recv_topic_flag_ = true;
    } else if (recv_topic_flag_ == true && tmp != "") {
      store_str_ = tmp;
      recv_topic_flag_ = false;
      recv_msg_if_update_flag_ = true;
    } else {
      continue;
    }
    delete info_buffer_ptr;
    if (end_thread_flag_) {
      break;
    }
    usleep(30000);
  }
  return;
}

void ZeroMQSubscriber::recv_img() {
  while (1) {
    char *info_buffer_ptr = new char[buffer_size_];

    int ret = zmq_recv(zmq_recv_subscriber_, info_buffer_ptr, buffer_size_, 0);
    if (ret == -1) {
      ROS_WARN_STREAM("[ ZeroMQSubscriber ] recv_msg failed");
      continue;
    }
    // 跳过topic的校验
    if (ret < buffer_size_) {
      continue;
    }
    cv::Mat my_mat(rows_, cols_, CV_8UC3, info_buffer_ptr);
    store_img_ = my_mat.clone();
    delete info_buffer_ptr;
    if (end_thread_flag_) {
      break;
    }
    usleep(30000);
  }
  return;
}
