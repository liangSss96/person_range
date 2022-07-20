/*
 * @Description:
 * @version: 1.0.0
 * @Author: ls
 * @Date: 2022-07-12 13:33:12
 * @LastEditors: ls
 * @LastEditTime: 2022-07-18 14:20:10
 * @todo:
 * @FilePath: /catkin_cone_batch/src/base_info/src/get_image_info.cpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#include "cv_bridge/cv_bridge.h"
#include "image_transport/image_transport.h"
#include "iostream"
#include "opencv2/opencv.hpp"
#include "ros/ros.h"
#include "sensor_msgs/CompressedImage.h"
#include "sensor_msgs/Image.h"
#include "sensor_msgs/image_encodings.h"

bool flag = false;

int count = 0;
cv::Mat get_img;
int count1 = 0;
int temp = 0;
std::string img_save_path;

void callback_compressed(const sensor_msgs::CompressedImageConstPtr &img_msg) {
  try {
    cv_bridge::CvImagePtr cv_ptr_compressed =
        cv_bridge::toCvCopy(img_msg, sensor_msgs::image_encodings::BGR8);
    get_img = cv_ptr_compressed->image;
    temp += 1;
    flag = true;
  } catch (cv_bridge::Exception &e) {
    std::cout << "cant't get image" << std::endl;
    ROS_ERROR_STREAM("cant't get image");
  }
}

void callback_image(const sensor_msgs::ImageConstPtr &img_msg) {
  try {
    cv_bridge::CvImagePtr cv_ptr =
        cv_bridge::toCvCopy(img_msg, sensor_msgs::image_encodings::BGR8);
    get_img = cv_ptr->image;
    temp += 1;
    flag = true;

  } catch (cv_bridge::Exception &e) {
    std::cout << "cant't get image" << std::endl;
    ROS_ERROR_STREAM("cant't get image");
  }
}

int main(int argc, char **argv) {
  std::string topic_name;
  int loop_rate_hz;

  ros::init(argc, argv, "info");
  ros::NodeHandle nh("~");
  ros::Subscriber s;
  nh.param("topic_name", topic_name, std::string("/front/image_raw"));
  nh.param("loop_rate_hz", loop_rate_hz, static_cast<int>(30));
  nh.param("img_save_path", img_save_path, std::string("~"));
  ros::Rate loop_rate(30);
  int index = topic_name.find_last_of('/');
  int len = topic_name.length();
  std::string sub = topic_name.substr(index + 1, len);
  // std::cout << topic_name << "  ;  " << sub << "  ;  " << img_save_path
  //           << std::endl;
  if (sub == "compressed") {
    s = nh.subscribe<sensor_msgs::CompressedImage>(topic_name, 1,
                                                   callback_compressed);
  } else {
    s = nh.subscribe<sensor_msgs::Image>(topic_name, 1, callback_image);
  }
  while (nh.ok()) {
    count += 1;
    if (!flag) {
      count1 = count;
    }
    if (count == 100) {
      int hz = temp / ((count - count1) / loop_rate_hz);
      std::cout << img_save_path + std::string("/base.png") << std::endl;
      cv::imwrite(img_save_path + std::string("/base.jpg"), get_img);
      std::cout << topic_name << ": "
                << "hz--" << hz << " || "
                << "resolution--" << get_img.size() << std::endl;

      break;
    }
    ros::spinOnce();
    loop_rate.sleep();
  }
  return 0;
}