/*
 * @Author: windzu
 * @Date: 2022-04-16 17:46:20
 * @LastEditTime: 2022-05-17 10:10:58
 * @LastEditors: ls
 * @Description:
 * @FilePath:
 * /catkin_cone/src/utils/common_utils/include/common_utils/opencv_extension.hpp
 * @Copyright (C) 2021-2022 plusgo Company Limited. All rights reserved.
 * @Licensed under the Apache License, Version 2.0 (the License)
 */
#pragma once
// c++ system headers
#include <string>
#include <vector>
// third party header
// opencv
#include "opencv2/opencv.hpp"
// local header
#include "base_structure/cr_object.hpp"
#include "enum/enum.hpp"

void filiter_by_tl_object_area(std::vector<cr_object> *detected_objects,
                               int min_area, int max_area);
void filiter_by_contour_area(std::vector<std::vector<cv::Point>> *contours,
                             int min_area, int max_area);

/**
 * @description: 无畸变的resize图像至指定大小，对于空白部分填充黑色
 * @param {cv::Mat& } img : raw input image
 * @param {int} output_w : out image width
 * @param {int} output_h : out image height
 * @return {cv::Mat} : resized image
 */
cv::Mat resize_img(const cv::Mat &img, int output_w, int output_h);

/**
 * @description:
 * 计算检测出的bbox在输入的原图中的坐标(输入检测的图像被resize_img函数无畸变的resize至input_width*input_height，所以检测出的bbox需要变换回真实的bbox)
 * @param {Mat} &img : raw image which input to detector
 * @param {float*} bbox : bbox of traffic light detected by detector
 * @param {int} input_width : 网络的输入图像的宽度
 * @param {int} input_height : 网络的输入图像的高度
 * @return {cv::Rect} : rect of traffic light detected by detector in raw image
 */
cv::Rect get_rect(const cv::Mat &img, float bbox[4], int input_width,
                  int input_height);
