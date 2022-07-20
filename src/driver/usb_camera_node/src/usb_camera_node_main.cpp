/*
 * @Author: windzu
 * @Date: 2022-02-24 18:56:19
 * @LastEditTime: 2022-04-16 20:43:59
 * @LastEditors: windzu
 * @Description:
 * @FilePath: /windzu_ws/src/driver/usb_camera_node/src/usb_camera_node_main.cpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */

// c system headers
#include <yaml-cpp/yaml.h>

// cpp system headers
#include <iostream>
#include <string>
#include <vector>

// third party headers
// ros
#include "cv_bridge/cv_bridge.h"
#include "image_transport/image_transport.h"
#include "ros/console.h"
#include "ros/ros.h"
#include "sensor_msgs/CompressedImage.h"
#include "sensor_msgs/Image.h"
#include "sensor_msgs/image_encodings.h"

// local headers
#include "common_utils/parse_camera_config.hpp"
#include "correct_img/correct_img.hpp"

int main(int argc, char **argv) {
  ros::init(argc, argv, "usb_camera");
  ros::NodeHandle nh;
  ros::NodeHandle pnh("~");

  std::string camera_config_path;
  std::string camera_id;
  std::string device_name;
  std::string publish_topic;
  cv::Size img_size;
  int fps;
  pnh.param<std::string>("camera_config_path", camera_config_path, "../../../config/camera_config.yaml");
  pnh.param<std::string>("camera_id", camera_id, "/camera/front_middle");

  // parse camera config
  bool ret = parse_camera_config(camera_config_path, camera_id, &device_name, &publish_topic, &img_size, &fps);
  if (!ret) {
    ROS_ERROR_STREAM("[ USB_CAMERA_NODE ] parse camera config failed ,camera_config_path : "
                     << camera_config_path << " camera_id : " << camera_id);
    return -1;
  }

  // correct img init
  CorrectImg correct_img(camera_config_path, camera_id);
  bool correct_img_ret = correct_img.init();
  if (!correct_img_ret) {
    ROS_WARN_STREAM("[ USB_CAMERA_NODE ] correct_img init failed , will publish raw image");
  }

  // video init
  cv::VideoCapture cap(device_name);
  if (!cap.isOpened()) {
    ROS_ERROR_STREAM("[ USB_CAMERA_NODE ] open camera failed , camera_device_name is : " << device_name);
    return -1;
  }

  // img publisher init
  image_transport::ImageTransport it(nh);
  image_transport::Publisher img_publisher = it.advertise(publish_topic, 1);
  cv::Mat frame;
  sensor_msgs::ImagePtr img_msg;

  ros::Rate loop_rate(fps);
  while (nh.ok()) {
    cap >> frame;
    if (!frame.empty()) {
      if (correct_img_ret) {
        correct_img.correct(&frame);
      }

      img_msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", frame).toImageMsg();
      img_publisher.publish(img_msg);
      cv::waitKey(1);
    }

    ros::spinOnce();
    loop_rate.sleep();
  }
  return 0;
}