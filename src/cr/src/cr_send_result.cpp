/*
 * @Description:
 * @version: 1.0.0
 * @Author: ls
 * @Date: 2022-05-16 13:17:34
 * @LastEditors: ls
 * @LastEditTime: 2022-07-19 15:25:16
 * @todo:
 * @FilePath: /catkin_cr_batch/src/cr/src/cr_send_result.cpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#include "cr/cr_send_result.hpp"

bool cr_send_result::init() {
  it_ptr_.reset(new image_transport::ImageTransport(nh_));
  img_publisher_ = it_ptr_->advertise(ros_img_publish_topic_, 1);
  it_ptr_1.reset(new image_transport::ImageTransport(nh_));
  img_publisher_1 = it_ptr_1->advertise(ros_img_publish_topic_1, 1);
  it_ptr_2.reset(new image_transport::ImageTransport(nh_));
  img_publisher_2 = it_ptr_2->advertise(ros_img_publish_topic_2, 1);
  it_ptr_3.reset(new image_transport::ImageTransport(nh_));
  img_publisher_3 = it_ptr_3->advertise(ros_img_publish_topic_3, 1);
  someone_or_not = nh_.advertise<std_msgs::String>(someone_publish_topic_, 1);
  return true;
}

void cr_send_result::send_result(const cv::Mat &img, const cr_result &result,
                                 int i, bool &flag) {
  // ros publish img
  int someone_flag = publish_result_people(result, i);
  bool ros_pub_flag = publish_img_with_bbox(img, result, i);
  if (!ros_pub_flag) {
    ROS_ERROR_STREAM("[ CR ] publish_img_with_bbox failed");
    return;
  } else {
    ROS_DEBUG_STREAM("[ CR ] publish_img_with_bbox success");
  }
  if (someone_flag == 1) {
    flag = true;
  }
  return;
}

bool cr_send_result::publish_img_with_bbox(const cv::Mat &img,
                                           const cr_result &result, int i) {
  cv::Mat publish_img = img.clone();
  draw(&publish_img, result);
  sensor_msgs::ImagePtr img_publish_msg_temp =
      cv_bridge::CvImage(std_msgs::Header(), "bgr8", publish_img).toImageMsg();
  switch (i) {
  case 0:
    img_publisher_.publish(img_publish_msg_temp);
    break;
  case 1:
    img_publisher_1.publish(img_publish_msg_temp);
    break;
  case 2:
    img_publisher_2.publish(img_publish_msg_temp);
    break;
  case 3:
    img_publisher_3.publish(img_publish_msg_temp);
    break;

  default:
    break;
  }

  return true;
}

void cr_send_result::draw(cv::Mat *img, const cr_result &result) {
  for (auto ob : result.object) {
    cv::Rect roi = ob.bbox;
    if (roi.x < 0 || roi.y < 0 || roi.width < 0 || roi.height < 0) {
      continue;
    }
    // draw bbox
    cv::rectangle(*img, roi, cv::Scalar(0, 0, 255), 3, cv::LINE_8, 0);
    // draw label and depth
    std::string class_name_str = class_names_[ob.oblcass];
    std::string put_txt = class_name_str;
    // float depth = calculate_depth(roi.height);
    std::string res = std::to_string(ob.depth);
    res = res.substr(0, res.find_last_not_of('0') + 1);
    cv::putText(*img, put_txt + " " + res + "m", cv::Point(roi.x, roi.y - 1),
                cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0x00, 0x00, 0x00), 3);
    // std::cout << ob.bbox.x << ";" << ob.bbox.y << ";"<< ob.bbox.width <<
    // ";"<< ob.bbox.height <<";" << ob.depth  << std::endl;
    ROS_DEBUG_STREAM("[ CR ] detected_object class is: " << put_txt);
  }
}

int cr_send_result::publish_result_people(const cr_result &result, int i) {
  std::stringstream ss;
  std_msgs::String msg;
  bool flag = false;
  for (auto ob : result.object) {
    if (i != 0 && ob.depth <= 5.0) {
      ss << "someone   " << std::to_string(ob.depth);
      flag = true;
      break;
    }
    if (i == 0 && ob.depth <= 20.0) {
      ss << "someone   " << std::to_string(ob.depth);
      flag = true;
      break;
    }
  }
  if (!flag) {
    ss << "no one";
  }
  msg.data = ss.str();
  someone_or_not.publish(msg);
  if (flag) {
    return 1;
  } else {
    return 0;
  }
}

float cr_send_result::calculate_depth(float height) {
  float depth_init = 1620.0 / height;
  float a1 = 1.62 * 0.18 / 0.71;
  float c1 = 0.18;

  float a = 1.0;
  float b = -(c1 + depth_init);
  float c = depth_init + c1 - depth_init + a1;
  float temp = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
  return round(temp * 100) / 100;
}
