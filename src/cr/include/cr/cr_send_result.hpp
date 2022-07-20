/*
 * @Description:
 * @version: 1.0.0
 * @Author: ls
 * @Date: 2022-05-16 11:37:40
 * @LastEditors: ls
 * @LastEditTime: 2022-07-19 15:20:07
 * @todo:
 * @FilePath: /catkin_cr_batch/src/cr/include/cr/cr_send_result.hpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */

#pragma once
// c++ system headers
#include <math.h>
#include <memory>
#include <string>
// third party headers
// opencv
#include "opencv2/opencv.hpp"
// ros
#include "cv_bridge/cv_bridge.h"
#include "image_transport/image_transport.h"
#include "ros/ros.h"
#include "std_msgs/String.h"
// local headers
#include "base_structure/cr_result.hpp"
#include "enum/enum.hpp"

class cr_send_result {
private:
  // ros
  ros::NodeHandle nh_;
  ros::NodeHandle pnh_;

  std::string ros_img_publish_topic_;
  std::string ros_img_publish_topic_1;
  std::string ros_img_publish_topic_2;
  std::string ros_img_publish_topic_3;
  std::string someone_publish_topic_;

  ros::Publisher someone_or_not;

  std::unique_ptr<image_transport::ImageTransport> it_ptr_;
  image_transport::Publisher img_publisher_;
  // sensor_msgs::ImagePtr img_publish_msg_;

  std::unique_ptr<image_transport::ImageTransport> it_ptr_1;
  image_transport::Publisher img_publisher_1;

  std::unique_ptr<image_transport::ImageTransport> it_ptr_2;
  image_transport::Publisher img_publisher_2;

  std::unique_ptr<image_transport::ImageTransport> it_ptr_3;
  image_transport::Publisher img_publisher_3;

  const std::string class_names_[1] = {"person"};

public:
  cr_send_result(ros::NodeHandle nh, ros::NodeHandle pnh) : nh_(nh), pnh_(pnh) {
    pnh_.param("ros_img_publish_topic", ros_img_publish_topic_,
               std::string("/perception/pr"));
    pnh_.param("ros_img_publish_topic1", ros_img_publish_topic_1,
               std::string("/perception/pr1"));
    pnh_.param("ros_img_publish_topic1", ros_img_publish_topic_2,
               std::string("/perception/pr2"));
    pnh_.param("ros_img_publish_topic1", ros_img_publish_topic_3,
               std::string("/perception/pr3"));
    pnh_.param("someone_publish_topic_", someone_publish_topic_,
               std::string("/perception/someone"));
  }

  bool init();
  void send_result(const cv::Mat &img, const cr_result &result, int i, bool &flag);

private:
  bool publish_img_with_bbox(const cv::Mat &img, const cr_result &result,
                             int i);
  int publish_result_people(const cr_result &result, int i);
  void draw(cv::Mat *img, const cr_result &result);
  float calculate_depth(float depth);
};
