/*
 * @Author: windzu
 * @Date: 2022-04-16 17:46:25
 * @LastEditTime: 2022-05-17 10:11:54
 * @LastEditors: ls
 * @Description:
 * @FilePath: /catkin_cone/src/utils/common_utils/src/opencv_extension.cpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#include "common_utils/opencv_extension.hpp"

void filiter_by_tl_object_area(std::vector<cr_object> *detected_objects,
                               int min_area, int max_area) {
  for (auto it = detected_objects->begin(); it != detected_objects->end();) {
    if (it->bbox.area() < min_area || it->bbox.area() > max_area) {
      it = detected_objects->erase(it);
    } else {
      it++;
    }
  }
}

void filiter_by_contour_area(std::vector<std::vector<cv::Point>> *contours,
                             int min_area, int max_area) {
  for (auto it = contours->begin(); it != contours->end();) {
    int area = cv::contourArea(*it);
    if (area < min_area || area > max_area) {
      it = contours->erase(it);
    } else {
      ++it;
    }
  }
}

cv::Mat resize_img(const cv::Mat &img, int output_w, int output_h) {
  int w, h, x, y;
  float r_w = output_w / (img.cols * 1.0);
  float r_h = output_h / (img.rows * 1.0);
  if (r_h > r_w) {
    w = output_w;
    h = r_w * img.rows;
    x = 0;
    y = (output_h - h) / 2;
  } else {
    w = r_h * img.cols;
    h = output_h;
    x = (output_w - w) / 2;
    y = 0;
  }
  cv::Mat re(h, w, CV_8UC3);
  cv::resize(img, re, re.size(), 0, 0, cv::INTER_LINEAR);
  cv::Mat out(output_h, output_w, CV_8UC3, cv::Scalar(128, 128, 128));
  re.copyTo(out(cv::Rect(x, y, re.cols, re.rows)));
  return out;
}

cv::Rect get_rect(const cv::Mat &img, float bbox[4], int input_width,
                  int input_height) {
  int l, r, t, b;
  float r_w = input_width / (img.cols * 1.0);
  float r_h = input_height / (img.rows * 1.0);
  if (r_h > r_w) {
    l = bbox[0] - bbox[2] / 2.f;
    r = bbox[0] + bbox[2] / 2.f;
    t = bbox[1] - bbox[3] / 2.f - (input_height - r_w * img.rows) / 2;
    b = bbox[1] + bbox[3] / 2.f - (input_height - r_w * img.rows) / 2;
    l = l / r_w;
    r = r / r_w;
    t = t / r_w;
    b = b / r_w;
  } else {
    l = bbox[0] - bbox[2] / 2.f - (input_width - r_h * img.cols) / 2;
    r = bbox[0] + bbox[2] / 2.f - (input_width - r_h * img.cols) / 2;
    t = bbox[1] - bbox[3] / 2.f;
    b = bbox[1] + bbox[3] / 2.f;
    l = l / r_h;
    r = r / r_h;
    t = t / r_h;
    b = b / r_h;
  }
  return cv::Rect(l, t, r - l, b - t);
}
