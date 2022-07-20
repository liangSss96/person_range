/*
 * @Description:
 * @version: 1.0.0
 * @Author: ls
 * @Date: 2022-05-16 10:40:54
 * @LastEditors: ls
 * @LastEditTime: 2022-07-19 15:28:54
 * @todo:
 * @FilePath: /catkin_cr_batch/src/cr/src/cr.cpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#include "cr/cr.hpp"

bool CR::init() {
  topic_list.push_back(make_pair(img_topic_, img_sub_));
  topic_list.push_back(make_pair(img_topic_1, img_sub_1));
  topic_list.push_back(make_pair(img_topic_2, img_sub_2));
  topic_list.push_back(make_pair(img_topic_3, img_sub_3));

  bool msgs_init_flag = msgs_sub_init();
  std::cout << "msgs_init_flag: " << msgs_init_flag << std::endl;
  if (!msgs_init_flag) {
    ROS_ERROR_STREAM("[ CR ] msgs_sub_init failed");
    return false;
  }

  detector_ptr_.reset(new TLDDetector(cr_detector_weight_path_));
  bool cr_detector_flag = detector_ptr_->init();
  if (!cr_detector_flag) {
    ROS_ERROR_STREAM("[ CR ] CR_detector init failed");
    return false;
  }

  postprocess_ptr_.reset(new CRPostProcess(nh_, pnh_));
  bool postprocess_flag = postprocess_ptr_->init();
  if (!postprocess_flag) {
    ROS_ERROR_STREAM("[ CR ] CR_postprocess init failed");
    return false;
  }

  cr_send_result_ptr_.reset(new cr_send_result(nh_, pnh_));
  bool cr_send_result_flag = cr_send_result_ptr_->init();
  if (!cr_send_result_flag) {
    ROS_ERROR_STREAM("[ CR ] CR_send_result init failed");
    return false;
  }

  zmq_publish.reset(new ZeroMQPublisher(zmq_pub_topic_, zmq_pub_port_));
  bool zmq_publish_flag = zmq_publish->init();
  if (!zmq_publish_flag) {
    ROS_ERROR_STREAM("[ CR ] zmq_publish init failed");
    return false;
  }

  return true;
}

void CR::start() {
  ros::Rate loop_rate(loop_rate_hz_);

  bool cr_detector_ret = false;
  while (nh_.ok()) {
    bool someone = false;
    std::vector<std::vector<cr_object>> detected_objects(BATCH_SIZE);
    std::vector<cr_result> result(BATCH_SIZE);
    std::vector<cv::Mat> temp;
    // cv::Mat img_front = locked_img_.clone();
    // cv::Mat img_back = locked_img_1.clone();
    // cv::Mat img_left = locked_img_2.clone();
    // cv::Mat img_right = locked_img_3.clone();
    temp.push_back(locked_img_);
    temp.push_back(locked_img_1);
    temp.push_back(locked_img_2);
    temp.push_back(locked_img_3);

    if (img_updated_ || img_updated_1 || img_updated_2 || img_updated_3) {
      std::cout << img_updated_ << img_updated_1 << img_updated_2
                << img_updated_3 << std::endl;
      cr_detector_ret = detector_ptr_->detect(temp, &detected_objects);
      if (cr_detector_ret) {
        for (int i = 0; i < BATCH_SIZE; i++) {
          result[i].object = detected_objects[i];
          postprocess_ptr_->process(&result[i], i);
        }
      }
    } else {
      std::cout << "no image" << std::endl;
    }
    for (int i = 0; i < BATCH_SIZE; i++) {
      cr_send_result_ptr_->send_result(temp[i], result[i], i, someone);
    }
    std::string people;
    if (someone) {
      zmq_publish->publish_str(std::string("yes"));
    } else {
      zmq_publish->publish_str(std::string("no"));
    }
    ros::spinOnce();
    loop_rate.sleep();
  }

  return;
}

bool CR::msgs_sub_init() {
  for (int i = 0; i < BATCH_SIZE; i++) {
    std::vector<std::string> v;
    split_string(topic_list[i].first, &v, "/");
    if (v.back() == "compressed") {
      topic_list[i].second = nh_.subscribe<sensor_msgs::CompressedImage>(
          topic_list[i].first, 1,
          boost::bind(&CR::receive_compressed_img_callback, this, _1, img[i],
                      flag[i], key[i]));
    } else {
      topic_list[i].second = nh_.subscribe<sensor_msgs::Image>(
          topic_list[i].first, 1,
          boost::bind(&CR::receive_raw_img_callback, this, _1, img[i], flag[i],
                      key[i]));
    }
  }
  ros::AsyncSpinner s(4);
  s.start();
  return true;
}

void CR::receive_raw_img_callback(const sensor_msgs::ImageConstPtr &img_msg,
                                  cv::Mat *get_img, bool *flag,
                                  std::mutex *key) {
  try {
    cv_bridge::CvImagePtr cv_ptr_img =
        cv_bridge::toCvCopy(img_msg, sensor_msgs::image_encodings::BGR8);
    key->lock();
    *get_img = cv_ptr_img->image;
    *flag = true;
    key->unlock();

  } catch (cv_bridge::Exception &e) {
    std::cout << "cant't get image" << std::endl;
    ROS_ERROR_STREAM("cant't get image");
  }
  return;
}

void CR::receive_compressed_img_callback(
    const sensor_msgs::CompressedImageConstPtr &img_msg, cv::Mat *get_img,
    bool *flag, std::mutex *key) {
  try {
    cv_bridge::CvImagePtr cv_ptr_compressed =
        cv_bridge::toCvCopy(img_msg, sensor_msgs::image_encodings::BGR8);
    key->lock();
    *get_img = cv_ptr_compressed->image;
    *flag = true;
    key->unlock();
  } catch (cv_bridge::Exception &e) {
    std::cout << "cant't get image" << std::endl;
    ROS_ERROR_STREAM("cant't get image");
  }
  return;
}